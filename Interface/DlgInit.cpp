// DlgInit.cpp: implementation of the CDlgInit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Sysark.h"
#include "DlgInit.h"
#include "Ioctrls.h"
#include "DataDef.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgInit::CDlgInit(HANDLE hDevice, HWND hParentWnd, vector<CCommonDlg*> &vComDlg, DlgLevel dlgLevel)
: m_hDevice(hDevice)
, m_hParentWnd(hParentWnd)
, m_DlgLevel(dlgLevel)
{
	m_vDlgs.assign(vComDlg.begin(), vComDlg.end());
	char OpenName[MAX_PATH+1];
	sprintf( OpenName, "0x%08x", m_hDevice );
	MessageBox(NULL, OpenName, "", MB_OK);

}

CDlgInit::~CDlgInit()
{
	Clear();
}

VOID CDlgInit::Clear()
{
	m_vDlgs.clear();
}

UINT CDlgInit::InitDialogThreadFunc(LPVOID pParam )
{
	ASSERT(pParam != NULL);

	CCommonDlg *pCommonDlg;
	CDlgInit *pDlgInit;
	int nDlg;

	nDlg = (int)((PULONG)pParam)[0];
	pCommonDlg = (CCommonDlg *)((PULONG)pParam)[1];
	pDlgInit = (CDlgInit *)((PULONG)pParam)[2];
	pCommonDlg->DeleteListAllItems();
	
	switch (pDlgInit->m_DlgLevel)
	{
	case FirstLevel://第一层
		{
			switch (nDlg)
			{
			case 0://PROCESS_DLG:
				pDlgInit->InitEnumProcessDlg(pCommonDlg);
				break;
			case 1://DRIVER_MODULE_DLG:
				pDlgInit->InitDriverModuleDlg(pCommonDlg);
				break;
			case 2:
				break;
			case 3://REGISTRY_DLG:
				break;
			case 4://FILE_DLG:
				break;
			case 5://STARTUP_DLG:
				pDlgInit->InitStartupDlg(pCommonDlg);
				break;
			case 6://SERVICES_DLG:
				pDlgInit->InitEnumServicesDlg(pCommonDlg);
				break;
			default:
				break;
			}
		}
		break;
	case KernelHookLevel://KernelHook 下的一层
		{
			switch (nDlg)
			{
			case 0://KernelHook_SSDT_DLG
				pDlgInit->InitSSDTDlg(pCommonDlg);
				break;
			case 1://KernelHook_ShadowSSDT_DLG
				pDlgInit->InitShadowSSDTDlg(pCommonDlg);
				break;
			default:
				break;
			}
			
		}
		break;
	default:
		break;
	}
	
	
	pCommonDlg->UpdateStatusBar(nDlg, pDlgInit->m_hParentWnd);
	delete []pParam;
	return 1;
}

//初始化各个子DLG
BOOL CDlgInit::InitDialog(int nDlg, CCommonDlg *pCommonDlg)
{
	ASSERT(nDlg >= 0 && pCommonDlg != NULL);
	BOOL bRet = FALSE;
	PULONG lpParam = new ULONG[3];
	lpParam[0] = (ULONG)nDlg;
	lpParam[1] = (ULONG)pCommonDlg;
	lpParam[2] = (ULONG)this;
	//HWND hWnd = pCommonDlg->m_pListCtrl->m_hWnd;
	//AfxBeginThread(InitDialogThreadFunc, (LPVOID)lpParam);
	InitDialogThreadFunc((LPVOID)lpParam);
	
	return bRet;
}

BOOL CDlgInit::ShowList( int nDlg, PVOID pAllInfo, ULONG ulInfoCount, CCommonDlg *pCommonDlg)
{
	ASSERT(nDlg >= 0 && NULL != pAllInfo && ulInfoCount >= 0);

	CHAR lpText[MAX_PATH] = {0};

	switch (nDlg)
	{
	case PROCESS_DLG://进程
		{
			PALL_PROCESSES_INFO pProcessInfo = (PALL_PROCESSES_INFO)pAllInfo;
			for (int i = 0; i < pProcessInfo->uCount; i++)
			{
				pCommonDlg->SetListItemText(i, 0, pProcessInfo->vProcessInf[i].ImageFileName);
				sprintf(lpText, "%d", pProcessInfo->vProcessInf[i].ProcessId);
				pCommonDlg->SetListItemText(i, 1, lpText);
				sprintf(lpText, "%d", pProcessInfo->vProcessInf[i].FatherProcessId);
				pCommonDlg->SetListItemText(i, 2, lpText);
				wcharTochar(pProcessInfo->vProcessInf[i].ImagePathName, lpText, MAX_PATH);
				pCommonDlg->SetListItemText(i, 3, lpText);
				sprintf(lpText, "0x%08x", pProcessInfo->vProcessInf[i].pEProcess);
				pCommonDlg->SetListItemText(i, 4, lpText);
				pCommonDlg->SetListItemColor(i, pProcessInfo->vProcessInf[i].bHide);
				
			}
		}
		break;
	case DRIVER_MODULE_DLG://驱动模块
		{
			PDRIVER_MODULE_INFO pDriverModuleInfo = (PDRIVER_MODULE_INFO)pAllInfo;
			for (int i = 0; i < ulInfoCount; i++)
			{
				if (pDriverModuleInfo != NULL)
				{
					wcharTochar(pDriverModuleInfo->BaseDllName, lpText, MAX_PATH);
					pCommonDlg->SetListItemText(i, 0, lpText);
					sprintf(lpText, "0x%08x", pDriverModuleInfo->DllBase);
					pCommonDlg->SetListItemText(i, 1, lpText);
					sprintf(lpText, "0x%08x", pDriverModuleInfo->SizeOfImage);
					pCommonDlg->SetListItemText(i, 2, lpText);
					wcharTochar(pDriverModuleInfo->FullDllName, lpText, MAX_PATH);
					pCommonDlg->SetListItemText(i, 4, lpText);
					sprintf(lpText, "%d", i);
					pCommonDlg->SetListItemText(i, 6, lpText);

				}
				pDriverModuleInfo++;
			}
		}
		break;
	case SSDT_DLG://SSDT
		{
			PSSDT_FUNC_INFO pSSDTFuncInfo = (PSSDT_FUNC_INFO)pAllInfo;
			for (int i = 0; i < ulInfoCount; i++)
			{
				if (NULL != pSSDTFuncInfo)
				{
					sprintf(lpText, "%d", pSSDTFuncInfo->ulIndex);
					pCommonDlg->SetListItemText(i, 0, lpText);
					pCommonDlg->SetListItemText(i, 1, pSSDTFuncInfo->lpwzFuncName);
					sprintf(lpText, "0x%08x", pSSDTFuncInfo->ulCurrenAddress);
					pCommonDlg->SetListItemText(i, 2, lpText);
					if (pSSDTFuncInfo->ulOriginalAddress != pSSDTFuncInfo->ulOriginalAddress)
					{
						pCommonDlg->SetListItemText(i, 3, "ssdt hook");
						pCommonDlg->SetListItemColor(i, 1);
					}
					sprintf(lpText, "0x%08x", pSSDTFuncInfo->ulOriginalAddress);
					pCommonDlg->SetListItemText(i, 4, lpText);
					pCommonDlg->SetListItemText(i, 5, pSSDTFuncInfo->lpwzModulePath);
				}
				pSSDTFuncInfo++;
			}
		}
		break;
	case ShadowSSDT_DLG://ShadowSSDT
		{
			PSSDT_FUNC_INFO pShadowSSDTFuncInfo = (PSSDT_FUNC_INFO)pAllInfo;
			for (int i = 0; i < ulInfoCount; i++)
			{
				if (NULL != pShadowSSDTFuncInfo)
				{
					sprintf(lpText, "%d", pShadowSSDTFuncInfo->ulIndex);
					pCommonDlg->SetListItemText(i, 0, lpText);
					pCommonDlg->SetListItemText(i, 1, pShadowSSDTFuncInfo->lpwzFuncName);
					sprintf(lpText, "0x%08x", pShadowSSDTFuncInfo->ulCurrenAddress);
					pCommonDlg->SetListItemText(i, 2, lpText);
					if (pShadowSSDTFuncInfo->ulOriginalAddress != pShadowSSDTFuncInfo->ulOriginalAddress)
					{
						pCommonDlg->SetListItemText(i, 3, "ssdt hook");
						pCommonDlg->SetListItemColor(i, 1);
					}
					sprintf(lpText, "0x%08x", pShadowSSDTFuncInfo->ulOriginalAddress);
					pCommonDlg->SetListItemText(i, 4, lpText);
					pCommonDlg->SetListItemText(i, 5, pShadowSSDTFuncInfo->lpwzModulePath);
				}
				pShadowSSDTFuncInfo++;
			}
		}
		break;
	case REGISTRY_DLG:
		{

		}
		break;
	case FILE_DLG:
		{


		}
		break;
	case STARTUP_DLG:
		{
			PSTARTUP_INFO pStartupInfo = (PSTARTUP_INFO)pAllInfo;
			for (int i = 0; i < ulInfoCount; i++)
			{
				wcharTochar(pStartupInfo->lpwzName, lpText, MAX_PATH);
				pCommonDlg->SetListItemText(i, 0, lpText);
				wcharTochar(pStartupInfo->lpwzKeyPath, lpText, MAX_PATH);
				pCommonDlg->SetListItemText(i, 2, lpText);
				pStartupInfo++;
			}

		}
		break;
	case SERVICES_DLG://服务
		{
			PSERVICES_INFO pServicesInfo = (PSERVICES_INFO)pAllInfo;
			for (int i = 0; i < ulInfoCount; i++)
			{
				wcharTochar(pServicesInfo->lpwzSrvName, lpText, MAX_PATH);
				pCommonDlg->SetListItemText(i, 0, lpText);
				wcharTochar(pServicesInfo->lpwzBootType, lpText, MAX_PATH);
				pCommonDlg->SetListItemText(i, 2, lpText);
				wcharTochar(pServicesInfo->lpwzDescription, lpText, MAX_PATH);
				pCommonDlg->SetListItemText(i, 3, lpText);
				wcharTochar(pServicesInfo->lpwzImageName, lpText, MAX_PATH);
				pCommonDlg->SetListItemText(i, 4, lpText);
				pServicesInfo++;
			}
			
		}
		break;
	default:
		break;
	}

	pCommonDlg->UpdateStatusBar(0, m_hParentWnd);


	return TRUE;
}


//初始化枚举进程DLG
BOOL CDlgInit::InitEnumProcessDlg(CCommonDlg *pCommonDlg)
{
	ASSERT(NULL != pCommonDlg);

	ALL_PROCESSES_INFO AllProcessInfo;
	DWORD dwRet;

	BOOL bRet = DeviceIoControl(m_hDevice, IOCTL_ENUM_PROCESS, NULL, NULL,\
		&AllProcessInfo, sizeof(ALL_PROCESSES_INFO), &dwRet, NULL);

	if (bRet)
	{
		ShowList(PROCESS_DLG, &AllProcessInfo, NULL, pCommonDlg);
	}


	return bRet;
}

BOOL CDlgInit::InitEnumServicesDlg(CCommonDlg *pCommonDlg)
{
	ASSERT(NULL != pCommonDlg);
	BOOL bRet = FALSE;
	ULONG ulCount = 0;
	PSERVICES_INFO pServicesInfo = NULL;
	DWORD dwRet;
	
	bRet = DeviceIoControl(m_hDevice, IOCTL_GET_SERVICES_COUNT, NULL, NULL,\
		&ulCount, sizeof(ULONG), &dwRet, NULL);
	
	if (bRet)
	{
		pServicesInfo = (PSERVICES_INFO)malloc(ulCount * sizeof(SERVICES_INFO));
		ZeroMemory(pServicesInfo, ulCount * sizeof(SERVICES_INFO));
		
		bRet = DeviceIoControl(m_hDevice, IOCTL_ENUM_SERVICES, NULL, NULL,\
			pServicesInfo, ulCount * sizeof(SERVICES_INFO), &dwRet, NULL);
		
		
		if (bRet)
		{
			ShowList(SERVICES_DLG, pServicesInfo, ulCount, pCommonDlg);
		}
		free(pServicesInfo);
		pServicesInfo = NULL;
	}
	
	return bRet;
	
}

//初始化驱动模块DLG
BOOL CDlgInit::InitDriverModuleDlg(CCommonDlg *pCommonDlg)
{
	ASSERT(NULL != pCommonDlg);
	BOOL bRet = FALSE;
	ULONG ulCount = 0;
	PDRIVER_MODULE_INFO pDriverModuleInfo = NULL;
	DWORD dwRet;
	
	bRet = DeviceIoControl(m_hDevice, IOCTL_GET_DRIVER_MODULE_INFO_SIZE, NULL, NULL,\
		&ulCount, sizeof(ULONG), &dwRet, NULL);
	
	if (bRet)
	{
		pDriverModuleInfo = (PDRIVER_MODULE_INFO)malloc(ulCount * sizeof(DRIVER_MODULE_INFO));
		ZeroMemory(pDriverModuleInfo, ulCount * sizeof(DRIVER_MODULE_INFO));

		bRet = DeviceIoControl(m_hDevice, IOCTL_ENUM_DRIVER_MODULE, NULL, NULL,\
			pDriverModuleInfo, ulCount * sizeof(DRIVER_MODULE_INFO), &dwRet, NULL);

		
		if (bRet)
		{
			ShowList(DRIVER_MODULE_DLG, pDriverModuleInfo, ulCount, pCommonDlg);
		}
		free(pDriverModuleInfo);
		pDriverModuleInfo = NULL;
	}

	return bRet;
}

//初始化SSDT DLG
BOOL CDlgInit::InitSSDTDlg(CCommonDlg *pCommonDlg)
{
	ASSERT(NULL != pCommonDlg);
	BOOL bRet = FALSE;
	ULONG ulCount = 0;
	PSSDT_FUNC_INFO pSSDTInfo = NULL;
	DWORD dwRet;
	
	bRet = DeviceIoControl(m_hDevice, IOCTL_GET_SSDT_COUNT, NULL, NULL,\
		&ulCount, sizeof(ULONG), &dwRet, NULL);
	
	if (bRet)
	{
		pSSDTInfo = (PSSDT_FUNC_INFO)malloc(ulCount * sizeof(SSDT_FUNC_INFO));
		ZeroMemory(pSSDTInfo, ulCount * sizeof(SSDT_FUNC_INFO));
		
		bRet = DeviceIoControl(m_hDevice, IOCTL_ENUM_SSDT, NULL, NULL,\
			pSSDTInfo, ulCount * sizeof(SSDT_FUNC_INFO), &dwRet, NULL);
		
		
		if (bRet)
		{
			ShowList(SSDT_DLG, pSSDTInfo, ulCount, pCommonDlg);
		}
		free(pSSDTInfo);
		pSSDTInfo = NULL;
	}
	return bRet;
}

BOOL CDlgInit::InitShadowSSDTDlg(CCommonDlg *pCommonDlg)
{
	ASSERT(NULL != pCommonDlg);
	BOOL bRet = FALSE;
	ULONG ulCount = 0;
	PSSDT_FUNC_INFO pShadowSSDTInfo = NULL;
	DWORD dwRet;
	
	bRet = DeviceIoControl(m_hDevice, IOCTL_GET_SHADOW_HOOK_COUNT, NULL, NULL,\
		&ulCount, sizeof(ULONG), &dwRet, NULL);
	
	if (bRet)
	{
		pShadowSSDTInfo = (PSSDT_FUNC_INFO)malloc(ulCount * sizeof(SSDT_FUNC_INFO));
		ZeroMemory(pShadowSSDTInfo, ulCount * sizeof(SSDT_FUNC_INFO));
		
		bRet = DeviceIoControl(m_hDevice, IOCTL_ENUM_SHADOW_HOOK, NULL, NULL,\
			pShadowSSDTInfo, ulCount * sizeof(SSDT_FUNC_INFO), &dwRet, NULL);
		
		
		if (bRet)
		{
			ShowList(SSDT_DLG, pShadowSSDTInfo, ulCount, pCommonDlg);
		}
		free(pShadowSSDTInfo);
		pShadowSSDTInfo = NULL;
	}
	return bRet;
}

BOOL CDlgInit::InitStartupDlg(CCommonDlg *pCommonDlg)
{
	ASSERT(NULL != pCommonDlg);
	BOOL bRet = FALSE;
	ULONG ulCount = 0;
	PSTARTUP_INFO pStartupInfo = NULL;
	DWORD dwRet;
	
	bRet = DeviceIoControl(m_hDevice, IOCTL_GET_STARTUP_INFO_COUNT, NULL, NULL,\
		&ulCount, sizeof(ULONG), &dwRet, NULL);
	
	if (bRet)
	{
		pStartupInfo = (PSTARTUP_INFO)malloc(ulCount * sizeof(STARTUP_INFO));
		ZeroMemory(pStartupInfo, ulCount * sizeof(STARTUP_INFO));
		
		bRet = DeviceIoControl(m_hDevice, IOCTL_ENUM_STARTUP, NULL, NULL,\
			pStartupInfo, ulCount * sizeof(STARTUP_INFO), &dwRet, NULL);
		
		
		if (bRet)
		{
			ShowList(STARTUP_DLG, pStartupInfo, ulCount, pCommonDlg);
		}
		free(pStartupInfo);
		pStartupInfo = NULL;
	}
	return bRet;
}

