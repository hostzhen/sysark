#pragma once
#ifndef HIVE_BASE_HEADER_
#define HIVE_BASE_HEADER_
#include"RegSuport.h"

#define REGSTATUS   ULONG 


#define REG_SUCCESS  0x00000000
#define REG_BUFFER_TOO_SMALL  0xc0000001
#define REG_DATA_TOO_LARGE 0xc0000002
#define REG_NOT_FIND 0xc0000003
#define REG_IS_INCLUDE 0xc0000004
#define REG_VALUE_NO_DATA  0xc0000005
#define REG_UNSUCCESSFUL 0xc0000006
#define REG_CAN_NOT_DELETE 0xc0000007
#define REG_NOT_SUPORT       0xc0000008
#define REG_ARIDY_IN  0xc0000009




typedef struct tagHIVE_LIST //hive列表
{
	char  Name[MAX_PATH];	//名称，eg.\REGISTRY\MACHINE\SECURITY
	char  Alias[MAX_PATH];	//别名，eg.SECURITY
	PHIVE   hRoot;
	struct tagHIVE_LIST* Next;
}HIVE_LIST,*PHIVE_LIST;

typedef struct tagSUBKEY_BUFFER/*EnumSubKey返回的数据结构*/
{
	HTREEITEM hParantItem;
	char Name[MAX_PATH];
}SUB_KEY_ENTRY,*PSUBKEY_ENTRY;


typedef struct tagKeyValue/*EnumValue返回的数据结构*/
{
	ULONG Type;
	ULONG DataLen;
	char  Name[MAX_PATH];
	unsigned char * DataBuffer;
}KEY_VALUE_ENTRY,*PKEY_VALUE_ENTRY;



class HiveAnysBase
{
public:
	HiveAnysBase(void);
	~HiveAnysBase(void);
private:	
	bool InternalGetValueData(PHIVE  hdesc, int nkof,char* name,int Valuelen,PKEY_VALUE_ENTRY pData);
	bool  IsKeyInHiveFile(PHIVE hdesc,int nkofs,char *name);
	BOOL  GetUserPath(char* OldPath, char* UserPath, ULONG ulLen);
protected:	
	PHIVE  GetKeyRootFromFullPath(char* FullPath,int *);
	PHIVE_LIST  m_hRootNode;
	BOOL InitHiveFile();
	VOID DeleteData();
public:
	int GetSubKeyCount(char* FullPath);
	REGSTATUS  EnumSubKey(char * lpFullPath, vector<PSUBKEY_ENTRY> &vPSubKey);
	REGSTATUS  EnumKeyValue(char* lpFullPath, vector<PKEY_VALUE_ENTRY> &vPKeyValue);
	REGSTATUS  AddSubKey(char* FullPath,char* Name);
	REGSTATUS  AddValue(char *FullPath,char *Name,BYTE * Data,ULONG DataSize,ULONG Type);
	REGSTATUS  DeleteSubKey(char *FullPath);
	REGSTATUS  DeleteValue(char* FullPath,char * Name);
	REGSTATUS  SetKeyValueData(char * FullPath,char *Name, BYTE *Data,ULONG DataSize);
	REGSTATUS  SetKeyValueName(char* FullPath,char * OldName,char * NewName);

	
};

#endif 
