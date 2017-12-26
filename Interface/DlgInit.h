// DlgInit.h: interface for the CDlgInit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLGINIT_H__C5CD43A3_6B64_4DE0_A3B9_C4CBF5C21155__INCLUDED_)
#define AFX_DLGINIT_H__C5CD43A3_6B64_4DE0_A3B9_C4CBF5C21155__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CommonDlg.h"
#include "Ioctrls.h"
#include "DataDef.h"

class CDlgInit  
{
public:
	CDlgInit(HANDLE hDevice, HWND hParentWnd, vector<CCommonDlg*> &vComDlg, DlgLevel dlgLevel);
	virtual ~CDlgInit();
private:
	CDlgInit();
	CDlgInit(HANDLE hDevice);
public:
	BOOL InitDialog(int nDlg, CCommonDlg *pCommonDlg);
	
private:
	static UINT __cdecl InitDialogThreadFunc( LPVOID pParam );
	BOOL InitEnumServicesDlg(CCommonDlg *pCommonDlg);
	BOOL InitEnumProcessDlg(CCommonDlg *pCommonDlg);
	BOOL InitEnumProcessModuleDlg(CCommonDlg *pCommonDlg);
	BOOL InitDriverModuleDlg(CCommonDlg *pCommonDlg);
	BOOL InitSSDTDlg(CCommonDlg *pCommonDlg);
	BOOL InitShadowSSDTDlg(CCommonDlg *pCommonDlg);
	BOOL InitStartupDlg(CCommonDlg *pCommonDlg);
	VOID Clear();


	BOOL ShowList(int nDlg, PVOID pAllInfo, ULONG ulInfoCount, CCommonDlg *pCommonDlg);
private:
	HANDLE m_hDevice;
	HWND m_hParentWnd;
	DlgLevel m_DlgLevel;
	vector<CCommonDlg*> m_vDlgs;
};

#endif // !defined(AFX_DLGINIT_H__C5CD43A3_6B64_4DE0_A3B9_C4CBF5C21155__INCLUDED_)
