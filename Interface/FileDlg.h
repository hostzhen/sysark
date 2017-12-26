#if !defined(AFX_FILEDLG_H__B7DE24C5_EA39_4EA6_A410_D346BBB09EA1__INCLUDED_)
#define AFX_FILEDLG_H__B7DE24C5_EA39_4EA6_A410_D346BBB09EA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileDlg.h : header file
//
#include "Import/tinystr.h"
#include "Import/tinyxml.h"
#include "Control/ReportCtrl.h"
#include "DataDef.h"
#include "Ioctrls.h"
#include "Registry/RegEditEx.h"
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
using namespace std;
/////////////////////////////////////////////////////////////////////////////
// CFileDlg dialog

class CFileDlg : public CDialog
{
// Construction
public:
	CFileDlg(HANDLE hDevice, CWnd* pParent = NULL);   // standard constructor
	~CFileDlg()
	{
		Clear();
	}

// Dialog Data
	//{{AFX_DATA(CFileDlg)
	enum { IDD = IDD_FILE_DLG };
	CTreeCtrl	m_TreeCtrl;
	CReportCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileDlg)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFileDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnItemexpandingFileTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangingFileTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnListCtrlMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTreeCtrlMsg(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL InitTreeView();
	VOID InitListColumn( LPCTSTR lpDlgTitle );
	VOID SetListItemText(int nItem, int nSubItem, LPCTSTR lpszText);
	VOID DeleteListAllItems();

private:
	VOID Clear();
	void InitListCtrl();
	void ItemExpandingFileTreeForFile(NMHDR* pNMHDR, LRESULT* pResult);
	void ItemExpandingFileTreeForRegistry(NMHDR* pNMHDR, LRESULT* pResult);
	BOOL IsMediaValid(CString &strPathName);
	HTREEITEM GetDriveNode(HTREEITEM hItem);
	UINT DeleteChildren(HTREEITEM hItem);
	BOOL IsDriveNode(HTREEITEM hItem);
	void AddDummyNode(HTREEITEM hItem);
	BOOL IsPathValid(CString &strPathName);
	CString GetPathFromItem(HTREEITEM hItem);
	BOOL HasSubdirectory(CString &strPathName);
	BOOL AddDrives(CString strDrive, HTREEITEM hParent);
	BOOL AddSubKey(CString strSubKey, HTREEITEM hParent);
	void SetButtonState(HTREEITEM hItem, CString &strPathName);
	void AddToListView(PDIRECTORY_INFO_EX fd);
	UINT AddDirectoryNodes(HTREEITEM hItem, CString &strPathName);

private:
	enum DLG_TYPE{
		DLG_TYPE_UNKNOWN = -1,
		DLG_TYPE_FILE,		//文件
		DLG_TYPE_REGISTRY	//注册表
	};
	HANDLE m_hDevice;
	TiXmlDocument *m_pDoc;	// 定义一个TiXmlDocument类指针
	CRegEditEx *m_pRegEditEx;
	vector<FLOAT> m_vColumnsWidth;
	DLG_TYPE m_dlgType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEDLG_H__B7DE24C5_EA39_4EA6_A410_D346BBB09EA1__INCLUDED_)
