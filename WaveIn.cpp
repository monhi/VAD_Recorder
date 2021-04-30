#include "StdAfx.h"
#include "WaveIn.h"
#include "general.h"
#include "VADRecorderdlg.h"

#pragma comment(lib,"winmm.lib")
bool isThreadActive;
bool Terminated;

void CALLBACK waveInFunc(HWAVEIN hwi,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	WAVEHDR *pHdr=NULL;
	isThreadActive = true;
	switch(uMsg)
	{
		case WIM_CLOSE:
			break;

		case WIM_DATA:
			{
				if(!Terminated)
				{
					CWaveIn *pDlg=(CWaveIn*)dwInstance;
					pDlg->ProcessHeader((WAVEHDR *)dwParam1);
				}
			}
			break;

		case WIM_OPEN:
			break;

		default:
			break;
	}
	isThreadActive = false;
}

CWaveIn::CWaveIn(void)
{
		isThreadActive	= false ;
		Terminated		= true  ;
		write_size		= 0		;
		wBytesPerSample = 0		;
		sps				= 0		;
		m_hWaveIn		= NULL	;
		pWriterQ		= NULL	;
		ZeroMemory(&m_stWFEX,sizeof(WAVEFORMATEX));
		ZeroMemory(m_stWHDR,MAX_INPUT_BUFFERS*sizeof(WAVEHDR));
		//Initialize webRTC VAD module.
		WebRtcVad_Create(&handle);
		WebRtcVad_Init(handle);
		WebRtcVad_set_mode(handle, 3);
}


CWaveIn::~CWaveIn(void)
{
	StopCapture();
	if(pWriterQ)
		delete pWriterQ;
	pWriterQ  = NULL;
	if(handle)
	{
		WebRtcVad_Free(handle);
		handle = NULL;
	}

}

int CWaveIn::GetDeviceCount(void)
{
	return waveInGetNumDevs();
}


int CWaveIn::GetDeviceCaps(int deviceNo,PWAVEINCAPS pWaveParams)
{
	return waveInGetDevCaps(deviceNo,pWaveParams,sizeof(WAVEINCAPS));
}

int CWaveIn::OpenDevice( WORD	 deviceNo,
						 DWORD	 samplesPerSecond,
						 WORD	 nChannels,
						 WORD    wBitsPerSample
					   )
{
	if(m_hWaveIn)// Device is open now.
		return FAILURE;
	if(pWriterQ ==NULL)
	{
		pWriterQ = new CWriterQ(25,samplesPerSecond,wBitsPerSample/8);
	}
	sps = samplesPerSecond;
	MMRESULT mRes				=	0					;
	m_stWFEX.nSamplesPerSec		=	samplesPerSecond	;
	m_stWFEX.nChannels			=	nChannels			;
	m_stWFEX.wBitsPerSample		=	wBitsPerSample		;
	m_stWFEX.wFormatTag			=	WAVE_FORMAT_PCM		;
	m_stWFEX.nBlockAlign		=	m_stWFEX.nChannels*m_stWFEX.wBitsPerSample/8;
	m_stWFEX.nAvgBytesPerSec	=	m_stWFEX.nSamplesPerSec*m_stWFEX.nBlockAlign;
	m_stWFEX.cbSize				=	sizeof(WAVEFORMATEX);

	sd.data_in			=	InBuf			;
	sd.data_out			=	OutBuf			;
	sd.end_of_input		=	0				;
	sd.input_frames		=	(sps/5)			;
	sd.output_frames	=	DEST_BUF_SIZE	;
	sd.src_ratio		=	((float)DEST_FS)/((float)sps);
	Terminated			=	false			;
	mRes=waveInOpen(&m_hWaveIn,
					deviceNo,
					&m_stWFEX,
					(DWORD_PTR)waveInFunc,
					(DWORD_PTR)this,
					CALLBACK_FUNCTION);
	return mRes;
}

int CWaveIn::ProcessHeader(WAVEHDR * pHdr)
{
	int			i			;
	MMRESULT	mRes=0		;
	int			res			;
	bool		recording = false;

	if(WHDR_DONE==(WHDR_DONE &pHdr->dwFlags))
	{
			src_short_to_float_array((const short*)pHdr->lpData, sd.data_in,write_size);
			sd.end_of_input = 0;
			src_simple (&sd, SRC_SINC_BEST_QUALITY, 1);			
			sd.data_out[1599] = sd.data_out[1598];
			src_float_to_short_array (sd.data_out, iBuffer, DEST_BUF_SIZE);
			for(i=0; i <10; i++)
			{
				res = WebRtcVad_Process(handle,DEST_FS,&iBuffer[i*160],160);
				if( res == 1)
				{
					recording = true;
					pWriterQ->Add2Queue((uint8*)(&iBuffer[i*160]),160);
				}
				else if( res == 0)
				{
				}
				else 
				{
				}
			}
			pointer->SetRecording(recording);
			pointer->PostMessage(WM_MY_THREAD_MESSAGE);



		mRes=waveInAddBuffer(m_hWaveIn,pHdr,sizeof(WAVEHDR));
	}
	return SUCCESS;
}

int CWaveIn::PrepareBuffers(float ratio)
{
	MMRESULT mRes=0;
	int nT1=0;
	int c_size;

	for(nT1=0;nT1<MAX_INPUT_BUFFERS;++nT1)
	{
		c_size = (SIZE_T)m_stWFEX.nAvgBytesPerSec*ratio;
		if(c_size%2)
			c_size--;
		m_stWHDR[nT1].lpData=(LPSTR)HeapAlloc(GetProcessHeap(),8,(SIZE_T)c_size);
		m_stWHDR[nT1].dwBufferLength=(DWORD)c_size								;
		wBytesPerSample		= m_stWFEX.wBitsPerSample/8							;
		write_size			= c_size/wBytesPerSample	;


		m_stWHDR[nT1].dwUser		= nT1									;
		mRes=waveInPrepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR))	;
		if(mRes!=0)
		{
			return FAILURE;
		}
		mRes=waveInAddBuffer(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
		if(mRes!=0)
		{
			return FAILURE;
		}
	}
	return SUCCESS;
}

int CWaveIn::UnPrepareBuffers()
{
	MMRESULT mRes=0;
	int nT1=0;

	if(m_hWaveIn)
	{
		Terminated = true;
		while(isThreadActive)
		{
			Sleep(30);
		}
		mRes=waveInStop(m_hWaveIn);

		for(nT1=0;nT1<MAX_INPUT_BUFFERS;++nT1)
		{
			if(m_stWHDR[nT1].lpData)
			{
				mRes=waveInUnprepareHeader(m_hWaveIn,&m_stWHDR[nT1],sizeof(WAVEHDR));
				HeapFree(GetProcessHeap(),0,m_stWHDR[nT1].lpData);
				ZeroMemory(&m_stWHDR[nT1],sizeof(WAVEHDR));
			}
		}
	}
	return SUCCESS;
}


int CWaveIn::StartCapture(float bufferSizeRatio)
{
	MMRESULT mRes=0;
	if(m_hWaveIn == NULL)
		return FAILURE;
	if(PrepareBuffers(bufferSizeRatio)==FAILURE)// failure in preparing buffers.
		return FAILURE;
	mRes=waveInStart(m_hWaveIn);
	return mRes;
}

int CWaveIn::StopCapture(void)
{
	MMRESULT mRes=0;
	if(m_hWaveIn)
	{
		UnPrepareBuffers();
		mRes=waveInClose(m_hWaveIn);
		m_hWaveIn = NULL;
		return SUCCESS;
	}
	return FAILURE;
}
