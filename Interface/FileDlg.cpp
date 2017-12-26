// FileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sysark.h"
#include "FileDlg.h"
#include "Registry/RegEditEx.h"
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MYCOMPUTER "我的电脑"
#define ILI_CDDRV     0
#define ILI_CLSDFLD   1
#define ILI_DRIVE     2 
#define ILI_FLOPPYDRV 3 
#define ILI_MYCOMP    4
#define ILI_OPENFLD   5 
#define ILI_TEXTFILE  6 


/////////////////////////////////////////////////////////////////////////////
// CFileDlg dialog


CFileDlg::CFileDlg(HANDLE hDevice, CWnd* pParent /*=NULL*/)
	: CDialog(CFileDlg::IDD, pParent)
	, m_pRegEditEx(NULL)
	, m_pDoc(NULL)
	, m_hDevice(hDevice)
	, m_dlgType(DLG_TYPE_UNKNOWN)
{
	//{{AFX_DATA_INIT(CFileDlg)
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


void CFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileDlg)
	DDX_Control(pDX, IDC_FILE_TREE, m_TreeCtrl);
	DDX_Control(pDX, IDC_FILE_LIST, m_ListCtrl);
	//}}AFX_DATA_MAP
}


BOOL CFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	InitListCtrl();
	
	return TRUE;
}

VOID CFileDlg::Clear()
{
	if (NULL != m_pDoc)
	{
		delete m_pDoc;
		m_pDoc = NULL;
	}

	if (NULL != m_pRegEditEx)
	{
		delete m_pRegEditEx;
		m_pRegEditEx = NULL;
	}

	m_vColumnsWidth.clear();
}


BEGIN_MESSAGE_MAP(CFileDlg, CDialog)
	//{{AFX_MSG_MAP(CFileDlg)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_FILE_TREE, OnItemexpandingFileTree)
	ON_NOTIFY(TVN_SELCHANGING, IDC_FILE_TREE, OnSelchangingFileTree)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_FILEDLG_LIST_MSG, OnListCtrlMsg)
	ON_MESSAGE(WM_FILEDLG_TREE_MSG, OnTreeCtrlMsg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileDlg message handlers

LRESULT CFileDlg::OnListCtrlMsg(WPARAM wParam, LPARAM lParam)
{
	//TODO: Add your message handle code
	ULONG ulItem = 0;
	CString strData;
	switch (m_dlgType)
	{
	case DLG_TYPE_REGISTRY:
		{
			PKEY_VALUE_ENTRY pKeyValue = (PKEY_VALUE_ENTRY)wParam;
			ulItem = m_ListCtrl.GetItemCount();

			strData = m_pRegEditEx->FormatKeyValueData(pKeyValue->Type, pKeyValue->DataLen, pKeyValue->DataBuffer);

			SetListItemText(ulItem, 0, pKeyValue->Name);
			SetListItemText(ulItem, 1, m_pRegEditEx->GetRegValueTypeName(pKeyValue->Type));
			SetListItemText(ulItem, 2, strData);

		}
		break;
	case DLG_TYPE_FILE:
		break;
	default:
		break;
	}

	return 0;
}

LRESULT CFileDlg::OnTreeCtrlMsg(WPARAM wParam, LPARAM lParam)
{
	//TODO: Add your message handle code
	ULONG ulItem = 0;
	PUCHAR lpBuffer = NULL;
	switch (m_dlgType)
	{
	case DLG_TYPE_REGISTRY:
		{
			PSUBKEY_ENTRY pSubKey = (PSUBKEY_ENTRY)wParam;

			AddSubKey(pSubKey->Name, pSubKey->hParantItem);
			
			free(pSubKey);
			pSubKey = NULL;
		}
		break;
	case DLG_TYPE_FILE:
		break;
	default:
		break;
	}
	
	return 0;
}

void CFileDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if (::IsWindow(m_ListCtrl.m_hWnd))
	{
		CRect rect;
		GetClientRect(&rect);
		rect.left = rect.Width()/4 + 10;
		//rect.right = rect.left;
		//rect.bottom = rect.top;
		m_ListCtrl.MoveWindow(&rect);
		for (int i = 0; i < m_vColumnsWidth.size(); i++)
		{
			m_ListCtrl.SetColumnWidth(i, m_vColumnsWidth.at(i) * rect.Width());
		}
		m_ListCtrl.ShowWindow(SW_SHOW);
	}
	if (::IsWindow(m_TreeCtrl.m_hWnd))
	{
		CRect rect;
		GetClientRect(&rect);
		rect.right = rect.Width()/4;
		m_TreeCtrl.MoveWindow(&rect);
		m_TreeCtrl.ShowWindow(SW_SHOW);
	}
	
}

BOOL CFileDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN\
		&& (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}



VOID CFileDlg::InitListColumn( LPCTSTR lpDlgTitle )
{
	ASSERT(NULL != lpDlgTitle);
	string strName;
	int nOrdinal = -1, nColumns = 0;
	if (NULL != m_pDoc)
	{
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
							if (strName.compare("文件") == NULL)
							{
								m_dlgType = DLG_TYPE_FILE;
							}
							else if (strName.compare("注册表") == NULL)
							{
								m_dlgType = DLG_TYPE_REGISTRY;
							}
							else
							{
								MessageBoxA("未知错误");
							}

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

//创建list ctrl
void CFileDlg::InitListCtrl()
{
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);
	
	if (::IsWindow(m_ListCtrl.m_hWnd))
	{
		m_ListCtrl.SetExtendedStyle(m_ListCtrl.GetExtendedStyle() | LVS_EX_GRIDLINES);
		m_ListCtrl.MoveWindow(&rect);
	}
	
}

VOID CFileDlg::SetListItemText(int nItem, int nSubItem, LPCTSTR lpszText)
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

VOID CFileDlg::DeleteListAllItems()
{
	if (::IsWindow(m_ListCtrl.m_hWnd))
	{
		m_ListCtrl.DeleteAllItems();
	}
}

void CFileDlg::OnItemexpandingFileTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	switch (m_dlgType)
	{
	case DLG_TYPE_FILE:
		{
			ItemExpandingFileTreeForFile(pNMHDR, pResult);
		}
		break;
	case DLG_TYPE_REGISTRY:
		{
			ItemExpandingFileTreeForRegistry(pNMHDR, pResult);
		}
		break;
	default:
		break;
	}
	*pResult = 0;
}

void CFileDlg::OnSelchangingFileTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

HBRUSH CFileDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

BOOL CFileDlg::InitTreeView()
{
	BOOL bRet = FALSE;
	switch (m_dlgType)
	{
	case DLG_TYPE_FILE: //文件
		{
			HTREEITEM hItem;
			int nPos = 0;
			UINT nCount = 0;
			CString strDrive = L"?:\\";
			
			m_TreeCtrl.ModifyStyle(0, TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_SINGLEEXPAND|TVSIL_NORMAL);
			hItem = m_TreeCtrl.InsertItem(MYCOMPUTER, TVI_ROOT);
			
			DWORD dwDriveList = ::GetLogicalDrives ();
			
			CString cTmp;
			
			while (dwDriveList) {
				if (dwDriveList & 1) {
					cTmp = strDrive;
					strDrive.SetAt (0, 0x41 + nPos);
					if (AddDrives(strDrive , hItem))
						nCount++;
				}
				dwDriveList >>= 1;
				nPos++;
			}
			m_TreeCtrl.Expand(hItem, TVE_EXPAND);
		}
		break;
	case DLG_TYPE_REGISTRY: //注册表
		{
			HTREEITEM hItem;
			m_TreeCtrl.ModifyStyle(0, TVS_HASLINES|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_SINGLEEXPAND|TVSIL_NORMAL);
			hItem = m_TreeCtrl.InsertItem(MYCOMPUTER, TVI_ROOT);
			// AddSubKey(ROOT_HKEY_CLASSES_ROOT, hItem);
			// AddSubKey(ROOT_HKEY_CURRENT_USER, hItem);
			AddSubKey(ROOT_HKEY_LOCAL_MACHINE, hItem);
			AddSubKey(ROOT_HKEY_USERS, hItem);
			// AddSubKey(ROOT_HKEY_CURRENT_CONFIG, hItem);
			
			m_pRegEditEx = new CRegEditEx(m_hWnd);
			m_pRegEditEx->InitializeInternalData();
			//m_pRegEditEx->InitializeCtrl(m_TreeCtrl, m_ListCtrl, hItem);
			
			m_TreeCtrl.Expand(hItem, TVE_EXPAND);
		}
		break;
	default:
		break;
	}
	
	return bRet;
}

//////////////////////////////////文件////////////////////////////////////////

void CFileDlg::ItemExpandingFileTreeForFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	CString strPathName = GetPathFromItem (hItem);
	m_ListCtrl.DeleteAllItems();
	
	if (!IsMediaValid (strPathName)) 
	{
		HTREEITEM hRoot = GetDriveNode (hItem);
		m_TreeCtrl.Expand (hRoot, TVE_COLLAPSE);
		DeleteChildren (hRoot);
		AddDummyNode (hRoot);
		*pResult = TRUE;
		return;
	}
	
	
	if (!IsPathValid (strPathName)) 
	{
		if(strPathName != MYCOMPUTER && strPathName != "")
		{
			m_TreeCtrl.DeleteItem (hItem);
			*pResult = TRUE;
			return;
		}
	}
	
	CWaitCursor wait;
	
	if (pNMTreeView->action == TVE_EXPAND) 
	{
		if(strPathName != MYCOMPUTER)
		{
			DeleteChildren (hItem);
			if (!AddDirectoryNodes (hItem, strPathName))
				*pResult = TRUE;
		}
	}
	else {
		if(strPathName != MYCOMPUTER)
		{
			DeleteChildren (hItem);
			if (IsDriveNode (hItem))
				AddDummyNode (hItem);
			else
				SetButtonState (hItem, strPathName);
		}
	}

	*pResult = 0;
}

BOOL CFileDlg::IsDriveNode(HTREEITEM hItem)
{
	
	return (m_TreeCtrl.GetParentItem (hItem) == NULL) ? TRUE : FALSE;
}

BOOL CFileDlg::AddSubKey(CString strSubKey, HTREEITEM hParent)
{
	HTREEITEM hItem;
	
	hItem = m_TreeCtrl.InsertItem(strSubKey.GetBuffer(0), 0, 0, hParent);
	AddDummyNode(hItem);

	return TRUE;
}


BOOL CFileDlg::AddDrives(CString strDrive, HTREEITEM hParent)
{
	HTREEITEM hItem;
	UINT nType = ::GetDriveType ((LPCTSTR) strDrive.GetBuffer(0));
	UINT nDrive = (UINT) strDrive[0] - 0x41;

	switch (nType) {

	case DRIVE_REMOVABLE:
		strDrive=strDrive;
		hItem = m_TreeCtrl.InsertItem(strDrive.GetBuffer(0), ILI_FLOPPYDRV, ILI_FLOPPYDRV, hParent);
		if (HasSubdirectory (strDrive))
		{
			AddDummyNode(hItem);
		}
		break;

	case DRIVE_FIXED:
		strDrive=strDrive;
		hItem = m_TreeCtrl.InsertItem(strDrive.GetBuffer(0),  ILI_DRIVE, ILI_DRIVE, hParent);
		if (HasSubdirectory (strDrive))
		{
			AddDummyNode(hItem);
		}
		break;

	case DRIVE_REMOTE:
		hItem = m_TreeCtrl.InsertItem(strDrive.GetBuffer(0), ILI_DRIVE, ILI_DRIVE, hParent);
		if (HasSubdirectory (strDrive))
		{
			AddDummyNode(hItem);
		}
		break;

	case DRIVE_CDROM:
		strDrive=strDrive;
		hItem = m_TreeCtrl.InsertItem(strDrive.GetBuffer(0), ILI_CDDRV, ILI_CDDRV, hParent);
		if (HasSubdirectory (strDrive))
		{
			AddDummyNode(hItem);
		}
		break;

	case DRIVE_RAMDISK:
		hItem = m_TreeCtrl.InsertItem(strDrive.GetBuffer(0), ILI_CDDRV, ILI_CDDRV, hParent);
		if (HasSubdirectory (strDrive))
		{
			AddDummyNode(hItem);
		}
		break;

	default:
		return FALSE;
	}

	return true;
}
BOOL CFileDlg::HasSubdirectory(CString &strPathName)
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	BOOL bResult = FALSE;
	UINT uDriveType = GetDriveType(strPathName.GetBuffer(0));
	if (DRIVE_FIXED !=uDriveType)
	{
		return bResult;
	}
	if ("A:\\" == strPathName)
	{
		    return bResult;
	}

	CString strFileSpec = strPathName;
	if (strFileSpec.Right (1) != "\\")
		strFileSpec += "\\";
	strFileSpec += "*.*";

	if ((hFind = ::FindFirstFile ((LPCTSTR) strFileSpec.GetBuffer(0), &fd)) !=
		INVALID_HANDLE_VALUE) {
			do {
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					CString strFileName = (LPCTSTR) &fd.cFileName;
					if ((strFileName != ".") && (strFileName != ".."))
						bResult = TRUE;
				}
			} while (::FindNextFile (hFind, &fd) && !bResult);
			::FindClose (hFind);
	}

	return bResult;
}


UINT CFileDlg::DeleteChildren(HTREEITEM hItem)
{
	
	UINT nCount = 0;
	HTREEITEM hChild = m_TreeCtrl.GetChildItem (hItem);
	
	while (hChild != NULL) {
		HTREEITEM hNextItem = m_TreeCtrl.GetNextSiblingItem (hChild);
		m_TreeCtrl.DeleteItem (hChild);
		hChild = hNextItem;
		nCount++;
		
		//		dwTreeItemCount--;
		
	}
	return nCount;
	
}


void CFileDlg::SetButtonState(HTREEITEM hItem, CString &strPathName)
{
	//if (HasSubdirectory (strPathName))
		AddDummyNode (hItem);
}

UINT CFileDlg::AddDirectoryNodes(HTREEITEM hItem, CString &strPathName)
{
 	DWORD    bytesReturned=0;
 	ULONG   num=0;
 	PDIRECTORY_INFO   pDirInfo = NULL;
 	DIRECTORY_INFO_EX  DIRECTORY_INFO_b;
	CHAR lpBuffer[MAX_PATH] = {0};
 	CString str,str1,strFileSpec = strPathName;
 	if (strFileSpec.Right (1) != "\\")
 		strFileSpec += "\\";
 	
 	str1=strFileSpec;
	//wcharTochar(strFileSpec.GetBuffer(strFileSpec.GetLength()), lpBuffer, strFileSpec.GetLength());
	strcpy(lpBuffer, strFileSpec.GetBuffer(0));
 
 	strFileSpec += "*.*";
 	DeviceIoControl(m_hDevice,\
		(DWORD)IOCTL_GET_DIRECTORY_INFO_COUNT,\
		lpBuffer, \
		sizeof(lpBuffer), \
		&num, \
		sizeof(ULONG), \
		&bytesReturned, \
		NULL);
 	if(num==0)
 	{
 		return 0;
 	}
 	pDirInfo = (PDIRECTORY_INFO)malloc(num * sizeof(DIRECTORY_INFO));
 	if(pDirInfo==NULL)
 	{
 		return 0;
 	}
	ZeroMemory(pDirInfo, num * sizeof(DIRECTORY_INFO));
 	DeviceIoControl(m_hDevice,\
		(DWORD)IOCTL_ENUM_DIRECTORY_INFO,\
		lpBuffer,\
		sizeof(lpBuffer),\
		pDirInfo,\
		num * sizeof(DIRECTORY_INFO),\
		&bytesReturned,\
		NULL);	
 	CWaitCursor wait;
 	WCHAR wTemp[MAX_PATH]={'\0'};
 	m_ListCtrl.DeleteAllItems();
 	//SetPath(str1,hDevice);
 	for(ULONG i = 0; i < num; i++)
 	{
 		str = str1 + pDirInfo[i].FileName;
 		CString strFileName = (LPCTSTR) &pDirInfo[i].FileName;
 		if(PathIsDirectory(str))
 		{
 			if(wcscmp(pDirInfo[i].FileName, L"."))
 			{
 				if(wcscmp(pDirInfo[i].FileName, L".."))
 				{
					ZeroMemory(lpBuffer, MAX_PATH);
					wcharTochar(pDirInfo[i].FileName, lpBuffer, MAX_PATH);
 					HTREEITEM hChild =
 						m_TreeCtrl.InsertItem ((LPCTSTR)lpBuffer,//&fd.cFileName,
 						ILI_CLSDFLD , ILI_OPENFLD , hItem , TVI_SORT);
 
 					CString strNewPathName = strPathName;
 					if (strNewPathName.Right (1) != "\\")
 					{strNewPathName += "\\";}
 
 					strNewPathName += pDirInfo[i].FileName;//fd.cFileName;
 					SetButtonState (hChild, strNewPathName);	
 					
 				}
 
 			}
 		}
 		else
 		{
 			DIRECTORY_INFO_b.DirectoryInfo = pDirInfo[i];
 			DIRECTORY_INFO_b.path = str1;
 		
 			AddToListView(&DIRECTORY_INFO_b);	
 		}
 	} 
	if (NULL != pDirInfo)
	{
		free(pDirInfo);
		pDirInfo = NULL;
	}
 	return num;
}

void CFileDlg::AddToListView(PDIRECTORY_INFO_EX fd)
{
	CString  strPath, temp;
	CHAR lpFileName[MAX_PATH] = {0};
	int itemNum = m_ListCtrl.GetItemCount();

	//全路径
 	strPath = fd->path + fd->DirectoryInfo.FileName;

	wcharTochar(fd->DirectoryInfo.FileName, lpFileName, MAX_PATH);
 	m_ListCtrl.InsertItem(itemNum, lpFileName);
 	temp.Format("%d-%d-%d",fd->DirectoryInfo.CreationTime.Year,fd->DirectoryInfo.CreationTime.Month,fd->DirectoryInfo.CreationTime.Day);
 	m_ListCtrl.SetItemText(itemNum, 2, temp);
 	if(fd->DirectoryInfo.AllocationSize.QuadPart > 1024*1024*1024)
 	{
 		temp.Format("%0.2fG",(float)(fd->DirectoryInfo.AllocationSize.QuadPart)/(float)(1024*1024*1024));
 	}
 	else if(fd->DirectoryInfo.AllocationSize.QuadPart > 1024*1024)
 	{
 		temp.Format("%0.2fM",(float)(fd->DirectoryInfo.AllocationSize.QuadPart)/(float)(1024*1024));
 	}
 	else if(fd->DirectoryInfo.AllocationSize.QuadPart > 1024)
 	{
 		temp.Format("%0.2fK",(float)(fd->DirectoryInfo.AllocationSize.QuadPart)/(float)(1024));
 	}
 	else
 	{
 		temp.Format("%ldB",fd->DirectoryInfo.AllocationSize.QuadPart);
 	}
 	m_ListCtrl.SetItemText(itemNum, 1, temp);
 	temp.Format("%d-%d-%d",fd->DirectoryInfo.LastWriteTime.Year,fd->DirectoryInfo.LastWriteTime.Month,fd->DirectoryInfo.LastWriteTime.Day);
 	m_ListCtrl.SetItemText(itemNum, 3, temp);
 	temp.Format("%d-%d-%d",fd->DirectoryInfo.LastAccessTime.Year,fd->DirectoryInfo.LastAccessTime.Month,fd->DirectoryInfo.LastAccessTime.Day);
 	m_ListCtrl.SetItemText(itemNum, 4, temp);
 	switch(fd->DirectoryInfo.FileAttributes)
 	{
 	case 0x00000001:
 		m_ListCtrl.SetItemText(itemNum, 5, "只读");
 		break;
 	case 0x00000002:
 		m_ListCtrl.SetItemText(itemNum, 5, "隐藏");
 		break;
 	case 0x00000004:
 		m_ListCtrl.SetItemText(itemNum, 5, "系统");
 		break;
 	case 0x00000020:
 		m_ListCtrl.SetItemText(itemNum, 5, "存档");
 		break;
 	case 0x00000080:
 		m_ListCtrl.SetItemText(itemNum, 5, "正常");
 		break;
 	case 0x00000100:
 		m_ListCtrl.SetItemText(itemNum, 5, "临时");
 		break;
 	case 0x00000800:
 		m_ListCtrl.SetItemText(itemNum, 5, "压缩");
 		break;
 	}

 	//m_ListCtrl.SetItem(itemNum, 0, LVIF_TEXT | LVIF_IMAGE, fd->DirectoryInfo.FileName, 
 //	/*ProcessList->GetFileIcon(strPath)*/NULL, 0, 0, 0);
 //		m_ListCtrl.SetItem(index, 0, LVIF_TEXT | LVIF_IMAGE,wBuffer, 
 //		0, 0, 0, 0);
 	//SetPath(strPath);
}

HTREEITEM CFileDlg::GetDriveNode(HTREEITEM hItem)
{
	
	HTREEITEM hParent;
	do {
		hParent = m_TreeCtrl.GetParentItem (hItem);
		if (hParent != NULL)
			hItem = hParent;
	} while (hParent != NULL);
	return hItem;
	
}
BOOL CFileDlg::IsMediaValid(CString &strPathName)
{
	
	UINT nDriveType = GetDriveType ((LPCTSTR) strPathName.GetBuffer(0));
	if ((nDriveType != DRIVE_REMOVABLE) && (nDriveType != DRIVE_CDROM))
		return TRUE;
	else
		return FALSE;
}

//获取路径
CString CFileDlg::GetPathFromItem(HTREEITEM hItem)
{
	CString strPathName;
	while (hItem != NULL) 
	{
		CString string = m_TreeCtrl.GetItemText (hItem);
		if ((string.Right (1) != "\\") && !strPathName.IsEmpty ())
		{
			string += "\\";
		}
		strPathName = string + strPathName;
		hItem = m_TreeCtrl.GetParentItem (hItem);
	}


	if(strPathName.Left(8) == MYCOMPUTER && strPathName.GetLength() > 8)
		strPathName = strPathName.Mid(9);

	return strPathName;

}

//判断该文件是否存在
BOOL CFileDlg::IsPathValid(CString &strPathName)
{
	
	if (strPathName.GetLength () == 3)
		return TRUE;
	
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	BOOL bResult = FALSE;
	
	if ((hFind = ::FindFirstFile ((LPCTSTR) strPathName.GetBuffer(0), &fd)) !=
		INVALID_HANDLE_VALUE) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			bResult = TRUE;
		::FindClose (hFind);
	}
	return bResult;
}

void CFileDlg::AddDummyNode(HTREEITEM hItem)
{
	m_TreeCtrl.InsertItem("", 0, 0, hItem);
}

/////////////////////////////////注册表/////////////////////////////////////////

void CFileDlg::ItemExpandingFileTreeForRegistry(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	CString strPathName = GetPathFromItem (hItem);
	m_ListCtrl.DeleteAllItems();
	
// 
// 	if(strPathName != MYCOMPUTER && strPathName != "")
// 	{
// 		m_TreeCtrl.DeleteItem (hItem);
// 		*pResult = TRUE;
// 		return;
// 	}
	
	CWaitCursor wait;
	
	if (pNMTreeView->action == TVE_EXPAND) 
	{
		if(strPathName != MYCOMPUTER)
		{
			DeleteChildren (hItem);
			if (m_pRegEditEx->EnumSubKeyForTree(hItem, strPathName)
				&& m_pRegEditEx->EnumKeyValueForList(strPathName))
			{
				*pResult = TRUE;
			}
			//if (!AddDirectoryNodes (hItem, strPathName))
		}
	}
	else {
		if(strPathName != MYCOMPUTER)
		{
			DeleteChildren (hItem);
			if (IsDriveNode (hItem))
				AddDummyNode (hItem);
			else
			{
				SetButtonState (hItem, strPathName);
			}
			m_pRegEditEx->EnumKeyValueForList(strPathName);
		}
	}

	*pResult = 0;
}

