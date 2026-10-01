# EAQUAL Makefile (modernized): 用 g++ 编译，去掉 2002 年的 libSndFile 依赖，
# 改由 src/EAQUALFrontEnd/wav_in.cpp 充当最小 WAV 读取（仅 PCM 16/24-bit）。
COMPILER = g++
CFLAGS  = -c -Wall -O3 -pipe -fpermissive -include ../../src/GlobalHeaders/zTypes.h -include ../../src/GlobalHeaders/zGlobals.h -I ../../src/libEAQUAL/ -I ../../src/GlobalHeaders/ -I ../../src/EAQUALFrontEnd/

EAQUAL : EAQUALMain.o EAQUALArgs.o EAQUALOutput.o wav_in.o libEAQUAL.a
	$(COMPILER) EAQUALMain.o EAQUALArgs.o EAQUALOutput.o wav_in.o libEAQUAL.a -lm -o ../../bin/EAQUAL

EAQUALMain.o : ../../src/EAQUALFrontEnd/EAQUALMain.cpp
	$(COMPILER) $(CFLAGS) ../../src/EAQUALFrontEnd/EAQUALMain.cpp

EAQUALArgs.o : ../../src/EAQUALFrontEnd/EAQUALArgs.cpp
	$(COMPILER) $(CFLAGS) ../../src/EAQUALFrontEnd/EAQUALArgs.cpp

EAQUALOutput.o : ../../src/EAQUALFrontEnd/EAQUALOutput.cpp
	$(COMPILER) $(CFLAGS) ../../src/EAQUALFrontEnd/EAQUALOutput.cpp

wav_in.o : ../../src/EAQUALFrontEnd/wav_in.cpp ../../src/EAQUALFrontEnd/wav_in.h
	$(COMPILER) $(CFLAGS) ../../src/EAQUALFrontEnd/wav_in.cpp

libEAQUAL.a : AvgBlock.o AvgTime.o CalcTables.o FBModel.o Fft.o FFTModel.o FileIO.o KogModel.o libEAQUAL.o MOVBlock.o PreProc.o PsychoModel.o RunAnalysis.o
	ar rcs libEAQUAL.a AvgBlock.o AvgTime.o CalcTables.o FBModel.o Fft.o FFTModel.o FileIO.o KogModel.o libEAQUAL.o MOVBlock.o PreProc.o PsychoModel.o RunAnalysis.o

AvgBlock.o : ../../src/libEAQUAL/AvgBlock.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/AvgBlock.cpp

AvgTime.o : ../../src/libEAQUAL/AvgTime.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/AvgTime.cpp

CalcTables.o : ../../src/libEAQUAL/CalcTables.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/CalcTables.cpp

FBModel.o : ../../src/libEAQUAL/FBModel.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/FBModel.cpp

Fft.o : ../../src/libEAQUAL/Fft.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/Fft.cpp

FFTModel.o : ../../src/libEAQUAL/FFTModel.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/FFTModel.cpp

FileIO.o : ../../src/libEAQUAL/FileIO.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/FileIO.cpp

KogModel.o : ../../src/libEAQUAL/KogModel.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/KogModel.cpp

libEAQUAL.o : ../../src/libEAQUAL/libEAQUAL.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/libEAQUAL.cpp

MOVBlock.o : ../../src/libEAQUAL/MOVBlock.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/MOVBlock.cpp

PreProc.o : ../../src/libEAQUAL/PreProc.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/PreProc.cpp

PsychoModel.o : ../../src/libEAQUAL/PsychoModel.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/PsychoModel.cpp

RunAnalysis.o : ../../src/libEAQUAL/RunAnalysis.cpp
	$(COMPILER) $(CFLAGS) ../../src/libEAQUAL/RunAnalysis.cpp

clean:
	rm -f *.o *.a ../../bin/EAQUAL
