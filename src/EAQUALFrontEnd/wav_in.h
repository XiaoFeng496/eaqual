#ifndef WAV_IN_H
#define WAV_IN_H

// 极简 WAV 读取替身：替换 EAQUAL 原本依赖的 libsndfile。
// 仅支持 PCM（16-bit / 24-bit），把整文件读入内存后按 short 块喂给 PEAQ 核心。
// 接口刻意与 libsndfile 的 sf_open_read / sf_read_short / sf_close 同名同签名，
// 因此 EAQUALMain.cpp 里调用点无需改动。

struct SF_INFO {
    int samplerate;
    int channels;
    int pcmbitwidth;
    int format;
};

typedef struct MYWAV_ SNDFILE;

SNDFILE* sf_open_read(const char* path, SF_INFO* info);
int      sf_read_short(SNDFILE* file, short* buffer, int items);
void     sf_close(SNDFILE* file);

#endif
