#include "StdAfx.h"
#include "WaveWriter.h"

CWaveWriter::CWaveWriter(void)
{
	init		=	false	;
	m_hOPFile	=	NULL	; 
}

CWaveWriter::~CWaveWriter(void)
{
	WaveClose();
}

int	CWaveWriter::WaveInit(const char* fileName,WORD bps_,DWORD sps_,WORD channels_)
{
	USES_CONVERSION;
	int			nT1=0	;
	MMRESULT	mRes	;
	if(init)
	{
		return FAILURE;
	}
	m_stWFEX.nSamplesPerSec		=	sps_											;
	m_stWFEX.nChannels			=	channels_										;
	m_stWFEX.wBitsPerSample		=	bps_											;
	m_stWFEX.wFormatTag			=	WAVE_FORMAT_PCM									;
	m_stWFEX.nBlockAlign		=	m_stWFEX.nChannels*m_stWFEX.wBitsPerSample/8	;
	m_stWFEX.nAvgBytesPerSec	=	m_stWFEX.nSamplesPerSec*m_stWFEX.nBlockAlign	;
	m_stWFEX.cbSize				=	sizeof(WAVEFORMATEX)							;

	ZeroMemory(&m_stmmIF,sizeof(MMIOINFO));
	DeleteFileA(fileName);
	m_hOPFile=mmioOpen(A2T(fileName),&m_stmmIF,MMIO_WRITE | MMIO_CREATE);
	if(m_hOPFile==NULL)
	{
		init    =	false	;
		return		FAILURE	;
	}
	ZeroMemory(&m_stckOutRIFF,sizeof(MMCKINFO));
	m_stckOutRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E'); 
	mRes=mmioCreateChunk(m_hOPFile, &m_stckOutRIFF, MMIO_CREATERIFF);
	if(mRes!=MMSYSERR_NOERROR)
	{
		init    =	false	;
		return		FAILURE	;
	}
	ZeroMemory(&m_stckOut,sizeof(MMCKINFO));
	m_stckOut.ckid = mmioFOURCC('f', 'm', 't', ' ');
	m_stckOut.cksize = sizeof(m_stWFEX);
	mRes=mmioCreateChunk(m_hOPFile, &m_stckOut, 0);
	if(mRes!=MMSYSERR_NOERROR)
	{
		init    =	false	;
		return		FAILURE	;
	}
	nT1=mmioWrite(m_hOPFile, (HPSTR) &m_stWFEX, sizeof(m_stWFEX));
	if(nT1!=sizeof(m_stWFEX))
	{
		init    =	false	;
		return		FAILURE	;
	}
	mRes=mmioAscend(m_hOPFile, &m_stckOut, 0);
	if(mRes!=MMSYSERR_NOERROR)
	{
		init    =	false	;
		return		FAILURE	;
	}
	m_stckOut.ckid = mmioFOURCC('d', 'a', 't', 'a');
	mRes=mmioCreateChunk(m_hOPFile, &m_stckOut, 0);
	if(mRes!=MMSYSERR_NOERROR)
	{
		init    =	false	;
		return		FAILURE	;
	}

	init		= true;
	return		SUCCESS;
}
int	CWaveWriter::WaveWrite(void* buffer,int len)
{
	if(init)
	{
		mmioWrite(m_hOPFile,(const char*)buffer,(len*m_stWFEX.wBitsPerSample)/8);
		return SUCCESS;
	}
	return FAILURE;
}

int	CWaveWriter::WaveClose()
{
	MMRESULT mRes;
	if(m_hOPFile)
	{
		mRes=mmioAscend(m_hOPFile, &m_stckOut, 0);
		if(mRes!=MMSYSERR_NOERROR)
		{
			return FAILURE;
		}
		mRes=mmioAscend(m_hOPFile, &m_stckOutRIFF, 0);
		if(mRes!=MMSYSERR_NOERROR)
		{
			return FAILURE;
		}
		mmioClose(m_hOPFile,0);
		m_hOPFile=NULL;
	}
	init = false;
	return SUCCESS;
}
