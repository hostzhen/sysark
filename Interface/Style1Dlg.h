#if !defined(AFX_STYLE1DLG_H__A8DC5263_23E4_42FF_BD50_5F60AD92E12A__INCLUDED_)
#define AFX_STYLE1DLG_H__A8DC5263_23E4_42FF_BD50_5F60AD92E12A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Style1Dlg.h : header file
//

#include "CommonDlg.h"
#include "DlgInit.h"
#include "FirstLittleDlg.h"
#include "FileDlg.h"
#include <vector>
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CStyle1Dlg dialog

class CStyle1Dlg : public CDialog
{
// Construction
public:
	CStyle1Dlg(CWnd* pParent = NULL, HANDLE hDevice = INVALID_HANDLE_VALUE);   // standard constructor

	~CStyle1Dlg()
	{
		Clear();
	}

// Dialog Data
	//{{AFX_DATA(CStyle1Dlg)
	enum { IDD = IDD_STYLE_1_DLG };
	CTabCtrl	m_tab;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStyle1Dlg)
	public:
	virtual BOOL DestroyWindow();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStyle1Dlg)
	afx_msg void OnSelchangeStyle1Tab(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnReflash();
	afx_msg void OnKillProcess();
	afx_msg void OnForceKillProcess();
	afx_msg void OnProcessModule();
	afx_msg void OnProcessThread();
	afx_msg void OnForceKillProcessAndDeleteFile();
	afx_msg void OnPaint();
	afx_msg void OnUnloadDriver();
	afx_msg void OnGotoFile();
	afx_msg void OnExportToFile();
	afx_msg void OnFilePropety();
	afx_msg void OnDeleteFile();
	afx_msg void OnCopyName();
	afx_msg void OnCopyPath();
	afx_msg void OnOnlineSearchName();
	afx_msg void OnOnlineAnalysis();
	afx_msg void OnUnhookSsdt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	VOID Clear();
	VOID ShowDlg(int nDlg);
	
public:
	BOOL InitChildDialog(int nDlg, LPCTSTR* pItemArray, DlgLevel dlgLevel);


private:
	vector<CCommonDlg*> m_vChildDlg;
	CDlgInit			*m_pDlgInit;
	CStyle1Dlg			*m_pStyle1Dlg;
	CFileDlg			*m_pFileDlg, *m_pRegDlg;
	DlgLevel			m_DlgLevel;
	HANDLE				m_hDevSrv;
	HWND				m_hParentWnd;
	int					m_CurSel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STYLE1DLG_H__A8DC5263_23E4_42FF_BD50_5F60AD92E12A__INCLUDED_)
