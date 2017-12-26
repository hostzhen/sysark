#include "StdAfx.h"
#include "HiveAnysBase.h"
#pragma comment(lib,"ntreglib.lib")
#include"RegSuport.h"
HiveAnysBase::HiveAnysBase(void)
: m_hRootNode(NULL)
{
}

HiveAnysBase::~HiveAnysBase(void)
{
	DeleteData();
}

BOOL HiveAnysBase::GetUserPath(char* OldPath, char* UserPath, ULONG ulLen)
{
	BOOL bRet = FALSE;
	strcpy(UserPath, "C:");
	strcat(UserPath, &OldPath[23]);

	return bRet;
}


BOOL HiveAnysBase::InitHiveFile()
{
	vector<PHIVE_FILE_LIST> vPHiveFileList;
	char lpName[MAX_PATH]={0}, lpPath[MAX_PATH] = {0}, lpUserPath[MAX_PATH] = {0};
	CString FilePath;
	PHIVE_LIST p = NULL;
	PHIVE pHive = NULL;
	PHIVE_LIST Seek = NULL;
	BOOL bRet = FALSE;

	if(!GetHiveFileList(vPHiveFileList))
		return bRet;
	for (int i = 0; i < vPHiveFileList.size(); i++)
	{
		wcharTochar(vPHiveFileList.at(i)->Path, lpPath, MAX_PATH);
		GetUserPath(lpPath, lpUserPath, MAX_PATH);
		pHive = NewOpenHive(lpUserPath, HMODE_RW);
		if(pHive == NULL)
		{
			continue;
		}

		if(m_hRootNode)
		{
			p = (PHIVE_LIST)malloc(sizeof(HIVE_LIST));
			ZeroMemory(p, sizeof(HIVE_LIST));
			ZeroMemory(lpName, MAX_PATH);
			wcharTochar(vPHiveFileList.at(i)->Name, lpName, MAX_PATH);
			CopyMemory(p->Name, lpName, MAX_PATH);
			p->hRoot = pHive;
			Seek->Next = p;
			Seek = p;
		}
		else
		{
			p = (PHIVE_LIST)malloc(sizeof(HIVE_LIST));
			ZeroMemory(p, sizeof(HIVE_LIST));
			ZeroMemory(lpName, MAX_PATH);
			wcharTochar(vPHiveFileList.at(i)->Name, lpName, MAX_PATH);
			CopyMemory(p->Name, lpName, strlen(lpName));
			p->hRoot = pHive;
			m_hRootNode = p;
			Seek = p;
			bRet = TRUE;
		}
	}

	return bRet;
}


VOID HiveAnysBase::DeleteData()
{
	if(m_hRootNode)
	{
		PHIVE_LIST Seek = 0;
		while(m_hRootNode)
		{
			Seek = m_hRootNode->Next;
			CloseHive(m_hRootNode->hRoot);
			m_hRootNode = Seek;			
		}
		m_hRootNode = 0;
	}
}
PHIVE  HiveAnysBase::GetKeyRootFromFullPath(char *FullPath,int * SubKeyPos)
{
	PHIVE_LIST Seek=m_hRootNode;
	SIZE_T Pos=0,i=0;
	ULONG ulSize = 0;
	CHAR lpPath[MAX_PATH] = {0};
	PCHAR pPos = NULL;

	if(Seek == 0 || FullPath == NULL || !strlen(FullPath))
		return NULL;

	if (strstr(FullPath, ROOT_HKEY_CLASSES_ROOT))
	{
		sprintf(lpPath, "%s\\SOFTWARE\\Classes", ROOT_HKEY_LOCAL_MACHINE);
		Pos = strlen(ROOT_HKEY_CLASSES_ROOT);
		strcat(lpPath, &FullPath[Pos]);
		strcpy(FullPath, lpPath);
	}
	else if (strstr(FullPath, ROOT_HKEY_CURRENT_CONFIG))
	{
		sprintf(lpPath, "%s\\SYSTEM\\ControlSet004\\Hardware Profiles\\Current", ROOT_HKEY_CURRENT_CONFIG);
		Pos = strlen(ROOT_HKEY_CURRENT_CONFIG);
		strcat(lpPath, &FullPath[Pos]);
		strcpy(FullPath, lpPath);
	}

	while(Seek)
	{
		if(strlen(Seek->Alias) && (pPos = strstr(FullPath, Seek->Alias)))
		{
			if (strcmp(FullPath, Seek->Alias))
			{
				//避免\REGISTRY\USER\S-1-5-20_Classes和\REGISTRY\USER\S-1-5-20这种情况
				int nNameLen = strlen(Seek->Alias);
				if (FullPath[nNameLen] != '\\')
				{
					Seek = Seek->Next;
					continue;
				}
			}
			
			ulSize = strlen(Seek->Alias);
			strcpy(lpPath, Seek->Name);
			strcat(lpPath, pPos + ulSize);
			strcpy(FullPath, lpPath);
			Pos = 0;
			
			if(SubKeyPos != 0)
			{
				*SubKeyPos = 0;
				for(i = 0; i < strlen(FullPath); i++)
				{
					if(FullPath[i] == '\\')
					{
						Pos++;
						if(Pos == 4)
						{
							break;
						}
					}
				}
				if(Pos == 4)
				{
					*SubKeyPos = i + 1;
				}
			}
			return Seek->hRoot;
		}
		Seek=Seek->Next;
	}
	return NULL;
}

int HiveAnysBase::GetSubKeyCount(char *FullPath)
{
	int Pos=0;
	struct nk_key *key ;
	int nkofs = 0, count = 0, countri = 0;
	PHIVE hRoot=GetKeyRootFromFullPath(FullPath,&Pos);
	if(hRoot==0)
		return -1;
	nkofs = trav_path( hRoot, hRoot->rootofs + 4, FullPath+Pos, TPF_NK_EXACT );
	if ( !nkofs ) 
	{
		return -1;
	}
	nkofs += 4;
	key = (struct nk_key *)(hRoot->buffer + nkofs);
	
	if (key->id != 0x6b6e)
	{
		return -1;
	}	
	return (int)key->no_subkeys ;
}
REGSTATUS HiveAnysBase::EnumSubKey(char *lpFullPath, vector<PSUBKEY_ENTRY> &vPSubKey)
{
	int Pos=0;
	struct nk_key *key ;
	int nkofs = 0, count = 0, countri = 0;
	int RetCount;
	struct ex_data ex;
	PSUBKEY_ENTRY pSubKey = NULL;
	CHAR FullPath[MAX_PATH] = {0};
	strcpy(FullPath, lpFullPath);

	PHIVE hdesc = GetKeyRootFromFullPath(FullPath, &Pos);
	if(hdesc==0)
		return REG_NOT_FIND;
	if(Pos > 6)
	{
		nkofs = trav_path( hdesc, hdesc->rootofs + 4, FullPath+Pos, TPF_NK_EXACT );
		if ( !nkofs ) 
		{
			return REG_NOT_FIND;
		}
		nkofs += 4;
	}
	else
	{
		nkofs = trav_path( hdesc, hdesc->rootofs + 4,"", TPF_VK_EXACT );
		if ( !nkofs ) 
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	key = (struct nk_key *)(hdesc->buffer + nkofs);

	if(key->no_subkeys)
	{
		
		while(ex_next_n(hdesc,nkofs,&count,&countri,&ex)>0)
		{
			if(ex.name)
			{
				pSubKey = (PSUBKEY_ENTRY)malloc(sizeof(SUB_KEY_ENTRY));
				ZeroMemory(pSubKey, sizeof(SUB_KEY_ENTRY));
				CopyMemory(pSubKey->Name, ex.name, strlen(ex.name));
				vPSubKey.push_back(pSubKey);
				FREE(ex.name);
			}
		}
	}
	return REG_SUCCESS;
}

REGSTATUS HiveAnysBase::EnumKeyValue(char *lpFullPath, vector<PKEY_VALUE_ENTRY> &vPKeyValue)
{
	int Pos=0;
	struct nk_key *key ;
	int nkofs = 0, count = 0, countri = 0;
	int RetCount;
	struct vex_data  vex={0};
	PKEY_VALUE_ENTRY pKeyValue = NULL;
	CHAR FullPath[MAX_PATH] = {0};
	strcpy(FullPath, lpFullPath);

	PHIVE hdesc=GetKeyRootFromFullPath(FullPath,&Pos);
	if(hdesc==0)
		return REG_NOT_FIND;
	if(Pos > 6)
	{
		nkofs = trav_path( hdesc, hdesc->rootofs + 4, FullPath+Pos, TPF_NK_EXACT );
		if ( !nkofs ) 
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	else
	{
		nkofs = trav_path( hdesc, hdesc->rootofs + 4,"", TPF_NK_EXACT );
		if ( !nkofs ) 
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	key = (struct nk_key *)(hdesc->buffer + nkofs);

	if(key->no_values)
	{
		while(ex_next_v(hdesc,nkofs,&count,&vex) > 0)
		{
			if(vex.name)
			{
				pKeyValue = (PKEY_VALUE_ENTRY)malloc(sizeof(KEY_VALUE_ENTRY));
				ZeroMemory(pKeyValue, sizeof(KEY_VALUE_ENTRY));
				RtlCopyMemory(pKeyValue->Name, vex.name, strlen(vex.name));
				pKeyValue->Type = vex.type;
				
				if(vex.type==REG_DWORD)
				{
					pKeyValue->DataLen = 4;
				}
				else
				{
					pKeyValue->DataLen = vex.size;
				}
					
				pKeyValue->DataBuffer = (UCHAR*)malloc(vex.size+8);
				if(!InternalGetValueData(hdesc, nkofs, vex.name, vex.size, pKeyValue))
				{
					free(pKeyValue->DataBuffer);
					free(pKeyValue);
					pKeyValue = NULL;
				}
				else
				{
					vPKeyValue.push_back(pKeyValue);
				}

				FREE(vex.name);
			}
		}

	}
	return REG_SUCCESS;
}

bool HiveAnysBase::InternalGetValueData(PHIVE hdesc, int nkofs, char *name,int len,PKEY_VALUE_ENTRY pData)
{
        void* data;
	data = (void *)get_val_data(hdesc, nkofs, name, 0);
	if(data)
	{
		__try
		{
			RtlCopyMemory(pData->DataBuffer,data,len);
			pData->DataLen=len;
		}
		__except(1)
		{
			return 1;
		}
	}
	return true;
}
REGSTATUS HiveAnysBase::DeleteSubKey(char *FullPath)
{
	int Pos=0;
	int nkofs = 0;
	char name[MAX_PATH]={0};
	int End=0;
	char * parent=0;	
	PHIVE hdesc=GetKeyRootFromFullPath(FullPath,&Pos);
	if(hdesc==0)
		return  REG_NOT_FIND;
	if(Pos==0)
	{
		nkofs=trav_path( hdesc, hdesc->rootofs + 4,"", TPF_NK_EXACT );
		if(!nkofs)
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	else
	{
		parent=new char[2048];
		RtlZeroMemory(parent,2048);
		for(End=strlen(FullPath);End>Pos;End--)
		{
			if(FullPath[End]=='\\')
			{
				RtlCopyMemory(name,FullPath+End+1,strlen(FullPath+End+1));
				RtlCopyMemory(parent,FullPath+Pos,End-Pos);
				break;
			}

		}
		if(End==Pos)
		{
			delete parent;
			return REG_CAN_NOT_DELETE;

		}
		nkofs = trav_path( hdesc, hdesc->rootofs + 4, parent, TPF_NK_EXACT );
		if ( !nkofs ) 
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;

		delete parent;
	}

	rdel_keys(hdesc,name,nkofs);
	FlushFileBuffers((HANDLE)hdesc->filehandle);

	return REG_SUCCESS;
}

REGSTATUS HiveAnysBase::DeleteValue(char *FullPath, char *Name)
{
	int Pos=0;
	int nkofs = 0;
	PHIVE hdesc=GetKeyRootFromFullPath(FullPath,&Pos);
	if(hdesc==0)
		return REG_NOT_FIND;
	if(Pos==0)
	{
			nkofs=trav_path( hdesc, hdesc->rootofs + 4,"", TPF_NK_EXACT );
		if(!nkofs)
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	else
	{
		nkofs = trav_path( hdesc, hdesc->rootofs + 4, FullPath+Pos, TPF_NK_EXACT );
		if ( !nkofs ) 
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	if(del_value(hdesc,nkofs,Name)==0)
		return REG_SUCCESS;
	return REG_UNSUCCESSFUL;

}

REGSTATUS HiveAnysBase::SetKeyValueData(char *FullPath, char *Name, BYTE *Data, ULONG DataSize)//如果是字符串，请提前转化成UNICODE编码
{
	//
	int Pos=0;
	int nkofs = 0;
	int Type=0;
	struct keyval * value=0;
	PHIVE hdesc=GetKeyRootFromFullPath(FullPath,&Pos);
	if(hdesc==0)
		return REG_NOT_FIND;
	nkofs = trav_path( hdesc, hdesc->rootofs + 4, FullPath+Pos, TPF_NK_EXACT );
	if(!nkofs)
		return REG_NOT_FIND;
	nkofs+=4;

	Type=get_val_type(hdesc,nkofs,Name);
	if(Type==REG_DWORD)
	{
		if(put_dword(hdesc,nkofs,Name,*(DWORD*)Data))
		{
			FlushFileBuffers((HANDLE)hdesc->filehandle);
			return REG_SUCCESS;
		}
		return REG_UNSUCCESSFUL;
	}
	else
	{
		value=(struct keyval*)malloc(sizeof(struct keyvala)+DataSize+8);
		RtlCopyMemory(&value->data,Data,DataSize);
		value->len=DataSize;
		if(put_buf2val(hdesc,value,nkofs,Name,Type))
		{
			free(value);
			FlushFileBuffers((HANDLE)hdesc->filehandle);
			return REG_SUCCESS;
		}

	}
	return REG_UNSUCCESSFUL;

}

REGSTATUS HiveAnysBase::AddSubKey(char *FullPath, char *Name)
{
	int Pos=0;
	int nkofs = 0, count = 0, countri = 0;
	PHIVE hdesc=GetKeyRootFromFullPath(FullPath,&Pos);
	if(hdesc==0)
		return REG_NOT_FIND;
	if(Pos==0)
	{
		nkofs=trav_path( hdesc, hdesc->rootofs + 4,"", TPF_NK_EXACT );
		if(!nkofs)
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	else
	{
		nkofs = trav_path( hdesc, hdesc->rootofs + 4, FullPath+Pos, TPF_NK_EXACT );
		if(!nkofs)
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	if(IsKeyInHiveFile(hdesc,nkofs,Name))
	{
		return REG_ARIDY_IN;
	}

	if(add_key(hdesc,nkofs,Name)!=0)
	{
		FlushFileBuffers((HANDLE)hdesc->filehandle);
		return REG_SUCCESS;
	}
	return REG_UNSUCCESSFUL;
}

REGSTATUS HiveAnysBase::AddValue(char *FullPath, char *Name, BYTE *Data, ULONG DataSize, ULONG Type)
{
	int Pos=0;
	int nkofs = 0, count = 0, countri = 0;
	REGSTATUS Ret=REG_UNSUCCESSFUL;
	PHIVE hdesc=GetKeyRootFromFullPath(FullPath,&Pos);
	if(hdesc==0)
		return REG_NOT_FIND;
	if(Pos==0)
	{
				nkofs=trav_path( hdesc, hdesc->rootofs + 4,"", TPF_NK_EXACT );
		if(!nkofs)
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	else
	{
		nkofs = trav_path( hdesc, hdesc->rootofs + 4, FullPath+Pos, TPF_NK_EXACT );
		if(!nkofs)
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	if(NULL!=add_value(hdesc,nkofs,Name,Type))
	{
		Ret=SetKeyValueData(FullPath,Name,Data,DataSize);
		FlushFileBuffers((HANDLE)hdesc->filehandle);
		return Ret;
	}
	return Ret;
}

bool HiveAnysBase::IsKeyInHiveFile(PHIVE hdesc,int nkofs,char *name)
{
	int Pos=0;
	struct nk_key *key ;
	int count = 0, countri = 0;
	struct ex_data ex;
	key = (struct nk_key *)(hdesc->buffer + nkofs);
	if(key->no_subkeys)
	{
		
		while(ex_next_n(hdesc,nkofs,&count,&countri,&ex)>0)
		{
			if(ex.name)
			{
				if(strcmp(ex.name,name)==0)
				{
					FREE(ex.name);
					return 1;
				}
				FREE(ex.name);
	
			}
		}
	}
	return 0;
}

REGSTATUS HiveAnysBase::SetKeyValueName(char *FullPath, char *OldName, char *NewName)
{
	int Pos=0;
	int nkofs = 0, count = 0, countri = 0;
	REGSTATUS Ret=REG_UNSUCCESSFUL;
	PHIVE hdesc=GetKeyRootFromFullPath(FullPath,&Pos);
	if(hdesc==0)
		return REG_NOT_FIND;
	if(Pos==0)
	{
				nkofs=trav_path( hdesc, hdesc->rootofs + 4,"", TPF_NK_EXACT );
		if(!nkofs)
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}
	else
	{
		nkofs = trav_path( hdesc, hdesc->rootofs + 4, FullPath+Pos, TPF_NK_EXACT );
		if(!nkofs)
		{
			return REG_NOT_FIND;
		}
		nkofs+=4;
	}

	int len=get_val_len(hdesc,nkofs,OldName);
	void * data=0;
	if(len>0)
	{
		data=get_val_data(hdesc,nkofs,OldName,0);
	}
	Ret=AddValue(FullPath,NewName,len>0?(BYTE*)data:0,len,get_val_type(hdesc,nkofs,OldName));

	if(Ret==REG_SUCCESS)
	{
		Ret= DeleteValue(FullPath,OldName);
		if(Ret!=REG_SUCCESS)
		{
			DeleteValue(FullPath,NewName);
		}
	}
	FlushFileBuffers((HANDLE)hdesc->filehandle);
	return Ret;

}