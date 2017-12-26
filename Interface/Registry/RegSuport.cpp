#include "StdAfx.h"
//#include"ModifyFileRigh.h"
#include"RegSuport.h"
#include "../Ioctrls.h"
#include "../DataDef.h"

#pragma warning(disable:4018)

HANDLE g_hDevice = NULL;


BOOL GetRightToAccessFile(HANDLE hSor,HANDLE hDes)
{
	/*
	hSor 源句柄
	hDes 目标句柄，
	目标句柄会获得和源句柄相同的权限
	*/
	ASSERT(hSor!=0);
	ASSERT(hDes!=0);
	DWORD dwRet;
	FILE_RIGHT_MODIFY Data={0};
	Data.SourceHandle=hSor;
	Data.DesHandle=hDes;
	
	BOOL bRet = DeviceIoControl(g_hDevice, IOCTL_MODIFY_FILE_RIGHT, &Data, sizeof(FILE_RIGHT_MODIFY),\
		NULL, NULL, &dwRet, NULL);

	return bRet;
	//return TRUE;

}

BOOL GetHiveFileList( vector<PHIVE_FILE_LIST> &vHiveFileList )
{
	//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\hivelist
	HKEY hKey=0;
	PHIVE_FILE_LIST pHiveFileList = NULL;
	WCHAR ItemName[MAX_PATH] = {0};
	unsigned char ItemData[MAX_PATH] = {0};
	ULONG ulType;
	ULONG ulItemNameLen = MAX_PATH;
	ULONG ulItemDataLen = MAX_PATH;
	int index=0;



	__try
	{

		if(RegOpenKey(HKEY_LOCAL_MACHINE, Hive_Registry_Path, &hKey)!=ERROR_SUCCESS)
			return FALSE;

		while((RegEnumValueW(hKey, index++, ItemName, &ulItemNameLen, 0, &ulType, ItemData, &ulItemDataLen) == ERROR_SUCCESS))
		{
			if(ulItemDataLen > 4)
			{
				if(ulType == REG_SZ)
				{
					pHiveFileList  = (PHIVE_FILE_LIST)malloc(sizeof(HIVE_FILE_LSIT));
					RtlZeroMemory(pHiveFileList, sizeof(HIVE_FILE_LSIT));
					RtlCopyMemory(pHiveFileList->Name, ItemName, ulItemNameLen * 2);
					RtlCopyMemory(pHiveFileList->Path, ItemData, ulItemDataLen * 2);
					vHiveFileList.push_back(pHiveFileList);

				}
			}
			RtlZeroMemory(ItemName, MAX_PATH * sizeof(WCHAR));
			RtlZeroMemory(ItemData, MAX_PATH * sizeof(WCHAR));
			ulItemNameLen = MAX_PATH;
			ulItemDataLen = MAX_PATH;

		}

		RegCloseKey(hKey);
		return TRUE;
	}
	__except(1)
	{
		if(hKey)
			RegCloseKey(hKey);
		return FALSE;
	}

}

HANDLE  ForceOpenFile(char* FilePath)
{
	HANDLE  RetHandle;
	HANDLE  HandleTemp;
	RetHandle=CreateFileA(FilePath,FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(RetHandle==INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA(FilePath);
		OutputDebugString("最小权限打开失败！\n");
		return NULL;
	}
	HandleTemp=CreateFileA("thisjkjlgjhhjlfshuetemp",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if(HandleTemp==NULL)
	{
		OutputDebugString("打开源句柄失败！\n");
		return NULL;
	}
	BOOL Ret=GetRightToAccessFile(HandleTemp,RetHandle);
	CloseHandle(HandleTemp);
	DeleteFile("thisjkjlgjhhjlfshuetemp");
	if(Ret)
		return RetHandle;
	CloseHandle(RetHandle);
	OutputDebugString("修改权限失败！");
	return NULL;
}
PHIVE  NewOpenHive(char* filename, int mode)
{
   HANDLE hFile;   
 // DWORD szread;   
   struct hive *hdesc;   
   int vofs;   
   unsigned long pofs;   
   char *c;   
   struct hbin_page *p;   
   struct regf_header *hdr;     
   
   ULONG High=0;
   int verbose = (mode & HMODE_VERBOSE);   
   CREATE(hdesc,struct hive,1);   
   hdesc->filename = str_dup(filename);   
   hdesc->state = 0;   
   hdesc->size = 0;   
   hdesc->buffer = NULL;   
   hFile= ForceOpenFile(filename);
   if(hFile==NULL)
   {
 	  // dprint(L"打开文件失败！\n");
 	  FREE(hdesc);
 	  return NULL;
   }
   ULONG FileSize=GetFileSize(hFile,&High);
   hdesc->size=FileSize;
 
   HANDLE hMap=CreateFileMapping( hFile, NULL,  PAGE_READWRITE, 0, 0, 0 );
   if(hMap==NULL)
   {
 	  CloseHandle(hFile);
 	   FREE(hdesc);
 	  return NULL;
   }
   PVOID base=MapViewOfFileEx( hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0, NULL );
 
   if(!base)
   {
 	  //dprint(L"映射文件失败！\n");
 	  CloseHandle(hFile);
 	   CloseHandle(hMap);
 	   FREE(hdesc);
 	  return NULL;
 
   }
   hdesc->filehandle=(ULONG)hFile;
   hdesc->buffer=(char*)base;
   CloseHandle(hMap);
 
     pofs = 0x1000;   
    hdr = (struct regf_header *)hdesc->buffer;   
    if (hdr->id != 0x66676572)    
    {   
     // printf("openHive(%s): File does not seem to be a registry hive!\n",filename);   
      return(hdesc);   
    }   
    for (c = hdr->name; *c && (c < hdr->name + 64); c += 2)    
        putchar(*c);   
    
    hdesc->rootofs = hdr->ofs_rootkey + 0x1000;   
    while (pofs < hdesc->size)    
    {   
 #ifdef LOAD_DEBUG    
      if (verbose)    
          hexdump(hdesc->buffer,pofs,pofs+0x20,1);   
 #endif    
      p = (struct hbin_page *)(hdesc->buffer + pofs);   
      if (p->id != 0x6E696268)    
      {   
            //printf("Page at 0x%lx is not 'hbin', assuming file contains garbage at end",pofs);   
            break;   
      }   
    
      hdesc->pages++;   
 #ifdef LOAD_DEBUG    
      if (verbose)    
          printf("\n###### Page at 0x%0lx has size 0x%0lx, next at 0x%0lx ######\n",pofs,p->len_page,p->ofs_next);   
 #endif    
    
      if (p->ofs_next == 0)    
      {   
 #ifdef LOAD_DEBUG    
        if (verbose)    
            printf("openhive debug: bailing out.. pagesize zero!\n");   
 #endif    
        return(hdesc);   
      }   
    
 #if 0    
    
      if (p->len_page != p->ofs_next)   
      {   
 #ifdef LOAD_DEBUG    
        if (verbose)    
            printf("openhive debug: len & ofs not same. HASTA!\n");   
 #endif    
        exit(0);   
    
      }   
    s
 #endif    
      vofs = pofs + 0x20; /* Skip page header */   
    
 #if 1    
      while (vofs-pofs < p->ofs_next)   
      {   
        vofs += parse_block(hdesc,vofs,verbose);   
      }   
 #endif    
      pofs += p->ofs_next;   
    
    }   
   return(hdesc);   

}

void CloseHive(PHIVE hive)
{
	FlushFileBuffers((HANDLE)hive->filehandle);
 	UnmapViewOfFile(hive->buffer);
 	CloseHandle((HANDLE)hive->filehandle);
}