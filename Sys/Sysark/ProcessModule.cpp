#include "ProcessModule.h"


extern SysarkZwQueryVirtualMemory ZwQueryVirtualMemory;

/************************************
* 函数名称:  EnumProcessModuleByPEB
* 功能描述:  遍历 PEBPEB 里 面的双向链表枚举进程模块
* 参数列表:  PEPROCESS pEProcess
* 参数列表:  PALL_PROCESS_MODULE_INFO pAllModuleInf
* 返回 值:   EXTERN_C VOID
/************************************/
EXTERN_C VOID EnumProcessModuleByPEB( IN PEPROCESS pEProcess, IN PALL_PROCESS_MODULE_INFO pAllModuleInf )
{
	PVOID PEB         = NULL,
		Ldr         = NULL,
		Flink       = NULL,
		p           = NULL,
		BaseAddress = NULL;
	ULONG uCount = 0;
	ULONG SizeOfImage = 0;
	PUNICODE_STRING FullDllName;
	ASSERT(pEProcess != NULL);

	KeAttachProcess(pEProcess);

	PEB = *(PVOID*)( (( PUCHAR)pEProcess) + 0x1b0 );
	Ldr   = *(PVOID*)( (( PUCHAR)PEB) + 0x0c );
	Flink =  ( PUCHAR)Ldr + 0x0c ;
	p     = *(PVOID*)(Flink);
	//按加载顺序遍历:
	do
	{
		BaseAddress = *( ( PVOID* )( ( PUCHAR )p + 0x18 ) );
		SizeOfImage = *(ULONG*)((PUCHAR)p + 0x20);
		FullDllName = ( ( PUNICODE_STRING)( ( PUCHAR )p + 0x24 ) );
		if (Flink != (PVOID)FullDllName)
		{
			pAllModuleInf->vModuleInf[uCount].BaseAddress = (ULONG)BaseAddress;
			pAllModuleInf->vModuleInf[uCount].size = SizeOfImage;
			RtlCopyMemory(pAllModuleInf->vModuleInf[uCount++].ImagePathName, FullDllName->Buffer, FullDllName->Length);
		}
		p = *( ( PVOID* )p );
	}
	while ( Flink != p );
	pAllModuleInf->uCount = uCount - 1;

	KeDetachProcess();
}

/************************************
* 函数名称:  EnumProcessModuleByVM
* 功能描述:  通过遍历进程的虚拟内存来查找进程的模块
* 参数列表:  PEPROCESS pEProcess
* 参数列表:  PALL_PROCESS_MODULE_INFO pAllModuleInf
* 返回 值:   VOID
/************************************/
EXTERN_C VOID EnumProcessModuleByVM( PEPROCESS pEProcess, PALL_PROCESS_MODULE_INFO pAllModuleInf )
{

	ULONG dwStartAddr = 0x00000000;
	HANDLE hProcess;
	MEMORY_BASIC_INFORMATION mbi;
	PUNICODE_STRING pSectionNam = NULL;
	OBJECT_ATTRIBUTES ObjectAttributes;
	CLIENT_ID ClientId={0};
	NTSTATUS status;
	int i=0;
	int count = -1;
	ULONG ulDllSize;
	//BOOL bInit = FALSE;



	//WinVer = GetWindowsVersion();
	//switch(WinVer)
	//{
	//case WINDOWS_VERSION_XP:
	//	if (!MmIsAddressValidEx(ZwQueryVirtualMemory)){
	//		ZwQueryVirtualMemory = GetZwQueryVirtualMemoryAddress();
	//	}
	//	break;
	//case WINDOWS_VERSION_7_7600_UP:
	//case WINDOWS_VERSION_7_7000:
	//	ReLoadNtosCALL((PVOID)(&ZwQueryVirtualMemory),L"ZwQueryVirtualMemory",SystemKernelModuleBase,(ULONG)ImageModuleBase); //win7导出了，可以直接用
	//	break;
	//case WINDOWS_VERSION_2K3_SP1_SP2:
	//	if (!MmIsAddressValidEx(ZwQueryVirtualMemory)){
	//		ZwQueryVirtualMemory = GetZwQueryVirtualMemoryAddress();
	//	}
	//	break;
	//}
	//ReLoadNtosCALL((PVOID)(&RObOpenObjectByPointer),L"ObOpenObjectByPointer",SystemKernelModuleBase,(ULONG)ImageModuleBase);
	//ReLoadNtosCALL((PVOID)(&RExAllocatePool),L"ExAllocatePool",SystemKernelModuleBase,(ULONG)ImageModuleBase);
	//ReLoadNtosCALL((PVOID)(&RExFreePool),L"ExFreePool",SystemKernelModuleBase,(ULONG)ImageModuleBase);
	//ReLoadNtosCALL((PVOID)(&RZwClose),L"ZwClose",SystemKernelModuleBase,(ULONG)ImageModuleBase);
	//if (ZwQueryVirtualMemory &&
	//	RObOpenObjectByPointer &&
	//	RExAllocatePool &&
	//	RExFreePool &&
	//	RZwClose)
	//{
	//	bInit = TRUE;
	//}
	//if (!bInit)
	//	return ;

	////进程已经退出鸟
	//if (!IsExitProcess((PEPROCESS)EProcess)){
	//	return ;
	//}
	status = ObOpenObjectByPointer(
		(PVOID)pEProcess,          // Object    
		OBJ_KERNEL_HANDLE,  // HandleAttributes    
		NULL,               // PassedAccessState OPTIONAL    
		(ACCESS_MASK)0,       // DesiredAccess    
		*PsProcessType,     // ObjectType    
		KernelMode,         // AccessMode    
		&hProcess);    
	if (!NT_SUCCESS(status))
	{
		//if (DebugOn)
		//KdPrint(("ObOpenObjectByPointer failed:%d",RtlNtStatusToDosError(status)));
		return ;
	}
	pSectionNam = (PUNICODE_STRING)ExAllocatePool(NonPagedPool, MAX_PATH * sizeof(WCHAR));
	if (!pSectionNam)
	{
		ZwClose(hProcess);
		return;
	}
	RtlZeroMemory(pSectionNam->Buffer, MAX_PATH * sizeof(WCHAR));
	pSectionNam->MaximumLength = MAX_PATH;

	__try
	{
		for (dwStartAddr = 0; dwStartAddr < 0x7fffffff; dwStartAddr = dwStartAddr + 0x10000)
		{
			status = ZwQueryVirtualMemory(
				hProcess,
				(PVOID)dwStartAddr,
				MemoryBasicInformation,
				&mbi,
				sizeof(MEMORY_BASIC_INFORMATION),
				0
				);
			if (NT_SUCCESS(status) && mbi.Type == MEM_IMAGE)
			{
				status = ZwQueryVirtualMemory(
					hProcess,
					(PVOID)dwStartAddr,
					MemorySectionName,
					pSectionNam,
					MAX_PATH*sizeof(WCHAR),
					0
					);
				if (NT_SUCCESS(status) && !MmIsAddressValid(pSectionNam->Buffer))
				{
					//如果当前的DLL模块路径 不等于前一个，则说明是开始了下一个DLL的枚举了，首先判断第一个字节是否是'\'
					if (((PCHAR)pSectionNam->Buffer)[0] == 0x5c)
					{ 
						if (count == -1 ||\
							RtlCompareMemory(pAllModuleInf->vModuleInf[count].ImagePathName, pSectionNam->Buffer, pSectionNam->Length)\
							!= pSectionNam->Length)
						{
							RtlCopyMemory(pAllModuleInf->vModuleInf[++count].ImagePathName, pSectionNam->Buffer, pSectionNam->Length);
							for (ulDllSize = dwStartAddr+mbi.RegionSize; ulDllSize < 0x7fffffff; ulDllSize += mbi.RegionSize)
							{
								status = ZwQueryVirtualMemory(
									hProcess,
									(PVOID)ulDllSize,
									MemoryBasicInformation,
									&mbi,
									sizeof(MEMORY_BASIC_INFORMATION),
									0
									);
								if (NT_SUCCESS(status) && mbi.Type != MEM_IMAGE)
								{
									pAllModuleInf->vModuleInf[count].size = ulDllSize - dwStartAddr;
								}
							}
						}
					}
				}
			}
		}

	}__except(EXCEPTION_EXECUTE_HANDLER){}
_FunctionRet:
	pAllModuleInf->uCount = count;
	ExFreePool(pSectionNam);
	ZwClose(hProcess);
}


NTSTATUS UnmapViewOfModule(IN HANDLE hProcess, IN PVOID lpBaseAddr )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	status = ZwUnmapViewOfSection(hProcess, lpBaseAddr);

	return status;
}
