// VADRecorderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "general.h"
#include "VADRecorder.h"
#include "VADRecorderDlg.h"
#include "WaveIn.h"
#include "ReaderQ.h"
#include "WaveWriter.h"
#include "config.h"
#define PROGRAM_MAJOR_VERSION	 1
#define PROGRAM_MINOR_VERSION	 0
#ifdef _DEBUG
#define new DEBUG_NEW
#endif



UINT  NOISE_FUNC(LPVOID pParam)
{
	CVADRecorderDlg *pointer = (CVADRecorderDlg*) pParam;
	pointer->NoiseRoutine();
	return SUCCESS;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVADRecorderDlg dialog




CVADRecorderDlg::CVADRecorderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVADRecorderDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVADRecorderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_Contents);
	DDX_Control(pDX, IDC_RECORDING, m_Recording);
}

BEGIN_MESSAGE_MAP(CVADRecorderDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OPEN_DEVICES, &CVADRecorderDlg::OnBnClickedOpenDevices)
	ON_CBN_SELCHANGE(IDC_INPUT_DEVICES, &CVADRecorderDlg::OnCbnSelchangeInputDevices)
	ON_BN_CLICKED(IDC_CLOSE_DEVICES, &CVADRecorderDlg::OnBnClickedCloseDevices)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_MY_THREAD_MESSAGE, OnThreadMessage)

END_MESSAGE_MAP()


// CVADRecorderDlg message handlers

BOOL CVADRecorderDlg::OnInitDialog()
{
	TCHAR	buffer		[MAX_PATH];
	TCHAR	PathBuffer	[MAX_PATH];

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// TODO: Add extra initialization here

	pWaveIn = new CWaveIn();
	pWaveIn->pointer = this;
	if(FillInputDevices()<=0)
	{
		MessageBox(L"NO Input Devices Found.",L"Error",MB_ICONERROR);
		CDialog::OnOK();
	}

	started= false;

	wsprintf(buffer,L"Project VAD Recorder, Version %d.%d.  (www.dspcom.ir)",PROGRAM_MAJOR_VERSION,PROGRAM_MINOR_VERSION);
	SetWindowText(buffer);
	IsNoiseThreadActive = false	;
	Terminated			= true	;
	NoiseThread			= NULL	;
	pReaderQ			= NULL	;
	GetModuleFileName(NULL,PathBuffer,MAX_PATH);
	Path = PathBuffer;
	Path = Path.Left(Path.ReverseFind('\\'));
	pNormalFile = NULL;
	m_Contents.AddString(L"Use following settings:");
	m_Contents.AddString(L"Sampling frequency less than 25 KHz.");
	m_Contents.AddString(L"16 bit per sample mode.");
	m_Contents.AddString(L"Mono mode.");
	m_Recording.SetBkColor(GRAY);
	m_Recording.SetTextColor(BLACK);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVADRecorderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVADRecorderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVADRecorderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



int CVADRecorderDlg::OpenInputDevice(void)
{
	int			deviceNo	;
	int			nT1	=	0	;
	int			iT1 =	0   ;
	CString		csT1		;
	double		dT1=	0.0	;
	MMRESULT	mRes=	0	;
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_INPUT_DEVICES);
	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_INPUT_FORMATS);

	deviceNo = pDevices->GetCurSel();
	if(deviceNo == -1)
		return FAILURE;
	nT1=pFormats->GetCurSel();
	if(nT1==-1)
		return FAILURE;
	pFormats->GetLBText(nT1,csT1);
	sscanf_s((PCHAR)(LPCTSTR)csT1,"%lf",&dT1);
	dT1=dT1*1000;
	sps = (int)dT1;
	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	csT1.Trim();
	if(csT1.Find(L"mono")!=-1)
		channels=1;
	if(csT1.Find(L"stereo")!=-1)
	{
		m_Contents.AddString(L"Only mono mode is supported in this version.");
		//channels=2;
		return FAILURE;
	}
	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	csT1.Trim();
	sscanf_s((PCHAR)(LPCTSTR)csT1,"%d",&iT1);
	bps = iT1;
	if(bps != 16)
	{
		m_Contents.AddString(L"Only 16 bit per sample is supported in this version.");
		return FAILURE;
	}
	if(sps >= 25000)
	{
		m_Contents.AddString(L"Sampling frequency should be less than 25KHz in this version.");
		return FAILURE;
	}
	mRes = pWaveIn->OpenDevice(deviceNo,sps,channels,bps);
	if(mRes!=MMSYSERR_NOERROR)
	{

		return FAILURE;
	}
	return SUCCESS;
}



void CVADRecorderDlg::OnBnClickedOpenDevices()
{
	USES_CONVERSION;
	SYSTEMTIME  stime	;
	CString NormalPath	;
	if(!started)
	{
		//Check to see if input and output devices have the same format.
		if(CheckInputOutputFormat()==false)
		{
			m_Contents.AddString(L"Input device error ...");
			return;
		}
		if(OpenInputDevice()==FAILURE)
		{
			m_Contents.AddString(L"Problem opening input device, returning...");
			return;
		}
		
		if(pWaveIn->StartCapture(CHUNK_RATIO)!= SUCCESS)
		{
			m_Contents.AddString(L"Problem starting input device, returning...");
			return;			
		}
		Terminated = false;

		pReaderQ = new CReaderQ();
		pReaderQ->Connect(pWaveIn->pWriterQ);

		pNormalFile = new CWaveWriter();
		Path += L"\\Recording\\";
		CreateDirectory(Path,0);
		GetSystemTime(&stime);
		NormalPath.Format(L"vad_%d_%d_%d_%d_%d_%d.wav",
			stime.wYear,
			stime.wMonth,
			stime.wDay,
			stime.wHour,
			stime.wMinute,
			stime.wSecond);
		

		NormalPath = Path + NormalPath	;

		m_Contents.AddString(L"VAD file is created at following address :");
		m_Contents.AddString(NormalPath);



		pNormalFile->WaveInit((LPCSTR)T2A(NormalPath),16,8000,1);
		NoiseThread = AfxBeginThread(NOISE_FUNC,this);
		m_Contents.AddString(L"Program activated ...");

		started = true;
		GetDlgItem(IDC_INPUT_DEVICES)->EnableWindow(false);
		GetDlgItem(IDC_INPUT_FORMATS)->EnableWindow(false);
		GetDlgItem(IDC_OPEN_DEVICES)->EnableWindow(false);
	}
	else
	{
		m_Contents.AddString(L"Program is active now");
	}
}

int CVADRecorderDlg::FillInputDevices(void)
{
	CComboBox		*pBox=(CComboBox*)GetDlgItem(IDC_INPUT_DEVICES);
	UINT			nDevices,nC1;
	WAVEINCAPS		stWIC={0};
	MMRESULT		mRes;

	pBox->ResetContent();
	nDevices = pWaveIn->GetDeviceCount();

	for(nC1=0;nC1<nDevices;++nC1)
	{
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes = pWaveIn->GetDeviceCaps(nC1,&stWIC);
		if(mRes==0)
			pBox->AddString(stWIC.szPname);
	}
	if(pBox->GetCount())
	{
		pBox->SetCurSel(0);
		OnCbnSelchangeInputDevices();
	}
	return nDevices;
}

void CVADRecorderDlg::OnCbnSelchangeInputDevices()
{
	// TODO: Add your control notification handler code here
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_INPUT_DEVICES);
	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_INPUT_FORMATS);
	int nSel;
	WAVEINCAPS stWIC={0};
	MMRESULT mRes;

	pFormats->ResetContent();
	nSel=pDevices->GetCurSel();
	if(nSel!=-1)
	{
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes = pWaveIn->GetDeviceCaps(nSel,&stWIC);
		if(mRes==0)
		{
			if(WAVE_FORMAT_1M08==(stWIC.dwFormats&WAVE_FORMAT_1M08))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, mono, 8-bit"),WAVE_FORMAT_1M08);
			if(WAVE_FORMAT_1M16==(stWIC.dwFormats&WAVE_FORMAT_1M16))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, mono, 16-bit"),WAVE_FORMAT_1M16);
			if(WAVE_FORMAT_1S08==(stWIC.dwFormats&WAVE_FORMAT_1S08))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, stereo, 8-bit"),WAVE_FORMAT_1S08);
			if(WAVE_FORMAT_1S16==(stWIC.dwFormats&WAVE_FORMAT_1S16))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, stereo, 16-bit"),WAVE_FORMAT_1S16);
			if(WAVE_FORMAT_2M08==(stWIC.dwFormats&WAVE_FORMAT_2M08))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, mono, 8-bit"),WAVE_FORMAT_2M08);
			if(WAVE_FORMAT_2M16==(stWIC.dwFormats&WAVE_FORMAT_2M16))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, mono, 16-bit"),WAVE_FORMAT_2M16);
			if(WAVE_FORMAT_2S08==(stWIC.dwFormats&WAVE_FORMAT_2S08))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, stereo, 8-bit"),WAVE_FORMAT_2S08);
			if(WAVE_FORMAT_2S16==(stWIC.dwFormats&WAVE_FORMAT_2S16))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, stereo, 16-bit"),WAVE_FORMAT_2S16);
			if(WAVE_FORMAT_4M08==(stWIC.dwFormats&WAVE_FORMAT_4M08))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, mono, 8-bit"),WAVE_FORMAT_4M08);
			if(WAVE_FORMAT_4M16==(stWIC.dwFormats&WAVE_FORMAT_4M16))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, mono, 16-bit"),WAVE_FORMAT_4M16);
			if(WAVE_FORMAT_4S08==(stWIC.dwFormats&WAVE_FORMAT_4S08))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, stereo, 8-bit"),WAVE_FORMAT_4S08);
			if(WAVE_FORMAT_4S16==(stWIC.dwFormats&WAVE_FORMAT_4S16))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, stereo, 16-bit"),WAVE_FORMAT_4S16);
			if(WAVE_FORMAT_96M08==(stWIC.dwFormats&WAVE_FORMAT_96M08))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, mono, 8-bit"),WAVE_FORMAT_96M08);
			if(WAVE_FORMAT_96S08==(stWIC.dwFormats&WAVE_FORMAT_96S08))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, stereo, 8-bit"),WAVE_FORMAT_96S08);
			if(WAVE_FORMAT_96M16==(stWIC.dwFormats&WAVE_FORMAT_96M16))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, mono, 16-bit"),WAVE_FORMAT_96M16);
			if(WAVE_FORMAT_96S16==(stWIC.dwFormats&WAVE_FORMAT_96S16))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, stereo, 16-bit"),WAVE_FORMAT_96S16);
			if(pFormats->GetCount())
				pFormats->SetCurSel(0);
		}
	}
}

void CVADRecorderDlg::OnBnClickedCloseDevices()
{
	// TODO: Add your control notification handler code here
	if(started)
	{
		pWaveIn->StopCapture()	;
		started		= false		;
		Terminated	= true		;
		Sleep(100);		
	}
	OnOK();
}

void CVADRecorderDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	
	while(IsNoiseThreadActive)
		Sleep(100);
	if(pWaveIn)
		delete pWaveIn;
	if(pReaderQ)
		delete pReaderQ;
	if(pNormalFile)
	{
		pNormalFile->WaveClose();
		delete pNormalFile;
		pNormalFile = NULL;
	}

}

bool CVADRecorderDlg::CheckInputOutputFormat(void)
{
	int		nT1		;
	CString csT1	;

	CComboBox *pInFormats  =(CComboBox*)GetDlgItem(IDC_INPUT_FORMATS );

	nT1=pInFormats->GetCurSel();
	if(nT1==-1)
		return false;
	return true;
}

LRESULT CVADRecorderDlg::OnThreadMessage(WPARAM, LPARAM)
{
	if(recording)
	{
		m_Recording.SetBkColor(LIGHTRED);
	}
	else
	{
		m_Recording.SetBkColor(GRAY);
	}
	return 0;
}


void CVADRecorderDlg::NoiseRoutine(void)
{
	IsNoiseThreadActive = true ;
	while(!Terminated)
	{
		while(pReaderQ->GetFilledSize() > NN )
		{
			pReaderQ->RemoveFromQueue((uint8*)NoiseBuffer,NN);
			pNormalFile->WaveWrite((void*)NoiseBuffer,NN);
			// Noise removal procedure:
		}
		Sleep(100);
	}
	IsNoiseThreadActive = false;
}
