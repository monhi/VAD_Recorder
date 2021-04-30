#include "StdAfx.h"
#include "WaveGen.h"
#include "general.h"

CWaveGen::CWaveGen(void)
{
	fileName[0] = 0;
	wFile    = NULL;
}

CWaveGen::~CWaveGen(void)
{
	CloseWaveFile();
}

int16 CWaveGen::SetFileName(const char* n)
{
	strcpy(fileName,n);
	return SUCCESS;
}

int16 CWaveGen::SetWaveType(uint8 st)
{
	waveType = st;
	switch( st )
	{
		case SAMPLE_TYPE_16BIT_PCM_INTEL:
			SetBytesPerSample(2);
		break;
		default:
			SetBytesPerSample(1);
	}
	return SUCCESS	;
}

int16 CWaveGen::SetSampleRate(uint32 rate)
{
	sampleRate = rate;
	return SUCCESS;
}

int16 CWaveGen::SetBytesPerSample(uint8 n)
{
	bytesPerSample = n;
	return SUCCESS;
}

int16 CWaveGen::WriteHeaderWave(uint32 size)
{
	uint8 header[44];
	uint32 temp_size;
	if(wFile==NULL)
		return FAILURE;
	header[0] = 'R';
	header[1] = 'I';
	header[2] = 'F';
	header[3] = 'F';
// ChunkSize : 36 + sub chunk size 2
	temp_size	=   size -8	;
	header[4]	=	(uint8)((temp_size & 0x000000FF)    );
	header[5]	=	(uint8)((temp_size & 0x0000FF00)>> 8);
	header[6]	=	(uint8)((temp_size & 0x00FF0000)>>16);
	header[7]	=	(uint8)((temp_size & 0xFF000000)>>24);
// WAVE
	header[8]   =	'W'		;
	header[9]   =	'A'		;
	header[10]  =	'V'		;
	header[11]  =	'E'		;
// fmt
	header[12]  =	'f'		;
	header[13]  =	'm'		;
	header[14]  =	't'		;
	header[15]  =	0x20	;
//  Subchunk1 Size
	header[16] =	16		;
	header[17] =	0		;
	header[18] =	0		;
	header[19] =	0		;
// Audio format : 1 no compression.
	header[20] = waveType	;
	header[21] = 0			;
// number of channels : mono mode.
	header[22] = 1	;
	header[23] = 0	;
// Sample Rate  : 8000 samples per second.
	header[24]	= (sampleRate & 0x000000FF)		;
	header[25]	= (sampleRate & 0x0000FF00)>>8	;
	header[26]	= (sampleRate & 0x00FF0000)>>16	;
	header[27]	= (sampleRate & 0xFF000000)>>24	;
//	Byte Rate    : 8000
	header[28]	= ((sampleRate*bytesPerSample) & 0x000000FF)	 ;
	header[29]	= ((sampleRate*bytesPerSample) & 0x0000FF00)>>8	 ;
	header[30]	= ((sampleRate*bytesPerSample) & 0x00FF0000)>>16 ;
	header[31]	= ((sampleRate*bytesPerSample) & 0xFF000000)>>24 ;
// Block Align
	header[32]	= bytesPerSample				; // 16 bit for each sample.
	header[33]	= 0								;
//  Bits Per Sample.
	header[34]  = bytesPerSample*8				;
	header[35]	= 0								;
//  data expression
	header[36]	= 'd'							;
	header[37]	= 'a'							;
	header[38]	= 't'							;
	header[39]	= 'a'							;
//  sub chunk 2 size : NumSamples * NumChannels * BitsPerSample/8	;
	temp_size   =	size - 44						;
	header[40]	=(uint8)((temp_size & 0x000000FF)	);
	header[41]	=(uint8)((temp_size & 0x0000FF00)>> 8);
	header[42]	=(uint8)((temp_size & 0x00FF0000)>>16);
	header[43]	=(uint8)((temp_size & 0xFF000000)>>24);
	fseek(wFile,0,SEEK_SET);
	if(fwrite((void*)header,1,44,wFile) == 0)
		return FAILURE;
	fseek(wFile,0,SEEK_END);
	return			SUCCESS						;

}

int16 CWaveGen::WriteData(void* data,uint32 size)
{
	if(!wFile)
		return FAILURE;
    if(fwrite(data,1,size,wFile)==0)
		return FAILURE;
	return SUCCESS;
}

int16 CWaveGen::OpenWaveFile(void)
{
	CloseWaveFile();
	wFile=fopen(fileName,"wb");
	if(wFile)
		return SUCCESS;
	return FAILURE;
}

int16 CWaveGen::CloseWaveFile(void)
{
	if(wFile)
		fclose(wFile);
	wFile = NULL;
	return SUCCESS;
}

int16 CWaveGen::WriteWaveSize(void)
{
	uint32 size;
	fseek(wFile,0,SEEK_END);
	size = ftell(wFile);
	return WriteHeaderWave(size);
}