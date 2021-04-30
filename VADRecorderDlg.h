// VADRecorderDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "general.h"
#include "WaveWriter.h"
#include "ColorEdit.h"
#include "wavein.h"
#include "resource.h"


// CVADRecorderDlg dialog
//class CWaveIn;
//class CWaveOut;
class CReaderQ;
class CVADRecorderDlg : public CDialog
{
// Construction
public:
	CVADRecorderDlg(CWnd* pParent = NULL);	// standard constructor
	void		 NoiseRoutine(void);

// Dialog Data
	enum { IDD = IDD_VADRecorder_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX)	;	// DDX/DDV support
// Implementation
protected:
	HICON m_hIcon;
	LRESULT OnThreadMessage(WPARAM, LPARAM);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	short			NoiseBuffer[NN]				;
	CString			Path						;
	bool			started						;
	CWaveIn			*pWaveIn					;
	int				OpenInputDevice(void)		;
	afx_msg void	OnBnClickedOpenDevices()	;
	int				FillInputDevices(void)		;
	afx_msg void	OnCbnSelchangeInputDevices();
	afx_msg void	OnBnClickedCloseDevices()	;
	afx_msg void	OnDestroy()					;
	bool			CheckInputOutputFormat(void);
	CListBox		m_Contents					;
	CWinThread*		NoiseThread					;
	bool			IsNoiseThreadActive			;
	bool			Terminated					;
	CReaderQ		*pReaderQ					;
	CWaveWriter		*pNormalFile				;
	DWORD			sps							;
	WORD			channels					;
	WORD			bps							;
	CColorEdit		m_Recording					;
	bool			recording					;
public: 
	void SetRecording(bool a){recording = a;}
};
