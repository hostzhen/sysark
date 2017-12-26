// SysarkDlg.h : header file
//

#if !defined(AFX_SYSARKDLG_H__557C3B0A_F525_4D0D_A704_C97C934A81A0__INCLUDED_)
#define AFX_SYSARKDLG_H__557C3B0A_F525_4D0D_A704_C97C934A81A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Style1Dlg.h"
#include "LoadServer.h"
#include <vector>
using namespace std;
/////////////////////////////////////////////////////////////////////////////
// CSysarkDlg dialog

class CSysarkDlg : public CDialog
{
// Construction
public:
	CSysarkDlg(CWnd* pParent = NULL);	// standard constructor
	~CSysarkDlg()
	{
		Clear();
	}

// Dialog Data
	//{{AFX_DATA(CSysarkDlg)
	enum { IDD = IDD_SYSARK_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSysarkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSysarkDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnStatusMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CStyle1Dlg *m_pStyle1Dlg;
	HANDLE m_hDevSrv;
	CStatusBar m_wndStatusBar;

private:
	BOOL InitChildDialog();
	VOID Clear(VOID);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSARKDLG_H__557C3B0A_F525_4D0D_A704_C97C934A81A0__INCLUDED_)
