// CommonDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sysark.h"
#include "CommonDlg.h"
#include "Ioctrls.h"

ULONG g_nSelectItem = -1;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//#define IDD_COMMON_LSIT		WM_USER+10

/////////////////////////////////////////////////////////////////////////////
// CCommonDlg dialog
#pragma comment(lib, "Tinyxml.lib")


CCommonDlg::CCommonDlg(CWnd* pParent /*=NULL*/, UINT nIDResource)
: CDialog(CCommonDlg::IDD, pParent)
, m_pDoc(NULL)
, m_nIDLButtonMenu(nIDResource)
, m_nMenuItem(-1)
, m_nSelectItem(-1)
{
	//{{AFX_DATA_INIT(CCommonDlg)
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


void CCommonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommonDlg)
	DDX_Control(pDX, IDC_COMMON_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
}

BOOL CCommonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	InitListCtrl();
	
	return TRUE;
}


//创建list ctrl
void CCommonDlg::InitListCtrl()
{
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);
	//rect.top = 10;

	if (::IsWindow(m_ListCtrl.m_hWnd))
	{
		m_ListCtrl.SetExtendedStyle(m_ListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES);
		m_ListCtrl.MoveWindow(&rect);
	}
	
	//if (NULL == m_pListCtrl)
	//{
	//	m_pListCtrl = new CReportCtrl();
	//	m_ListCtrl.Create(
	//		this,
	//		IDD_COMMON_LSIT, rect, WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_EDITLABELS); 
	//	m_ListCtrl.SetExtendedStyle(m_ListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES);
	//	
	//	m_ListCtrl.MoveWindow(&rect);
	//}
}

VOID CCommonDlg::InitListColumn( LPCTSTR lpDlgTitle )
{
	ASSERT(NULL != lpDlgTitle);
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
								// 								if (nDlg != nOrdinal)
								// 								{
								// 									break;
								// 								}
							}
							else if (!strcmp(pAttrOfSource->Name(), "label"))
							{
								strName = pAttrOfSource->Value();
								if (strName.compare(lpDlgTitle) != NULL)
								{
									break;
								}
							}
							else if (!strcmp(pAttrOfSource->Name(), "num_columns"))
							{
								nColumns = atoi(pAttrOfSource->Value());
							}
							pAttrOfSource = pAttrOfSource->Next();
						}
						
						if (strName.compare(lpDlgTitle) == NULL)
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
										
										m_ListCtrl.InsertColumn(nOrdinal - 1, strName.c_str());
									}
									else if (!strcmp(pAttrOfSource->Name(), "width"))
									{
										fWidth = atof(pAttrOfSource->Value());
										m_vColumnsWidth.push_back(fWidth);
										m_ListCtrl.SetColumnWidth(nOrdinal - 1, fWidth * rect.Width());
									}
									pAttrOfSource = pAttrOfSource->Next();
								}
								pSourceElement = pSourceElement->NextSiblingElement();
							}
							m_ListCtrl.InsertItem(0, "1", NULL);
							m_ListCtrl.InsertItem(1, "2", NULL);
							m_ListCtrl.InsertItem(2, "3", NULL);
							return;
						}
						
						
					}
					pSourceElement = pSourceElement->NextSiblingElement();
				}
			}
		}
	}
	
}


BEGIN_MESSAGE_MAP(CCommonDlg, CDialog)
//{{AFX_MSG_MAP(CCommonDlg)
ON_WM_SIZE()
ON_WM_RBUTTONDOWN()
ON_NOTIFY(NM_RCLICK, IDC_COMMON_LIST, OnNMRclickList)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCommonDlg message handlers

void CCommonDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
	if (::IsWindow(m_ListCtrl.m_hWnd))
	{
		CRect rect;
		FLOAT fWidth;
		GetClientRect(&rect);
		m_ListCtrl.MoveWindow(&rect);
		for (int i = 0; i < m_vColumnsWidth.size(); i++)
		{
			m_ListCtrl.SetColumnWidth(i, m_vColumnsWidth.at(i) * rect.Width());
		}
		m_ListCtrl.ShowWindow(SW_SHOW);
		m_ListCtrl.UpdateWindow();
		//UpdateWindow();
	}
	
}

void CCommonDlg::UTF8ToGBK( char *&szOut )
{
	unsigned short *wszGBK;
	char *szGBK;
	//长度
	int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)szOut, -1, NULL, 0);
	wszGBK = new unsigned short[len+1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)szOut, -1, (LPWSTR)wszGBK, len);
	
	//长度
	len = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
	szGBK = new char[len+1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
	
	//szOut = szGBK; //这样得到的szOut不正确，因为此句意义是将szGBK的首地址赋给szOut，当delete []szGBK执行后szGBK的内
	
	//存空间将被释放，此时将得不到szOut的内容
	
	memset(szOut,'\0',strlen(szGBK)+1); //改将szGBK的内容赋给szOut ，这样即使szGBK被释放也能得到正确的值
	memcpy(szOut,szGBK,strlen(szGBK));
	
	
	delete []szGBK;
	delete []wszGBK;
}

void CCommonDlg::GBKToUTF8(char* &szOut)
{
	char* strGBK = szOut;
	
	int len=MultiByteToWideChar(CP_ACP, 0, (LPCSTR)strGBK, -1, NULL,0);
	unsigned short * wszUtf8 = new unsigned short[len+1];
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)strGBK, -1, (LPWSTR)wszUtf8, len);
	
	len = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)wszUtf8, -1, NULL, 0, NULL, NULL);
	char *szUtf8=new char[len + 1];
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte (CP_UTF8, 0, (LPWSTR)wszUtf8, -1, szUtf8, len, NULL,NULL);
	
	//szOut = szUtf8;
	memset(szOut,'\0',strlen(szUtf8)+1);
	memcpy(szOut,szUtf8,strlen(szUtf8));
	
	delete[] szUtf8;
	delete[] wszUtf8;
}

VOID CCommonDlg::SetListItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	ASSERT(::IsWindow(m_ListCtrl.m_hWnd));
	
	if(m_ListCtrl.GetItemCount() <= nItem)
	{
		m_ListCtrl.InsertItem(nItem, lpszText, NULL);
	}
	else
	{
		m_ListCtrl.SetItemText(nItem, nSubItem, lpszText);
	}
}

VOID CCommonDlg::DeleteListAllItems()
{
	if (::IsWindow(m_ListCtrl.m_hWnd))
	{
		m_ListCtrl.DeleteAllItems();
	}
}

VOID CCommonDlg::UpdateStatusBar( int nDlg, HWND hWnd )
{

	switch (nDlg)
	{
	case 0:
		{
			//进程对话框
			PTCHAR lpMsg = new TCHAR[MAX_PATH];
			ZeroMemory(lpMsg, MAX_PATH);
			sprintf(lpMsg, "进程：%d", m_ListCtrl.GetItemCount());
			
			::SendMessage(hWnd, WM_STATUS_MSG, (WPARAM)lpMsg, NULL);
		}
		break;
	default:
		break;
	}
}

void CCommonDlg::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	
	CDialog::OnRButtonDown(nFlags, point);
}

void CCommonDlg::OnNMRclickList( NMHDR* pNMHDR, LRESULT* pResult )
{
	   // TODO: Add your control notification handler code here
    CMenu   Menu;  
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
//     if(pNMListView->iItem == -1)
//     {
//         //点击在空白
// 		g_nSelectItem -1;
// 		m_nSelectItem = -1;
//     }
//     else
    {
        // 右击在一个item上
		if   (Menu.LoadMenu(m_nIDLButtonMenu))                  
		{  
			g_nSelectItem = pNMListView->iItem;
			m_nSelectItem = pNMListView->iItem;
			CMenu*   pPopup   =   Menu.GetSubMenu(0);  
			ASSERT(pPopup   !=   NULL);  
			CPoint   oPoint;  
			GetCursorPos(   &oPoint   );  
			SetForegroundWindow();  
			pPopup->TrackPopupMenu(  
				TPM_LEFTALIGN   |   TPM_RIGHTBUTTON,  
				oPoint.x,   oPoint.y,   this); 
			HMENU hmenu = Menu.Detach(); 
			Menu.DestroyMenu();
			m_nMenuItem = pNMListView->iItem;
		}   
    }
	
	
    *pResult = 0;

}

INT CCommonDlg::GetSelectItemPid()
{
	CString strText;
	strText = m_ListCtrl.GetItemText(m_nSelectItem, 1);
	return atoi(strText);
}

//获取当前选择行的路径，一般列头是“**路径”
VOID CCommonDlg::GetSelectItemFilePath(LPSTR lpFilePath, ULONG ulMaxPath)
{
	CString strText;
	int nColumn = -1;

	for (int i = 0; i < m_ListCtrl.GetHeaderCtrl()->GetItemCount(); i++)
	{
		strText = m_ListCtrl.GetHeaderText(i);
		if (strText.FindOneOf("路径") >= 0)
		{
			nColumn = i;
			break;
		}
	}
	strText = m_ListCtrl.GetItemText(m_nSelectItem, nColumn);
	if (strText.GetLength() <= ulMaxPath)
	{
		strcpy(lpFilePath, strText.GetBuffer(0));
	}
}


//获取当前选择行对应列的值，一般列头是“**名称”，lpListName是“名称”等
VOID CCommonDlg::GetSelectItemValue(LPSTR lpSrvName, ULONG ulMaxName, LPSTR lpListName)
{
	CString strText;
	int nColumn = -1;
	
	for (int i = 0; i < m_ListCtrl.GetHeaderCtrl()->GetItemCount(); i++)
	{
		strText = m_ListCtrl.GetHeaderText(i);
		if (strText.Find(lpListName) >= 0)
		{
			nColumn = i;
			break;
		}
	}
	strText = m_ListCtrl.GetItemText(m_nSelectItem, nColumn);
	if (strText.GetLength() <= ulMaxName)
	{
		strcpy(lpSrvName, strText.GetBuffer(0));
	}
}

VOID CCommonDlg::SetListItemColor(int nItem, int nColFlag)
{
	COLORREF clrText = -1, clrBkgnd = -1;
	switch (nColFlag)
	{
	case 0:
		break;
	case 1:
		//红色
		clrText = RGB(255, 0, 0);
		break;
	case 2:
		break;
	}
	m_ListCtrl.SetItemTextColor(0, -1, clrText, FALSE);
	//m_ListCtrl.SetItemColor(nItem, clrText, clrBkgnd);
}



BOOL CCommonDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN\
		&& (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CCommonDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}
