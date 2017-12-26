#include "stdafx.h"
#include "Sysark.h"
#include "FileDlgInit.h"
#include "Ioctrls.h"
#include "DataDef.h"

CFileDlgInit::CFileDlgInit()
{

}

CFileDlgInit::~CFileDlgInit()
{
	Clear();
}

VOID CFileDlgInit::Clear()
{
	m_vFileDlg.clear();
}

BOOL CFileDlgInit::ShowList(int nDlg, PVOID pAllInfo, ULONG ulInfoCount, CFileDlg *pFileDlg)
{
	BOOL bRet = FALSE;


	return bRet;
}
