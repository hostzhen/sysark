#include "DataDef.h"

//全局变量
PLIST_ENTRY KiDispatcherReadyListHead = NULL;
PLIST_ENTRY KiWaitListhead =         NULL;
SysarkZwQueryVirtualMemory ZwQueryVirtualMemory;



PVOID GetZwQueryVirtualMemoryAddress()
{
	PVOID FunctionAddress=0;
	ULONG ulSearchStart;
	int Index = 0;
	PUCHAR i=0;

	/*
	lkd> u ZwQueryVirtualMemory
	nt!ZwQueryVirtualMemory:
	804ffb90 b8b2000000      mov     eax,0B2h
	804ffb95 8d542404        lea     edx,[esp+4]
	804ffb99 9c              pushfd
	804ffb9a 6a08            push    8
	804ffb9c e8f0e80300      call    nt!KiSystemService (8053e491)
	804ffba1 c21800          ret     18h
	nt!ZwQueryVolumeInformationFile:                 <------------------从这里一直往上减，当Index一样的时候，就是地址了。
	804ffba4 b8b3000000      mov     eax,0B3h
	804ffba9 8d542404        lea     edx,[esp+4]
	*/
	//GetFunctionIndexByName("ZwQueryVirtualMemory",&Index);
	//if (Index)
	{
		ulSearchStart = (ULONG)ZwQueryVolumeInformationFile;
		for (i=(PUCHAR)ulSearchStart;i > (PUCHAR)ulSearchStart - 0x50;i--)
		{
			//if (MmIsAddressValidEx(i))
			{
				if (*i == Index){
					FunctionAddress = i - 1;

					//if (DebugOn)
						KdPrint(("FunctionAddress:%08x\n",FunctionAddress));
					break;
				}
			}
		}
	}
	return FunctionAddress;
}

#pragma INITCODE
EXTERN_C NTSTATUS DataInitialize()
{
	NTSTATUS status = STATUS_SUCCESS;

	ZwQueryVirtualMemory = (SysarkZwQueryVirtualMemory)GetZwQueryVirtualMemoryAddress();


	/************************************************************************/
	/*全局变量的地址可以搜索用到该变量的函数，例如系统中用到 KiWaitInListHead的例程有
	  KeWaitForSingleObject等，不过这里使用硬编码
	  */
	/************************************************************************/
	//这个地址也可以搜索用到该变量的函数
	//WINXP
	KiWaitListhead = (PLIST_ENTRY)0x80553d88;
	KiDispatcherReadyListHead = (PLIST_ENTRY)0x80554820;

	return status;
}
#pragma PAGEDCODE
EXTERN_C ULONG GetUndocumentFuncAddrByName( PUNICODE_STRING lpFuncName )
{

	ULONG ulFuncAddr;

	ulFuncAddr = (ULONG) MmGetSystemRoutineAddress(lpFuncName);//MmGetSystemRoutineAddress可以通过函数名获得函数地址

	return ulFuncAddr;
}

#pragma PAGEDCODE
EXTERN_C BOOLEAN IsProcess(PVOID Eprocess)   
{   
	ULONG CurrentPeb = 0;   
	ULONG Peb = 0;
	ULONG flags;
	_PsGetProcessPeb PsGetProcessPeb;
	if (MmIsAddressValid(Eprocess))   
	{   
		//UNICODE_STRING usFuncName;
		//RtlInitUnicodeString(&usFuncName, L"PsLookupProcessByProcessId");
		//PsGetProcessPeb = (_PsGetProcessPeb)GetUndocumentFuncAddrByName(&usFuncName);
		//CurrentPeb = (ULONG)PsGetProcessPeb(PsGetCurrentProcess()) & 0xFFFF0000;       
		//Peb = (ULONG)PsGetProcessPeb((PEPROCESS)Eprocess) & 0xFFFF0000;   
		//if (CurrentPeb == Peb) return TRUE;   

		flags = *(PULONG)((ULONG)Eprocess + Flags_EPROCESS);
		if ((flags & 0xc) != 0xc)
		{
			return TRUE;
		}

	}   
	return FALSE;   
} 


#pragma PAGEDCODE
EXTERN_C BOOLEAN GetFunctionNameByIndex( ULONG ulModuleBase, PULONG Index, PCHAR lpszFunctionName )
{
	UNICODE_STRING wsNtDllString;

	HANDLE hNtSection;
	ULONG ulNtDllModuleBase;
	PIMAGE_DOS_HEADER pDosHeader;
	PIMAGE_NT_HEADERS NtDllHeader;

	IMAGE_OPTIONAL_HEADER opthdr;
	DWORD* arrayOfFunctionAddresses;
	DWORD* arrayOfFunctionNames;
	WORD* arrayOfFunctionOrdinals;
	DWORD functionOrdinal;
	DWORD Base, x, functionAddress,position;
	PCHAR functionName;
	IMAGE_EXPORT_DIRECTORY *pExportTable;
	BOOLEAN bRetOK = FALSE;
	BOOLEAN bInit = FALSE;

	__try
	{
		//NtDllHeader=(PIMAGE_NT_HEADERS)GetPeHead((ULONG)ulNtDllModuleBase);
		ulNtDllModuleBase = (ULONG)ulModuleBase;
		pDosHeader=(PIMAGE_DOS_HEADER)ulNtDllModuleBase;
		if (pDosHeader->e_magic!=IMAGE_DOS_SIGNATURE)
		{
			//if (DebugOn)
				KdPrint(("failed to find NtHeader\r\n"));
			return bRetOK;
		}
		NtDllHeader=(PIMAGE_NT_HEADERS)(ULONG)((ULONG)pDosHeader+pDosHeader->e_lfanew);
		if (NtDllHeader->Signature!=IMAGE_NT_SIGNATURE)
		{
			//if (DebugOn)
				KdPrint(("failed to find NtHeader\r\n"));
			return bRetOK;
		}
		opthdr = NtDllHeader->OptionalHeader;
		pExportTable =(IMAGE_EXPORT_DIRECTORY*)((PUCHAR)ulNtDllModuleBase + opthdr.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]. VirtualAddress); //得到导出表
		arrayOfFunctionAddresses = (DWORD*)( (PUCHAR)ulNtDllModuleBase + pExportTable->AddressOfFunctions);  //地址表
		arrayOfFunctionNames = (DWORD*)((PUCHAR)ulNtDllModuleBase + pExportTable->AddressOfNames);         //函数名表
		arrayOfFunctionOrdinals = (WORD*)( (PUCHAR)ulNtDllModuleBase + pExportTable->AddressOfNameOrdinals);

		Base = pExportTable->Base;

		for(x = 0; x < pExportTable->NumberOfFunctions; x++) //在整个导出表里扫描
		{
			functionName = (PCHAR)( (PUCHAR)ulNtDllModuleBase + arrayOfFunctionNames[x]);
			functionOrdinal = arrayOfFunctionOrdinals[x] + Base - 1; 
			functionAddress = (DWORD)((PUCHAR)ulNtDllModuleBase + arrayOfFunctionAddresses[functionOrdinal]);
			position  = *((WORD*)(functionAddress + 1));  //得到服务号

			if (*Index == position)
			{
				//if (DebugOn)
					KdPrint(("search success[%s]",functionName));

				strcat(lpszFunctionName,functionName);
				bRetOK = TRUE;
				break;
			}
		}

	}__except(EXCEPTION_EXECUTE_HANDLER){
		KdPrint(("EXCEPTION_EXECUTE_HANDLER[%08x]",GetExceptionCode()));
	}
	return bRetOK;
}


#pragma PAGEDCODE
EXTERN_C HANDLE MapFileAsSection(PUNICODE_STRING FileName,PVOID *ModuleBase)
{
	NTSTATUS status;
	HANDLE  hSection, hFile;
	DWORD dwKSDT;
	PVOID BaseAddress = NULL;
	SIZE_T size=0;
	IO_STATUS_BLOCK iosb;
	OBJECT_ATTRIBUTES oa = {sizeof oa, 0, FileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE};
	BOOLEAN bInit = FALSE;


	*ModuleBase=NULL;

	status=ZwOpenFile(
		&hFile, 
		FILE_EXECUTE | SYNCHRONIZE, 
		&oa, 
		&iosb, 
		FILE_SHARE_READ, 
		FILE_SYNCHRONOUS_IO_NONALERT);

	if(!NT_SUCCESS(status))
	{
		//if (DebugOn)
			KdPrint(("ZwOpenFile failed\n"));
		return NULL;
	}
	oa.ObjectName = 0;

	status=ZwCreateSection(
		&hSection, 
		SECTION_ALL_ACCESS, 
		&oa, 
		0,
		PAGE_EXECUTE, 
		SEC_IMAGE, 
		hFile);
	if(!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		//KdPrint(("ZwCreateSection failed:%d\n",RtlNtStatusToDosError(status)));
		return NULL;
	}

	status=ZwMapViewOfSection(
		hSection, 
		NtCurrentProcess(),
		&BaseAddress, 
		0,
		1000, 
		0,
		&size,
		(SECTION_INHERIT)1,
		MEM_TOP_DOWN, 
		PAGE_READWRITE); 
	if(!NT_SUCCESS(status))
	{
		ZwClose(hFile);
		ZwClose(hSection);

		//if (DebugOn)
			//KdPrint(("ZwMapViewOfSection failed %d\n",RtlNtStatusToDosError(status)));
		return NULL;
	}
	ZwClose(hFile);
	__try
	{
		*ModuleBase=BaseAddress;
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		return NULL;
	}
	return hSection;
}