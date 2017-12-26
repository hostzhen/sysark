#include "Process.h"


extern PEPROCESS g_pSystemProcess;
extern PLIST_ENTRY KiDispatcherReadyListHead;
extern PLIST_ENTRY KiWaitListhead;

BOOLEAN ProcessIsFind( ULONG uPID, PALL_PROCESSES_INFO pAllProcessInfo )
{
	DWORD dwCount;
	for (dwCount = 0; dwCount < pAllProcessInfo->uCount; dwCount++)
	{
		if (pAllProcessInfo->vProcessInf[dwCount].ProcessId == uPID)
		{
			return TRUE;
		}
	}
	return FALSE;
}

VOID DisplayList(PLIST_ENTRY ListHead, PALL_PROCESSES_INFO pAllProcessInfo)
{
	PKTHREAD Thread;
	PKPROCESS pProcess;
	PEPROCESS pEprocess, pCurProcess;
	DWORD dwCount = pAllProcessInfo->uCount;
	PLIST_ENTRY List = ListHead->Flink;
	if ( List == ListHead )
	{
		return;
	}
	PLIST_ENTRY NextList = List;
	pCurProcess = PsGetCurrentProcess();
	while ( NextList != ListHead )
	{
		Thread = CONTAINING_RECORD(NextList, KTHREAD, WaitListEntry);
		pProcess = (PKPROCESS)Thread->ApcState.Process;
		if (NULL == (ULONG)pProcess)
		{
			KdPrint(("获取失败\n"));
			NextList = NextList->Flink;
			break;
		}
		pEprocess = (PEPROCESS)pProcess;
		if (pCurProcess == pEprocess)
		{
			//_asm int 3
		}
		if (!ProcessIsFind(*(PULONG)((PUCHAR)pEprocess + UniqueProcessId_EPROCESS), pAllProcessInfo))
		{
			pAllProcessInfo->vProcessInf[dwCount].FatherProcessId = *(PULONG)((PUCHAR)pEprocess + InheritedFromUniqueProcessId_EPROCESS);
			pAllProcessInfo->vProcessInf[dwCount].pEProcess = pEprocess;
			pAllProcessInfo->vProcessInf[dwCount].ProcessId = *(PULONG)((PUCHAR)pEprocess + UniqueProcessId_EPROCESS);
			if ((INT)pAllProcessInfo->vProcessInf[dwCount].ProcessId < 0)
			{
				pAllProcessInfo->vProcessInf[dwCount].ProcessId = 0;
			}
			pAllProcessInfo->vProcessInf[dwCount].bHide = NT_SUCCESS(IsHideProcess(pEprocess)) ? TRUE : FALSE;
			RtlCopyMemory(pAllProcessInfo->vProcessInf[dwCount].ImageFileName, (PUCHAR)pEprocess + ImageFileName_EPROCESS, MAX_PATH);
			GetProcessImagePath(pAllProcessInfo->vProcessInf[dwCount].ProcessId, pAllProcessInfo->vProcessInf[dwCount].ImagePathName);
			dwCount++;
			pAllProcessInfo->uCount = dwCount;
			DbgPrint("ImageFileName = %s \n", (PUCHAR)pEprocess + ImageFileName_EPROCESS);
		}

		NextList = NextList->Flink;
	}
}


EXTERN_C VOID EnumProcessByThreadList( IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp, IN PALL_PROCESSES_INFO pAllProcessInfo )
{
	//_asm int 3
	//ScanXpHandleTable();
	//_asm int 3
	//EnumByCurrentProcessHandleTable();
	return;
	ULONG i;
	for ( i = 0; i < 32 ;i++ )
	{
		DisplayList(KiDispatcherReadyListHead + i, pAllProcessInfo);
	}
	DisplayList(KiWaitListhead, pAllProcessInfo);
}

#pragma PAGEDCODE
EXTERN_C VOID EnumProcessByActiveProcess( IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp, IN PALL_PROCESSES_INFO pAllProcessInfo )
{
	PEPROCESS		pCurEProcess, pStartEProcess;
	ULONG           dwCount = 0;
	PLIST_ENTRY pListActiveProcess;

	ULONG    dwPIdOffset = GetPlantformDependentInfo(PROCESS_ID_OFFSET);
	ULONG	 dwFatherPidOffset = GetPlantformDependentInfo(PROCESS_FATHER_ID_OFFSET);
	ULONG    dwPNameOffset = GetPlantformDependentInfo(FILE_NAME_OFFSET);
	ULONG    dwPLinkOffset = GetPlantformDependentInfo(PROCESS_LINK_OFFSET);


	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
	ULONG ulReadCurLength = 0;
	ULONG ulReadMaxLength = IrpStack->Parameters.Read.Length;
	PUCHAR pBuffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;

	//pCurPInfo = pAllProcessInfo;
	//FindByActiveProcessLinks();

	// 获取当前进程的地址
	pCurEProcess = PsGetCurrentProcess();
	pStartEProcess = pCurEProcess;
	do
	{
		//if (IsExitProcess(pCurEProcess))
		if (MmIsAddressValid((PVOID)pCurEProcess))
		{
			pAllProcessInfo->vProcessInf[dwCount].bHide = FALSE;
			pAllProcessInfo->vProcessInf[dwCount].FatherProcessId = *((ULONG32 *)((ULONG)pCurEProcess + dwFatherPidOffset));
			pAllProcessInfo->vProcessInf[dwCount].pEProcess = pCurEProcess;
			pAllProcessInfo->vProcessInf[dwCount].ProcessId = *((ULONG32 *)((ULONG)pCurEProcess + dwPIdOffset));
			if ((INT) pAllProcessInfo->vProcessInf[dwCount].ProcessId < 0)
			{
				pAllProcessInfo->vProcessInf[dwCount].ProcessId = 0;
			}
			switch (pAllProcessInfo->vProcessInf[dwCount].ProcessId)
			{
			case 0:
				RtlCopyMemory(pAllProcessInfo->vProcessInf[dwCount].ImageFileName, "System Idle Process", sizeof("System Idle Process"));
				break;
			case 4:
				RtlCopyMemory(pAllProcessInfo->vProcessInf[dwCount].ImageFileName, "System", sizeof("System"));
				break;
			default:
				RtlCopyMemory(pAllProcessInfo->vProcessInf[dwCount].ImageFileName, (PVOID)((ULONG)pCurEProcess + dwPNameOffset), 64);
				break;
			}
			GetProcessImagePath(pAllProcessInfo->vProcessInf[dwCount].ProcessId, pAllProcessInfo->vProcessInf[dwCount].ImagePathName);
			dwCount++;

			pListActiveProcess = (PLIST_ENTRY)((ULONG)pCurEProcess + dwPLinkOffset);
			pCurEProcess = (PEPROCESS)((ULONG)pListActiveProcess->Flink - dwPLinkOffset);
		}
	}while (pCurEProcess != pStartEProcess);

	pAllProcessInfo->uCount = dwCount;
	KdPrint(("ProcessCount = %d", dwCount));
}


#pragma PAGEDCODE
ULONG
GetPlantformDependentInfo(
						  ULONG dwFlag
						  )   
{    
	ULONG current_build;    
	ULONG ans = 0;    

	PsGetVersion(NULL, NULL, &current_build, NULL);    

	switch ( dwFlag )   
	{    
	case EPROCESS_SIZE:    
		if (current_build == 2195) ans = 0 ;        // 2000，当前不支持2000，下同   
		if (current_build == 2600) ans = 0x25C;     // xp   
		if (current_build == 3790) ans = 0x270;     // 2003   
		break;    
	case PEB_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x1b0;    
		if (current_build == 3790)  ans = 0x1a0;   
		break;    
	case FILE_NAME_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x174;    
		if (current_build == 3790)  ans = 0x164;   
		break;    
	case PROCESS_LINK_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x088;    
		if (current_build == 3790)  ans = 0x098;   
		break;    
	case PROCESS_ID_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x084;    
		if (current_build == 3790)  ans = 0x094;   
		break;    
	case PROCESS_FATHER_ID_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x14c;    
		if (current_build == 3790)  ans = 0;   
		break;
	case EXIT_TIME_OFFSET:    
		if (current_build == 2195)  ans = 0;    
		if (current_build == 2600)  ans = 0x078;    
		if (current_build == 3790)  ans = 0x088;   
		break;    
	}    
	return ans;    
}



#pragma PAGEDCODE
/************************************
* 函数名称:    GetProcessImagePath
* 功能描述: 获取程序的全路径
* 参数列表: IN DWORD dwProcessId
* 参数列表: OUT PUNICODE_STRING ProcessImagePath
* 返回 值:   NTSTATUS
/************************************/
NTSTATUS GetProcessImagePath( IN ULONG32 dwPId, OUT PWCHAR ProcessImagePath )
{    
	NTSTATUS Status;    
	HANDLE hProcess;    
	PEPROCESS pEprocess;    
	ULONG returnedLength;    
	ULONG bufferLength;    
	PVOID buffer;
	PWCH pPos;    
	UNICODE_STRING DeviceName;
	PUNICODE_STRING imageName;
	ULONG uCount;
	PDEVICE_OBJECT DeviceObject;
	PFILE_OBJECT FileObject;
	PZwQueryInformationProcess ZwQueryInformationProcess = NULL;
	DeviceName.Buffer = NULL;

	switch (dwPId)
	{
	case 0:
		RtlCopyMemory(ProcessImagePath, L"System Idle Process", sizeof(L"System Idle Process"));
		return STATUS_SUCCESS;
	case 4:
		RtlCopyMemory(ProcessImagePath, L"System", sizeof(L"System"));
		return STATUS_SUCCESS;
	default:
		break;
	}

	__try
	{
		UNICODE_STRING routineName;    

		RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");    

		ZwQueryInformationProcess =    
			(PZwQueryInformationProcess) GetUndocumentFuncAddrByName(&routineName);    

		Status = PsLookupProcessByProcessId((HANDLE)dwPId, &pEprocess); 

		Status = ObOpenObjectByPointer(pEprocess,          // Object    
		OBJ_KERNEL_HANDLE,  // HandleAttributes    
		NULL,               // PassedAccessState OPTIONAL    
		GENERIC_READ,       // DesiredAccess    
		*PsProcessType,     // ObjectType    
		KernelMode,         // AccessMode    
			&hProcess);

		//    
		// Step one - get the size we need    
		//    
		Status = ZwQueryInformationProcess( hProcess,    
			ProcessImageFileName,    
			NULL, // buffer    
			0, // buffer size    
			&returnedLength);

		//    
		// If we get here, the buffer IS going to be big enough for us, so    
		// let's allocate some storage.    
		//    
		buffer = ExAllocatePoolWithTag(PagedPool, returnedLength, 'ipgD');
		RtlZeroMemory(buffer, returnedLength);

		//    
		// Now lets go get the data    
		//    
		Status = ZwQueryInformationProcess( hProcess,    
			ProcessImageFileName,    
			buffer,    
			returnedLength,    
			&returnedLength);

		//    
		// Ah, we got what we needed    
		//    
		imageName = (PUNICODE_STRING) buffer;

		pPos = imageName->Buffer;
		uCount = 0;

		while (uCount < 3)
		{
			if (*pPos == '\\')
			{
				++uCount;
			}
			pPos++;
		}

		DeviceName.Length = (pPos - imageName->Buffer - 1) * sizeof(WCHAR);
		DeviceName.MaximumLength = DeviceName.Length;
		DeviceName.Buffer = (PWCH)ExAllocatePool(PagedPool, DeviceName.Length);

		RtlCopyMemory(DeviceName.Buffer, imageName->Buffer, DeviceName.Length);

		//得到设备对象句柄，计数器加1
		//如果是第一次调用IoGetDeviceObjectPointer，会打开设备，相当于调用ZwCreateFile
		Status = IoGetDeviceObjectPointer(&DeviceName, SYNCHRONIZE, &FileObject, &DeviceObject);
		if (NT_SUCCESS(Status))
		{
			//_asm int 3
			IoVolumeDeviceToDosName(FileObject->DeviceObject, &DeviceName);
			RtlCopyMemory((PUCHAR)ProcessImagePath, DeviceName.Buffer, DeviceName.Length);
			RtlCopyMemory((PUCHAR)ProcessImagePath + DeviceName.Length, --pPos, imageName->Length - (pPos - imageName->Buffer));

			//KdPrint(("image path is %wZ\n", ProcessImagePath));
			ObDereferenceObject(FileObject);
		}
		else
		{
			RtlCopyMemory(ProcessImagePath, ((PUNICODE_STRING) buffer)->Buffer, ((PUNICODE_STRING) buffer)->Length);
		}


	}
	__finally
	{
		if (DeviceName.Buffer != NULL)
		{
			ExFreePool(DeviceName.Buffer);
			DeviceName.Buffer = NULL;
		}

		ZwClose(hProcess);    

		//    
		// free our buffer    
		//    
		ExFreePool(buffer); 

	}
   

	//    
	// And tell the caller what happened.    
	//       
	return Status;    

}




NTSTATUS TerminateProcess( PEPROCESS Process )
{
	NTSTATUS          Status;
	PETHREAD          Thread;
	PSPTERMINATETHREAD PspTerminateThread;
	UNICODE_STRING usFuncName;

	Status = STATUS_SUCCESS;
	__try
	{
		PspTerminateThread = (PSPTERMINATETHREAD)GetPspTerminateThread();

		for (Thread = GetNextProcessThread( Process, NULL );
			Thread != NULL;
			Thread = GetNextProcessThread( Process, Thread ))
		{
			if (!PsIsThreadTerminating(Thread))
			{
				Status = (*PspTerminateThread)( Thread, 0);
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		Status = GetExceptionCode();
	}
	return Status;
}


//载自ReactOS-0.3.4-REL-src
PETHREAD
NTAPI
GetNextProcessThread(IN PEPROCESS Process,
					 IN PETHREAD Thread OPTIONAL)
{
	PETHREAD FoundThread = NULL;
	PLIST_ENTRY ListHead, Entry;
	PAGED_CODE();

	if (Thread)
	{
		//  Entry = Thread->ThreadListEntry.Flink;;//   +0x22c ThreadListEntry  : _LIST_ENTRY
		Entry = (PLIST_ENTRY)((ULONG)(Thread)+0x22c);
		Entry=Entry->Flink;
	}
	else
	{
		Entry = (PLIST_ENTRY)((ULONG)(Process)+0x190);//+0x190 ThreadListHead   : _LIST_ENTRY
		Entry = Entry->Flink; 
	}
	// ListHead = &Process->ThreadListHead;
	ListHead = (PLIST_ENTRY)((ULONG)Process + 0x190);
	while (ListHead != Entry)
	{
		//   FoundThread = CONTAINING_RECORD(Entry, ETHREAD, ThreadListEntry);
		FoundThread = (PETHREAD)((ULONG)Entry - 0x22c);
		//    if (ObReferenceObjectSafe(FoundThread)) break;
		if (ObReferenceObject(FoundThread)) break;
		FoundThread = NULL;
		Entry = Entry->Flink;
	}
	if (Thread) ObDereferenceObject(Thread);
	return FoundThread;
}


PVOID GetPspTerminateThread()
{
	ULONG size,index;
	PULONG buf;
	ULONG i;
	PSYSTEM_MODULE_INFORMATION module;
	PVOID driverAddress=0;
	ULONG ntosknlBase;
	ULONG ntosknlEndAddr;
	ULONG curAddr;
	NTSTATUS status;
	PVOID retAddr;
	ULONG code1_sp2=0x8b55ff8b,code2_sp2=0x0cec83ec,code3_sp2=0xfff84d83,code4_sp2=0x7d8b5756;

	ZwQuerySystemInformation(SystemModuleInformation,&size, 0, &size);
	if(NULL==(buf = (PULONG)ExAllocatePool(PagedPool, size)))
	{
		DbgPrint("failed alloc memory failed \n");
		return 0;
	}
	status=ZwQuerySystemInformation(SystemModuleInformation,buf, size , 0);
	if(!NT_SUCCESS( status ))
	{
		DbgPrint("failed query\n");
		return 0;
	}
	module = (PSYSTEM_MODULE_INFORMATION)(( PULONG )buf + 1);
	ntosknlEndAddr=(ULONG)module->Base+(ULONG)module->Size;
	ntosknlBase=(ULONG)module->Base;
	curAddr=ntosknlBase;
	ExFreePool(buf);
	for (i=curAddr;i<=ntosknlEndAddr;i++)
	{
		if ((*((ULONG *)i)==code1_sp2)&&(*((ULONG *)(i+4))==code2_sp2)&&(*((ULONG *)(i+8))==code3_sp2)&&(*((ULONG*)(i+12))==code4_sp2)) 
		{
			retAddr=(PVOID*)i;
			DbgPrint("MyPspTerminateThreadByPointer  adress is:%x\n",retAddr); 
			return retAddr;
		}
	}
	DbgPrint("Can't Find MyPspTerminateThreadByPointer  Address:%x\n"); 
	return 0;
}

NTSTATUS IsHideProcess( PEPROCESS pEprocess )
{
	ULONG pCurrentEprocess;
	PLIST_ENTRY pListActiveProcess;
	ULONG    dwPLinkOffset = GetPlantformDependentInfo(PROCESS_LINK_OFFSET);

	pCurrentEprocess = (ULONG)g_pSystemProcess;
	while(pCurrentEprocess != 0)
	{
		if ((ULONG)pEprocess == pCurrentEprocess)
		{
			//正常进程
			return STATUS_UNSUCCESSFUL;
		}
		pListActiveProcess = (PLIST_ENTRY)((ULONG)pCurrentEprocess + dwPLinkOffset);
		pCurrentEprocess = (ULONG)pListActiveProcess->Flink - dwPLinkOffset;
		//(ULONG)pCurrentEprocess = (ULONG)pListActiveProcess->Flink - dwPLinkOffset;
		if (pCurrentEprocess == (ULONG)g_pSystemProcess)
		{
			break;
		}
	}

	return STATUS_SUCCESS;
}


NTSTATUS IsExitProcess( PEPROCESS pEprocess )
{
	//WIN_VER_DETAIL WinVer;
	//ULONG SectionObjectOffset = NULL;
	//ULONG SectionObject;
	//ULONG SegmentOffset = NULL;
	//ULONG Segment;
	//BOOL bRetOK = FALSE;

	//if (!ARGUMENT_PRESENT(Eprocess) ||
	//	!Eprocess)
	//{
	//	return bRetOK;
	//}
	////排除system进程
	//if (Eprocess == SystemEProcess){
	//	return TRUE;
	//}
	//__try
	//{

	//	if (!WinVersion)
	//		WinVer = GetWindowsVersion();
	//	else
	//		WinVer = WinVersion;

	//	switch (WinVer)
	//	{
	//	case WINDOWS_VERSION_XP:
	//		SectionObjectOffset = 0x138;
	//		SegmentOffset=0x14;
	//		break;
	//	case WINDOWS_VERSION_2K3_SP1_SP2:
	//		SectionObjectOffset = 0x124;
	//		SegmentOffset=0x14;
	//		break;
	//	case WINDOWS_VERSION_7_7600_UP:
	//		SectionObjectOffset = 0x128;
	//		SegmentOffset=0x14;
	//		break;
	//	case WINDOWS_VERSION_7_7000:
	//		SectionObjectOffset = 0x128;
	//		SegmentOffset=0x14;
	//		break;
	//	}
	//	if (SegmentOffset &&
	//		SectionObjectOffset)
	//	{
	//		if (MmIsAddressValidEx(((ULONG)Eprocess + SectionObjectOffset)) ){
	//			SectionObject = *(PULONG)((ULONG)Eprocess + SectionObjectOffset);

	//			if (MmIsAddressValidEx(((ULONG)SectionObject + SegmentOffset))){
	//				Segment = *(PULONG)((ULONG)SectionObject + SegmentOffset);

	//				if (MmIsAddressValidEx(Segment)){
	//					bRetOK = TRUE;  //进程是有效的
	//					__leave;
	//				}
	//			}
	//		}
	//	}
	//}
	//__except(EXCEPTION_EXECUTE_HANDLER){
	//	KdPrint(("%08x\r\n",GetExceptionCode()));
	//}
	//return bRetOK;
	return FALSE;
}








//通过ActiveProcessLinks遍历进程
VOID 
FindByActiveProcessLinks()
{
	ULONG eproc=0x0;
	int current_PID=0;
	int start_PID=0;
	int count=0;
	PLIST_ENTRY plist_active_procs;
	DbgPrint("FindByActiveProcessLinks:\n");
	eproc=(ULONG)PsGetCurrentProcess();
	start_PID=*((int *)(eproc+UniqueProcessId_EPROCESS));
	current_PID=start_PID;
	while(1)
	{
		if((count>=1)&&(start_PID==current_PID))
		{
			return;
		}
		else
		{
			plist_active_procs=(PLIST_ENTRY)(eproc+ActiveProcessLinks_EPROCESS);
			eproc=(ULONG)plist_active_procs->Flink;
			eproc=eproc-ActiveProcessLinks_EPROCESS;
			current_PID=*((int *)(eproc+UniqueProcessId_EPROCESS));
			if(current_PID<0)
				current_PID=0;
			DbgPrint("process id %4d,address %8x\n",current_PID,eproc);
			count++;
		}
	}
	DbgPrint("Total number is %d\n\n",count);
}

//通过扫描当前进程的handletable获得进程表
VOID EnumByCurrentProcessHandleTable()
{
	PEPROCESS pEprocess;
	PLIST_ENTRY start_list,HandleTableList=NULL;
	ULONG addr;
	int count=0;
	int pid;
	DbgPrint("FindByObjectTable:\n");
	pEprocess = (PEPROCESS)PsGetCurrentProcess();
	HandleTableList = (PLIST_ENTRY)(*(PULONG)((ULONG)pEprocess + ObjectTable_EPROCESS) + HandleTableList_ObjectTable);
	start_list = HandleTableList;
	do {
		pid = *(PULONG)((ULONG)HandleTableList + UniqueProcessId_ObjectTable - HandleTableList_ObjectTable);
		if(pid != 4)
			addr = *(PULONG)((ULONG)HandleTableList + QuotaProcess_ObjectTable - HandleTableList_ObjectTable);
		else 
			addr = 0xffffffff;
		DbgPrint("process id %4d,address %8x\n",pid,addr);
		count++;
		if(pid != 4)
			addr = *(PULONG)((ULONG)HandleTableList + UniqueProcessId_ObjectTable - HandleTableList_ObjectTable);
		HandleTableList = HandleTableList->Flink;
	} while(start_list != HandleTableList);
	DbgPrint("Total number is %d\n",count);
}


PVOID GetPspCidTableAddress()
{
	PVOID PspCidTable = NULL;
	UNICODE_STRING usFuncName;
	RtlInitUnicodeString(&usFuncName, L"PsLookupProcessByProcessId");
	PUCHAR ptmp = (PUCHAR)GetUndocumentFuncAddrByName(&usFuncName);

	for (int c = 0; c < 256; c++)
	{
		if ((ptmp[c] == 0xff) && (ptmp[c + 1] == 0x35) && (ptmp[c + 6] == 0xe8))
		{
			ptmp += c + 2;
			PspCidTable = **(PVOID **)ptmp;
			break;
		}
	}
	return PspCidTable;
}

VOID RecordProcessInf( IN PVOID pInfo, IN PALL_PROCESSES_INFO pAllProcessInfo )
{
	DWORD dwCount = pAllProcessInfo->uCount;
	PEPROCESS pEprocess = (PEPROCESS)pInfo;
	if (IsProcess(pInfo))
	{
		if (!ProcessIsFind(*(PULONG)((PUCHAR)pEprocess + UniqueProcessId_EPROCESS), pAllProcessInfo))
		{
			pAllProcessInfo->vProcessInf[dwCount].FatherProcessId = *(PULONG)((PUCHAR)pEprocess + InheritedFromUniqueProcessId_EPROCESS);
			pAllProcessInfo->vProcessInf[dwCount].pEProcess = pEprocess;
			pAllProcessInfo->vProcessInf[dwCount].ProcessId = *(PULONG)((PUCHAR)pEprocess + UniqueProcessId_EPROCESS);
			if ((INT)pAllProcessInfo->vProcessInf[dwCount].ProcessId < 0)
			{
				pAllProcessInfo->vProcessInf[dwCount].ProcessId = 0;
			}
			pAllProcessInfo->vProcessInf[dwCount].bHide = NT_SUCCESS(IsHideProcess(pEprocess)) ? TRUE : FALSE;
			RtlCopyMemory(pAllProcessInfo->vProcessInf[dwCount].ImageFileName, (PUCHAR)pEprocess + ImageFileName_EPROCESS, MAX_PATH);
			GetProcessImagePath(pAllProcessInfo->vProcessInf[dwCount].ProcessId, pAllProcessInfo->vProcessInf[dwCount].ImagePathName);
			dwCount++;
			pAllProcessInfo->uCount = dwCount;
			DbgPrint("ImageFileName = %s \n", (PUCHAR)pEprocess + ImageFileName_EPROCESS);
		}

		//KdPrint(("Object is 0x%08x, filename is %s\n", pObject, pEprocess->ImageFileName));
	}

}


EXTERN_C NTSTATUS ScanXpHandleTable( IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp, IN PALL_PROCESSES_INFO pAllProcessInfo )
{
	PHANDLE_TABLE pPspCidTable;
	PHANDLE_TABLE_ENTRY pHandleTableEntry;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG LevelIndex;
	PULONG TableCode, pObject;
	PEPROCESS pEprocess = NULL;
	ULONG i, j, k, dwCount = 0;

	ULONG top_level_count = 32; 
	ULONG mid_level_count = 1024; 
	ULONG sub_level_count = PAGE_SIZE / sizeof (HANDLE_TABLE_ENTRY); 

	pPspCidTable = (PHANDLE_TABLE)GetPspCidTableAddress();
	if (NULL == pPspCidTable)
	{
		KdPrint(("GetPspCidTableAddress fail\n"));
		return status;
	}
	LevelIndex = (ULONG)*(PULONG)pPspCidTable & TABLE_LEVEL_MASK;
	TableCode = (PULONG)((pPspCidTable->TableCode | 0x80000000) & 0xfffffff8);

	//sub_level_count = pPspCidTable->NextHandleNeedingPool > pPspCidTable->HandleCount ? pPspCidTable->HandleCount : pPspCidTable->NextHandleNeedingPool;

	switch (LevelIndex)
	{
	case 0: //table de niveau 0, TableCode pointe vers une entrée de la handle table.
		{
			DbgPrint("Level 0.");
			pHandleTableEntry = (PHANDLE_TABLE_ENTRY)TableCode;
			pHandleTableEntry++;
			for (i = 0; i < sub_level_count; i++)
			{
				if (pHandleTableEntry->Object)
				{
					pObject = (PULONG)(( (ULONG)pHandleTableEntry->Object | 0x80000000)& 0xfffffff8);

					POBJECT_HEADER ObjectHeader = OBJECT_TO_OBJECT_HEADER(pObject);
					KdPrint(("Object is 0x%08x\n", pObject));


					if (ObjectHeader && \
						MmIsAddressValid(&ObjectHeader->Type) &&\
						ObjectHeader->Type == *PsProcessType)
					{
						RecordProcessInf(pObject, pAllProcessInfo);
					}

				}
				pHandleTableEntry++;
			}
			KdPrint(("Object is 0x%08x\n", pHandleTableEntry));

			break;
		}

	case 1: //table de niveau 1, TableCode pointe vers un tableau de pointeur sur les entrées.
		{
			DbgPrint("Level1");
			for( i = 0 ; i < top_level_count ; i++ )
			{
				if(*TableCode)
				{
					pHandleTableEntry = (PHANDLE_TABLE_ENTRY)*TableCode;
					for( j = 0 ; j < sub_level_count ; j++ )
					{
						if (pHandleTableEntry->Object)
						{
							pObject = (PULONG)(( (ULONG)pHandleTableEntry->Object | 0x80000000)& 0xfffffff8);

							POBJECT_HEADER ObjectHeader = OBJECT_TO_OBJECT_HEADER(pObject);
							KdPrint(("Object is 0x%08x\n", pObject));


							if (ObjectHeader && \
								MmIsAddressValid(&ObjectHeader->Type) &&\
								ObjectHeader->Type == *PsProcessType)
							{
								RecordProcessInf(pObject, pAllProcessInfo);
							}

						}
						pHandleTableEntry++;
					}
				}
				TableCode++;
			}
			break;
		}

	case 2://Table de niveau 2, Table code faire vers tableau de tableau de pointeur sur les entrées.
		DbgPrint("Level 2");
		for( i = 0 ; i < top_level_count ; i++ )
		{
			if(*TableCode)
			{
				for( j = 0 ; j < mid_level_count ; j++ )
				{
					if( *(PULONG*)(*TableCode) )
					{
						pHandleTableEntry = (PHANDLE_TABLE_ENTRY)*((PULONG)*TableCode);
						for( k = 0 ; k < sub_level_count ; k++ )
						{
							if (pHandleTableEntry->Object)
							{
								pObject = (PULONG)(( (ULONG)pHandleTableEntry->Object | 0x80000000)& 0xfffffff8);

								POBJECT_HEADER ObjectHeader = OBJECT_TO_OBJECT_HEADER(pObject);
								KdPrint(("Object is 0x%08x\n", pObject));


								if (ObjectHeader && \
									MmIsAddressValid(&ObjectHeader->Type) &&\
									ObjectHeader->Type == *PsProcessType)
								{
									RecordProcessInf(pObject, pAllProcessInfo);
								}

							}
							pHandleTableEntry++;
						}
					}
					(*TableCode)++;
				}
			}
			(TableCode)++;
		}
		break;
	}
	return status;
}

// XP: nt!PsTerminateSystemThread -> GetPspTerminateThreadByPointer
// 2k: NtTerminateThread -> PspTerminateThreadByPointer
//只处理XP
PCHAR GetPspTerminateThreadByPointer()
{
	char * PsTerminateSystemThreadAddr;
	int iLen;
	DWORD dwAddr;
	DWORD NtTerminateThreadAddr;
	char * pAddr = NULL;

	PsTerminateSystemThreadAddr= (char *)PsTerminateSystemThread;
	__asm
	{
		__emit 0x90;
		__emit 0x90;
	}
	for (iLen=0;iLen<50;iLen++)
	{
		if (*PsTerminateSystemThreadAddr == (char)0xff
			&& *(PsTerminateSystemThreadAddr+1) == (char)0x75
			&& *(PsTerminateSystemThreadAddr+2) == (char)0x08
			)
		{
			__asm
			{
				__emit 0x90;
				__emit 0x90;
			}
			PsTerminateSystemThreadAddr += 5;
			dwAddr = *(DWORD *)PsTerminateSystemThreadAddr + (DWORD)PsTerminateSystemThreadAddr +4;

			//DbgPrint("PspTerminateThreadByPointer:: 0x%x ",dwAddr);
			return (PCHAR)dwAddr;
			//break;
		}
		PsTerminateSystemThreadAddr++;
	}
	return (PCHAR)dwAddr;
}