#pragma once
#include "General.h"
class CWaveGen
{
public:
	CWaveGen(void);
public:
	~CWaveGen(void);
public:
	int16	SetFileName(const char* n);
	int16	SetWaveType(uint8 s);
	int16	SetSampleRate(uint32 rate);
	int16	SetBytesPerSample(uint8 n);
	int16	WriteHeaderWave(uint32 size);
	int16	WriteData(void* data,uint32 size);
	int16	OpenWaveFile(void);
	int16	CloseWaveFile(void);
	int16	WriteWaveSize(void);
private:
	char	fileName[MAX_PATH];
	uint8	waveType;
	uint32	sampleRate;
	uint8	bytesPerSample;
	FILE*   wFile;
};
