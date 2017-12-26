#include "StdAfx.h"
#include "RegEditEx.h"
#include"DataFormat.h"

TCHAR RegErrorMessage[][50]=
{
	TEXT("执行成功"),
	TEXT("输出缓冲区太小"),
	TEXT("输出缓冲区太大"),
	TEXT("没有找到键值或子键"),
	TEXT("不包含"),
	TEXT("值不包含数据"),
	TEXT("失败，内部错误"),
	TEXT("不能删除，可能是根键"),
	TEXT("暂时不支持的操作"),
	TEXT("要添加的键值或子键已经存在"),
	TEXT(""),
	TEXT(""),
	TEXT("")
};

CRegEditEx::CRegEditEx(HWND hWnd)
: m_hWnd(hWnd)
{
	IsInited=0;

}

CRegEditEx::~CRegEditEx(void)
{

}

VOID CRegEditEx::InitializeCtrl(CTreeCtrl &Tree,CListCtrl &List, HTREEITEM hParentItem)
{
	if(!IsInited)
		return;
	HTREEITEM hItem;
	PHIVE_LIST Seek = m_hRootNode;
	while(Seek)
	{		
		hItem = Tree.InsertItem(Seek->Name, 0, 0, hParentItem);
		Tree.InsertItem("", 0, 0, hItem);
		Seek = Seek->Next;
	}
}

BOOL CRegEditEx::EnumSubKeyForTree(HTREEITEM hParantItem, CString &strPathName)
{
	vector<PSUBKEY_ENTRY> vPSubKey;
	PSUBKEY_ENTRY pSubKey = NULL;
	BOOL bRet = TRUE;
	
	if (!IsRootKey(strPathName, vPSubKey))
	{
		this->EnumSubKey(strPathName.GetBuffer(0), vPSubKey);
	}
		
	for (int i = 0; i < vPSubKey.size(); i++)
	{
		pSubKey = vPSubKey.at(i);
		pSubKey->hParantItem = hParantItem;
		::SendMessage(m_hWnd, WM_FILEDLG_TREE_MSG, (WPARAM)pSubKey, NULL);
	}
	vPSubKey.clear();

	
	return bRet;
}
//判断是否是根key
BOOL CRegEditEx::IsRootKey(CString &strPathName, vector<PSUBKEY_ENTRY> &vPSubKey)
{
	BOOL bRet = FALSE;
	int nLen = 0;
	PSUBKEY_ENTRY pSubKey = NULL;
	PHIVE_LIST pHiveList = m_hRootNode;
	PCHAR pPos;

	if (!strPathName.Compare(ROOT_HKEY_LOCAL_MACHINE)) //HKEY_LOCAL_MACHINE
	{
		while (pHiveList)
		{
			if (pPos = strstr(pHiveList->Name, TAG_HKEY_LOCAL_MACHINE))
			{
				nLen = strlen(TAG_HKEY_LOCAL_MACHINE);
				pSubKey = (PSUBKEY_ENTRY)malloc(sizeof(SUB_KEY_ENTRY));
				strcpy(pSubKey->Name, &pHiveList->Name[nLen]);
				sprintf(pHiveList->Alias, "%s\\%s", ROOT_HKEY_LOCAL_MACHINE, pSubKey->Name);
				vPSubKey.push_back(pSubKey);
			}
			pHiveList = pHiveList->Next;
		}
		bRet = TRUE;
		
	}
	else if (!strPathName.Compare(ROOT_HKEY_USERS))	//HKEY_USERS
	{
		while (pHiveList)
		{
			if (pPos = strstr(pHiveList->Name, TAG_HKEY_USERS))
			{
				nLen = strlen(TAG_HKEY_USERS);
				pSubKey = (PSUBKEY_ENTRY)malloc(sizeof(SUB_KEY_ENTRY));
				strcpy(pSubKey->Name, &pHiveList->Name[nLen]);
				sprintf(pHiveList->Alias, "%s\\%s", ROOT_HKEY_USERS, pSubKey->Name);
				vPSubKey.push_back(pSubKey);
			}
			pHiveList = pHiveList->Next;
		}
		bRet = TRUE;
	}
	else if (!strPathName.Compare(ROOT_HKEY_CLASSES_ROOT))
	{
		strPathName.Format("%s", ROOT_HKEY_LOCAL_MACHINE);
		IsRootKey(strPathName, vPSubKey);
		strPathName.Format("%s\\SOFTWARE\\Classes", ROOT_HKEY_LOCAL_MACHINE);

		for (int i = 0; i < vPSubKey.size(); i++)
		{
			free(vPSubKey.at(i));
		}
		vPSubKey.clear();
	}
	else if (!strPathName.Compare(ROOT_HKEY_CURRENT_CONFIG))
	{
		strPathName.Format("%s", ROOT_HKEY_LOCAL_MACHINE);
		IsRootKey(strPathName, vPSubKey);
		strPathName.Format("%s\\SYSTEM\\ControlSet004\\Hardware Profiles\\Current", ROOT_HKEY_LOCAL_MACHINE);
		
		for (int i = 0; i < vPSubKey.size(); i++)
		{
			free(vPSubKey.at(i));
		}
		vPSubKey.clear();
	}

	return bRet;
}

BOOL CRegEditEx::EnumKeyValueForList(CString &strPathName)
{
	vector<PKEY_VALUE_ENTRY> vPKeyValue;
	vector<PSUBKEY_ENTRY> vPSubKey;
	PKEY_VALUE_ENTRY pKeyValue = NULL;
	BOOL bRet = TRUE;

	if (!IsRootKey(strPathName, vPSubKey))
	{
		this->EnumKeyValue(strPathName.GetBuffer(0), vPKeyValue);
	}
	

		
	for (int i = 0; i < vPKeyValue.size(); i++)
	{
		pKeyValue = vPKeyValue.at(i);
		::SendMessage(m_hWnd, WM_FILEDLG_LIST_MSG, (WPARAM)pKeyValue, NULL);
	}

	vPKeyValue.clear();
	for (i = 0; i < vPSubKey.size(); i++)
	{
		free(vPSubKey.at(i));
	}
	vPSubKey.clear();
	
	return bRet;
}

bool CRegEditEx::InitializeInternalData()
{
	IsInited = this->InitHiveFile();	
	return IsInited;
}

CString CRegEditEx::FormatError(REGSTATUS Error)
{
	int index=Error&0x0FF;
	return CString(RegErrorMessage[index]);
}
CString CRegEditEx::GetRegValueTypeName(unsigned long Type)
{
	switch(Type)
	{
	case REG_DWORD:
		return CString(L"REG_DWORD");
	case REG_SZ:
		return CString(L"REG_SZ");
	case REG_EXPAND_SZ :
		return CString(L"REG_EXPAND_SZ ");
	case REG_BINARY:
		return CString(L"REG_BINARY");
	case REG_LINK:
		return CString(L"REG_LINK");
	case REG_MULTI_SZ:
		return CString(L"REG_MULTI_SZ");
	//case REG_QWORD:
	//	return CString(L" REG_QWORD ");
	default:
		return CString(L"Other");
	}
	return CString(L"");
}

int CRegEditEx::GetIcoIndexOfType(unsigned long Type)
{
	switch(Type)
	{
	case REG_DWORD:
		return 0;
	case REG_SZ:
		return 1;
	case REG_EXPAND_SZ :
		return 1;
	case REG_BINARY:
		return 0;
	case REG_LINK:
		return 0;
	case REG_MULTI_SZ:
		return 1;
	//case  REG_QWORD :
	//	return 2;
	default:
		return 2;
	}

}

CString CRegEditEx::FormatKeyValueData(ULONG Type, ULONG Len, PVOID Buffer)
{
	CString Temp, Ret="";
	int nPos = 0;
	CHAR *lpBuffer = NULL;
	WCHAR *lpwzBuffer = (WCHAR*)Buffer;
	
	lpBuffer = (PCHAR)malloc(Len);
	ZeroMemory(lpBuffer, Len);
	wcharTochar((WCHAR*)Buffer, lpBuffer, Len);


	switch(Type)
	{
	case REG_DWORD:
		{
			Ret.Format("0x%08x", *(int*)Buffer);
			break;
		}
	case REG_SZ:
		Ret.Format("%s", (CHAR*)lpBuffer);
		break;
	case REG_EXPAND_SZ :
		Ret.Format("%s", (CHAR*)lpBuffer);
		break;
	case REG_BINARY:
		Ret = CDataFormat::FormatDataToHex((UCHAR*)Buffer,Len);
		break;
	case REG_LINK:
		Ret.Format("%s", (CHAR*)lpBuffer);
		break;
	case REG_MULTI_SZ:
		{
			while((2 * nPos) < Len)
			{
				Temp.Format("%S", (WCHAR*)&lpwzBuffer[nPos]);
				nPos += Temp.GetLength();
				if (Temp.GetLength() == 0)
				{
					nPos++;
				}
				else if ((2 * nPos) <= Len)
				{
					Ret += Temp;
					Ret += " ";
					nPos++;
				}
			}
		}
		break;
	default:
		{
			Ret = CDataFormat::FormatDataToHex((UCHAR*)Buffer,Len);
		}
		break;
	}

	if (NULL != lpBuffer)
	{
		free(lpBuffer);
		lpBuffer = NULL;
	}
	return  Ret;
}