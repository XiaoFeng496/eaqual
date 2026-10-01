/////////////////////////////////////////////////////////////////////////////////////
// wav_in.cpp - 极简 WAV 读取替身（替代 libsndfile）
// 仅解析标准 RIFF/WAVE + fmt + data，支持 16/24-bit PCM，整文件读入内存。
// 与 libsndfile 的 sf_open_read / sf_read_short / sf_close 保持同名同签名。
// 管道模式：path == "-" 时从 stdin 读 raw int16（test 走管道，ref 仍走文件）。
/////////////////////////////////////////////////////////////////////////////////////
#include "wav_in.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

struct MYWAV_ {
    short*  data;        // 交错存放的 int16 样本，长度 = frames * channels
    long    frames;
    int     channels;
    long    pos_items;   // 已读位置（以 item = 单声道样本 为单位）
};

static unsigned long le32(const unsigned char* p){
    return (unsigned long)p[0]        | ((unsigned long)p[1] << 8) |
          ((unsigned long)p[2] << 16) | ((unsigned long)p[3] << 24);
}
static unsigned short le16(const unsigned char* p){
    return (unsigned short)(p[0] | (p[1] << 8));
}

// 从任意 FILE* 读取 raw little-endian 交错 PCM（默认 48000/16/2，可由 -srate/-bits/-ch 改写）。
// 目前供 stdin('-') 使用：stdin 经 _setmode 置二进制后即为裸 PCM 流。
static SNDFILE* read_raw_pcm(FILE* fp, SF_INFO* info){
    size_t cap = 1u << 16, len = 0;
    unsigned char* buf = (unsigned char*)malloc(cap);
    if(!buf) return NULL;
    size_t n;
    while((n = fread(buf + len, 1, cap - len, fp)) > 0){
        len += n;
        if(len >= cap){
            cap *= 2;
            unsigned char* nb = (unsigned char*)realloc(buf, cap);
            if(!nb){ free(buf); return NULL; }
            buf = nb;
        }
    }
    int channels = (info->channels > 0) ? info->channels : 2;
    int bits     = (info->pcmbitwidth > 0) ? info->pcmbitwidth : 16;
    long bps = (bits == 24) ? 3 : 2;
    long total = (long)(len / (unsigned long)bps);   // 单声道样本数
    long frames = total / channels;
    if(frames <= 0){ free(buf); return NULL; }
    short* data = (short*)malloc((size_t)frames * channels * sizeof(short));
    if(!data){ free(buf); return NULL; }
    if(bits == 24){
        for(long i = 0; i < frames*channels; i++){
            long v = (long)(buf[3*i] | (buf[3*i+1] << 8) | (buf[3*i+2] << 16));
            if(v & 0x800000) v -= 0x1000000;
            data[i] = (short)(v >> 8);
        }
    } else {
        for(long i = 0; i < frames*channels; i++)
            data[i] = (short)(buf[2*i] | (buf[2*i+1] << 8));
    }
    free(buf);
    MYWAV_* h = (MYWAV_*)malloc(sizeof(MYWAV_));
    h->data = data;
    h->frames = frames;
    h->channels = channels;
    h->pos_items = 0;
    info->samplerate   = (info->samplerate > 0) ? info->samplerate : 48000;
    info->channels    = channels;
    info->pcmbitwidth = bits;
    return h;
}

SNDFILE* sf_open_read(const char* path, SF_INFO* info){
    // 管道模式：'-' 表示从 stdin 读 raw PCM（test 走管道，ref 仍走文件）
    if(path && strcmp(path, "-") == 0){
        _setmode(fileno(stdin), O_BINARY);   // 管道二进制必须关掉 CRLF 翻译，否则 PCM 字节被改写
        return read_raw_pcm(stdin, info);
    }

    FILE* fp = fopen(path, "rb");
    if(!fp) return NULL;

    unsigned char hdr[12];
    if(fread(hdr, 1, 12, fp) != 12){ fclose(fp); return NULL; }
    if(memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0){
        fclose(fp); return NULL;
    }

    int   channels = 0, samplerate = 0, bits = 0, audioFormat = 0;
    unsigned char fmt[40];
    long  dataSize = 0;
    int   foundFmt = 0, foundData = 0;

    while(1){
        unsigned char ch[8];
        if(fread(ch, 1, 8, fp) != 8) break;
        unsigned long sz = le32(ch + 4);
        if(memcmp(ch, "fmt ", 4) == 0){
            unsigned long rsz = (sz < sizeof(fmt)) ? sz : sizeof(fmt);
            if(fread(fmt, 1, rsz, fp) != (size_t)rsz){ fclose(fp); return NULL; }
            audioFormat = le16(fmt + 0);
            channels    = le16(fmt + 2);
            samplerate  = (int)le32(fmt + 4);
            bits        = le16(fmt + 14);
            foundFmt = 1;
            if((long)sz > (long)rsz) fseek(fp, (long)sz - (long)rsz, SEEK_CUR);
        } else if(memcmp(ch, "data", 4) == 0){
            dataSize = (long)sz;
            foundData = 1;
            break;
        } else {
            long skip = (long)sz + (sz & 1);   // 块按偶数字节对齐
            fseek(fp, skip, SEEK_CUR);
        }
    }
    if(!foundFmt || !foundData){ fclose(fp); return NULL; }
    if(audioFormat != 1){ fclose(fp); return NULL; }          // 只支持 PCM
    if(bits != 16 && bits != 24){ fclose(fp); return NULL; }

    long bytesPerFrame = (long)channels * (bits / 8);
    long frames = dataSize / bytesPerFrame;
    if(frames <= 0){ fclose(fp); return NULL; }

    short* data = (short*)malloc((size_t)frames * channels * sizeof(short));
    if(!data){ fclose(fp); return NULL; }

    unsigned char* raw = (unsigned char*)malloc((size_t)dataSize);
    if(!raw){ free(data); fclose(fp); return NULL; }
    if(fread(raw, 1, (size_t)dataSize, fp) != (size_t)dataSize){
        free(raw); free(data); fclose(fp); return NULL;
    }
    fclose(fp);

    long total = frames * channels;
    if(bits == 16){
        for(long i = 0; i < total; i++)
            data[i] = (short)(raw[2*i] | (raw[2*i + 1] << 8));
    } else {   // 24-bit -> 16-bit（右移 8 位，保留符号）
        for(long i = 0; i < total; i++){
            long v = (long)(raw[3*i] | (raw[3*i + 1] << 8) | (raw[3*i + 2] << 16));
            if(v & 0x800000) v -= 0x1000000;
            data[i] = (short)(v >> 8);
        }
    }
    free(raw);

    MYWAV_* h = (MYWAV_*)malloc(sizeof(MYWAV_));
    h->data = data;
    h->frames = frames;
    h->channels = channels;
    h->pos_items = 0;
    info->samplerate  = samplerate;
    info->channels   = channels;
    info->pcmbitwidth = bits;
    return h;
}

int sf_read_short(SNDFILE* file, short* buffer, int items){
    MYWAV_* h = (MYWAV_*)file;
    if(!h || items <= 0) return 0;
    long total_items = h->frames * h->channels;
    long avail = total_items - h->pos_items;
    if(avail < 0) avail = 0;
    long n = (avail < (long)items) ? avail : (long)items;
    for(long i = 0; i < n; i++)
        buffer[i] = h->data[h->pos_items + i];
    h->pos_items += n;
    return (int)n;
}

void sf_close(SNDFILE* file){
    MYWAV_* h = (MYWAV_*)file;
    if(!h) return;
    free(h->data);
    free(h);
}
