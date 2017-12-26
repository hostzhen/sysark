#pragma once
#include"HiveAnysBase.h"
class CRegEditEx :public HiveAnysBase
{
private:
	bool IsInited;
	HWND m_hWnd;
public:
	CRegEditEx(HWND hWnd);
	~CRegEditEx(void);	
	VOID InitializeCtrl(CTreeCtrl &Tree, CListCtrl &List, HTREEITEM hParentItem);
	bool InitializeInternalData();
	BOOL EnumSubKeyForTree(HTREEITEM hParantItem, CString &strPathName);
	BOOL EnumKeyValueForList(CString &strPathName);
	CString FormatError(REGSTATUS Error);
	static CString GetRegValueTypeName(unsigned long Type);
	int GetIcoIndexOfType(unsigned long Type);
	static CString FormatKeyValueData(ULONG Type,ULONG Len,PVOID Buffer);
private:
	BOOL IsRootKey(CString &strPathName, vector<PSUBKEY_ENTRY> &vPSubKey);
};
