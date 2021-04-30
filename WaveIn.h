#pragma once
#include <mmsystem.h>
#define MAX_INPUT_BUFFERS   3
#include "WriterQ.h"
#include "general.h"
#include "./vad/include/webrtc_vad.h"
#include "./sr/samplerate.h"
class CVADRecorderDlg;

/*
This class is a simple wave in device which
captures sound data from input microphones.
*/
class CWaveIn
{
public:
			CWaveIn			(void);
			~CWaveIn		(void);
	int		GetDeviceCount	(void);
	int		GetDeviceCaps	(int deviceNo,PWAVEINCAPS pWaveParams);
	int		OpenDevice		(	 WORD	 deviceNo,
								DWORD	 samplesPerSecond,
								 WORD	 nChannels,
								 WORD    wBitsPerSample
							);
	int						StartCapture	(float ratio);
	int						StopCapture		(void);
	int						ProcessHeader	(WAVEHDR * pHdr);
	CWriterQ* pWriterQ;
	CVADRecorderDlg* pointer;
private:
	WAVEFORMATEX			m_stWFEX					 ;
	HWAVEIN					m_hWaveIn					 ;
	WAVEHDR					m_stWHDR[MAX_INPUT_BUFFERS]	 ;
	int						PrepareBuffers(float)		 ;
	int						UnPrepareBuffers(void)		 ;
	uint32					write_size					 ;
	int						wBytesPerSample				 ;
	VadInst*				handle						 ;
	Float					InBuf[5000]					 ;
	Float					OutBuf[5000]				 ;
	DWORD					sps							 ;
	SRC_DATA				sd							 ;
	short					iBuffer[DEST_BUF_SIZE]		 ;


};
