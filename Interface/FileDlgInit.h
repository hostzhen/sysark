// FileDlgInit.h: interface for the CFileDlgInit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEDLGINIT_H__D6ED8B0F_51D0_48D3_97EE_3B0BD7D49CD2__INCLUDED_)
#define AFX_FILEDLGINIT_H__D6ED8B0F_51D0_48D3_97EE_3B0BD7D49CD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FileDlg.h"
#include "Ioctrls.h"
#include "DataDef.h"

class CFileDlgInit  
{
public:
	CFileDlgInit();
	virtual ~CFileDlgInit();

private:
	VOID Clear();
	BOOL ShowList(int nDlg, PVOID pAllInfo, ULONG ulInfoCount, CFileDlg *pFileDlg);

private:
	vector<CFileDlg*> m_vFileDlg;
};

#endif // !defined(AFX_FILEDLGINIT_H__D6ED8B0F_51D0_48D3_97EE_3B0BD7D49CD2__INCLUDED_)
