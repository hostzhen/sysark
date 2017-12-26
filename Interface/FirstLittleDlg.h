#if !defined(AFX_FIRSTLITTLEDLG_H__642F01C1_31E0_4A39_9D26_255FABC7E163__INCLUDED_)
#define AFX_FIRSTLITTLEDLG_H__642F01C1_31E0_4A39_9D26_255FABC7E163__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FirstLittleDlg.h : header file
//

#include "Import/tinystr.h"
#include "Import/tinyxml.h"
#include "Control/ReportCtrl.h"
#include "DataDef.h"
#include <vector>
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CFirstLittleDlg dialog

class CFirstLittleDlg : public CDialog
{
// Construction
public:
	CFirstLittleDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFirstLittleDlg)
	enum { IDD = IDD_FIRST_LITTLE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFirstLittleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFirstLittleDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	VOID InitListColumn(int nDlg);
	VOID SetListItemText(int nItem, int nSubItem, LPCSTR lpszText);
	VOID SetDlgInitData(int nDlg, LittleDlgType dlgType, PVOID pInitData);
private:
	void InitListCtrl();

private:
	TiXmlDocument *m_pDoc;	// 定义一个TiXmlDocument类指针
	CReportCtrl	*m_pListCtrl;
	vector<FLOAT> m_vColumnsWidth;
	int m_nDlg;
	LittleDlgType m_dlgType;
	PVOID m_pInitData;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIRSTLITTLEDLG_H__642F01C1_31E0_4A39_9D26_255FABC7E163__INCLUDED_)
