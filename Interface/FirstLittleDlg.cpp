// FirstLittleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sysark.h"
#include "FirstLittleDlg.h"
#include "DataDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDD_COMMON_LSIT		WM_USER+10
/////////////////////////////////////////////////////////////////////////////
// CFirstLittleDlg dialog


CFirstLittleDlg::CFirstLittleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFirstLittleDlg::IDD, pParent)
	, m_pListCtrl(NULL)
	, m_pDoc(NULL)
	, m_nDlg(-1)
	, m_dlgType(UNKNOWN_DLG)
	, m_pInitData(NULL)
{
	//{{AFX_DATA_INIT(CFirstLittleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	if (NULL == m_pDoc)
	{
		m_pDoc = new TiXmlDocument;
		if (NULL == m_pDoc)
		{
			MessageBoxA(" new TiXmlDocument failed!", "ERROR", MB_OK);
		}
	}
}


void CFirstLittleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFirstLittleDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFirstLittleDlg, CDialog)
	//{{AFX_MSG_MAP(CFirstLittleDlg)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFirstLittleDlg message handlers


BOOL CFirstLittleDlg::OnInitDialog()
{
	CHAR lpText[MAX_PATH] = {0};
	CDialog::OnInitDialog();
	
	InitListCtrl();

	InitListColumn(m_nDlg);

	switch (m_dlgType)
	{
	case PROCESS_MODULE_DLG:
		{
			ASSERT(m_pInitData != NULL);
			PALL_PROCESS_MODULE_INFO pAllProcModuleInf = (PALL_PROCESS_MODULE_INFO)m_pInitData;
			for (int i = 0; i < pAllProcModuleInf->uCount; i++)
			{	
				wcharTochar(pAllProcModuleInf->vModuleInf[i].ImagePathName, lpText, MAX_PATH);
				SetListItemText(i, 0, lpText);
				sprintf(lpText, "0x%08x", pAllProcModuleInf->vModuleInf[i].BaseAddress);
				SetListItemText(i, 1, lpText);
				sprintf(lpText, "0x%08x", pAllProcModuleInf->vModuleInf[i].size);
				SetListItemText(i, 2, lpText);
			}
		}
		break;
	case PROCESS_THREAD_DLG:
		{
			ASSERT(m_pInitData != NULL);
			PALL_PROCESS_THREAD_INFO pAllProcThreadInf = (PALL_PROCESS_THREAD_INFO)m_pInitData;
			for (int i = 0; i < pAllProcThreadInf->uCount; i++)
			{	
				sprintf(lpText, "%d", pAllProcThreadInf->vThreadInf[i].ThreadID);
				SetListItemText(i, 0, lpText);
				sprintf(lpText, "0x%08x", pAllProcThreadInf->vThreadInf[i].EThread);
				SetListItemText(i, 1, lpText);
				sprintf(lpText, "0x%08x", pAllProcThreadInf->vThreadInf[i].Teb);
				SetListItemText(i, 2, lpText);
				sprintf(lpText, "%d", pAllProcThreadInf->vThreadInf[i].ulPriority);
				SetListItemText(i, 3, lpText);
				sprintf(lpText, "0x%08x", pAllProcThreadInf->vThreadInf[i].ThreadStart);
				SetListItemText(i, 4, lpText);
				//wcharTochar( pAllProcThreadInf->vThreadInf[i].lpszThreadModule, lpText, MAX_PATH);
				//SetListItemText(i, 5, pAllProcThreadInf->vThreadInf[i].lpszThreadModule);
				sprintf(lpText, "%d", pAllProcThreadInf->vThreadInf[i].ulContextSwitches);
				SetListItemText(i, 5, lpText);
				//sprintf(lpText, "%d", pAllProcThreadInf->vThreadInf[i].ulPriority);
				//SetListItemText(i, 5, lpText);
			}
		}
		break;
	default:
		break;
	}
	UpdateWindow();
	
	return TRUE;
}



//创建list ctrl
void CFirstLittleDlg::InitListCtrl()
{
	CRect rect;

	if (NULL == m_pListCtrl)
	{
		GetClientRect(&rect);
		m_pListCtrl = new CReportCtrl();
		m_pListCtrl->Create(
			this,
			IDD_COMMON_LSIT, rect, WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_EDITLABELS); 
		m_pListCtrl->SetExtendedStyle(m_pListCtrl->GetExtendedStyle() | LVS_EX_GRIDLINES);
		
		m_pListCtrl->MoveWindow(&rect);
	}
}

VOID CFirstLittleDlg::SetListItemText( int nItem, int nSubItem, LPCSTR lpszText )
{
	ASSERT(::IsWindow(m_pListCtrl->m_hWnd));
	
	if(m_pListCtrl->GetItemCount() <= nItem)
	{
		m_pListCtrl->InsertItem(nItem, lpszText, NULL);
	}
	else
	{
		m_pListCtrl->SetItemText(nItem, nSubItem, lpszText);
	}
}

VOID CFirstLittleDlg::InitListColumn( int nDlg )
{
	string strName;
	int nOrdinal = -1, nColumns = 0;
	if (NULL != m_pDoc)
	{
		char szFName[MAX_PATH];
		if (m_pDoc->LoadFile(CONFIG_FILE_NAME))
		{
			TiXmlElement *pRootElement = m_pDoc->RootElement();
			if (NULL != pRootElement)
			{
				TiXmlElement *pSourceElement = pRootElement->FirstChildElement();
				//循环遍历所有参数的信息
				while (pSourceElement)
				{
					strName.erase(strName.begin(), strName.end());//strName.clear()
					strName = pSourceElement->Value();
					m_vColumnsWidth.clear();
					if (!strcmp(strName.c_str(), "Dlalog"))
					{
						TiXmlAttribute *pAttrOfSource = pSourceElement->FirstAttribute();
						//循环遍历所有信息
						while (pAttrOfSource)
						{
							if (!strcmp(pAttrOfSource->Name(), "ordinal"))
							{
								nOrdinal = atoi(pAttrOfSource->Value());
								if (nDlg != nOrdinal)
								{
									break;
								}
							}
							else if (!strcmp(pAttrOfSource->Name(), "label"))
							{
								strName = pAttrOfSource->Value();
							}
							else if (!strcmp(pAttrOfSource->Name(), "num_columns"))
							{
								nColumns = atoi(pAttrOfSource->Value());
							}
							pAttrOfSource = pAttrOfSource->Next();
						}

						if (nDlg == nOrdinal)
						{
							FLOAT fWidth = 0.0;
							CRect rect;
							GetClientRect(&rect);

							pSourceElement = pSourceElement->FirstChildElement();
							while (pSourceElement)
							{
								pAttrOfSource = pSourceElement->FirstAttribute();
								
								//循环遍历list控件中的每一列基本信息
								while (pAttrOfSource)
								{
									if (!strcmp(pAttrOfSource->Name(), "ordinal"))
									{
										nOrdinal = atoi(pAttrOfSource->Value());
									}
									else if (!strcmp(pAttrOfSource->Name(), "label"))
									{
										strName = pAttrOfSource->Value();
										//strName.assign("进程");

										m_pListCtrl->InsertColumn(nOrdinal - 1, strName.c_str());
									}
									else if (!strcmp(pAttrOfSource->Name(), "width"))
									{
										fWidth = atof(pAttrOfSource->Value());
										m_vColumnsWidth.push_back(fWidth);
										m_pListCtrl->SetColumnWidth(nOrdinal - 1, fWidth * rect.Width());
									}
									pAttrOfSource = pAttrOfSource->Next();
								}
								pSourceElement = pSourceElement->NextSiblingElement();
							}
// 							m_pListCtrl->InsertItem(0, "1", NULL);
// 							m_pListCtrl->InsertItem(1, "2", NULL);
// 							m_pListCtrl->InsertItem(2, "3", NULL);
							return;
						}


					}
					pSourceElement = pSourceElement->NextSiblingElement();
				}
			}
		}
	}

}


void CFirstLittleDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	if (m_pListCtrl != NULL && ::IsWindow(m_pListCtrl->m_hWnd))
	{
		CRect rect;
		FLOAT fWidth;
		GetClientRect(&rect);
		m_pListCtrl->MoveWindow(&rect);
		for (int i = 0; i < m_vColumnsWidth.size(); i++)
		{
			m_pListCtrl->SetColumnWidth(i, m_vColumnsWidth.at(i) * rect.Width());
		}
		//m_pListCtrl->InsertItem(0, "zero");
	}
}

VOID CFirstLittleDlg::SetDlgInitData(int nDlg, LittleDlgType dlgType, PVOID pInitData)
{
	ASSERT(pInitData != NULL);

	m_nDlg = nDlg;
	m_dlgType = dlgType;
	m_pInitData = pInitData;
}

