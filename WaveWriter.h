#pragma once
#include "general.h"
#include <MMSystem.h>


class CWaveWriter
{
public:
			CWaveWriter(void);
			~CWaveWriter(void);
	int		WaveInit(const char* fileName,WORD bps_,DWORD sps_,WORD channels_);
	int		WaveWrite(void* buffer,int len);
	int		WaveClose();
private:
	bool			init					;
	HMMIO			m_hOPFile				;
	WAVEFORMATEX	m_stWFEX				;
	MMIOINFO		m_stmmIF				;
	MMCKINFO		m_stckOut,m_stckOutRIFF	; 

};
