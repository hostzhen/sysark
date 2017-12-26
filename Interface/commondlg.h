#if !defined(AFX_COMMONDLG_H__2F5BAF64_7059_4791_B5BC_B7BCBB9195BD__INCLUDED_)
#define AFX_COMMONDLG_H__2F5BAF64_7059_4791_B5BC_B7BCBB9195BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommonDlg.h : header file
//
#include "Import/tinystr.h"
#include "Import/tinyxml.h"
#include "Control/ReportCtrl.h"
#include "DataDef.h"
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CCommonDlg dialog

class CCommonDlg : public CDialog
{
// Construction
public:
	CCommonDlg(CWnd* pParent = NULL, UINT nIDResource = NULL);   // standard constructor
	~CCommonDlg()
	{
		if (NULL != m_pDoc)
		{
			delete m_pDoc;
			m_pDoc = NULL;
		}
	}

// Dialog Data
	//{{AFX_DATA(CCommonDlg)
	enum { IDD = IDD_COMMON_DLG };
	CReportCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommonDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCommonDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnNMRclickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	INT GetSelectItemPid();
	VOID GetSelectItemFilePath(LPSTR lpFilePath, ULONG ulMaxPath);
	VOID GetSelectItemValue(LPSTR lpSrvName, ULONG ulMaxName, LPSTR lpListName);
	VOID InitListColumn(LPCTSTR lpDlgTitle);
	VOID SetListItemText(int nItem, int nSubItem, LPCTSTR lpszText);
	VOID SetListItemColor(int nItem, int nColFlag);
	VOID DeleteListAllItems();
	VOID UpdateStatusBar(int nDlg, HWND hWnd);
private:
	void InitListCtrl();
	void GBKToUTF8(char* &szOut);
	void UTF8ToGBK( char *&szOut );

private:
	TiXmlDocument *m_pDoc;	// 定义一个TiXmlDocument类指针
	vector<FLOAT> m_vColumnsWidth;
	UINT m_nIDLButtonMenu;
	UINT m_nMenuItem;
	UINT m_nSelectItem;
public:
	//CReportCtrl	*m_pListCtrl;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMONDLG_H__2F5BAF64_7059_4791_B5BC_B7BCBB9195BD__INCLUDED_)
