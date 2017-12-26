#ifdef __cplusplus
extern "C"
{
#endif
#include "ntifs.h"
#include "File.h"
#ifdef __cplusplus
}
#endif 


//全局变量
PDIRECTORY_INFO g_pDirectoryInfo = NULL;
PPAGED_LOOKASIDE_LIST g_pPageListDirectory = NULL;


#pragma PAGEDCODE
EXTERN_C
VOID DeleteFile(PUNICODE_STRING pFilePath)
{
	ASSERT(NULL != pFilePath);

	OBJECT_ATTRIBUTES ObjAttr;
	HANDLE hFile;
	IO_STATUS_BLOCK IoStatusBlock;
	InitializeObjectAttributes(&ObjAttr, pFilePath, OBJ_VALID_ATTRIBUTES, NULL, NULL);

	IoCreateFile(&hFile, GENERIC_ALL, &ObjAttr, &IoStatusBlock, NULL, NULL, NULL, NULL, FILE_DELETE_ON_CLOSE, NULL, NULL, CreateFileTypeNone, NULL, NULL);
}

NTSTATUS 
EventCompletion( 
				IN PDEVICE_OBJECT DeviceObject, 
				IN PIRP Irp, 
				IN PVOID Context 
				) 
{ 
	PIO_STATUS_BLOCK lpiosb; 
	lpiosb = Irp->UserIosb; 
	lpiosb->Status = Irp->IoStatus.Status; 
	lpiosb->Information = Irp->IoStatus.Information; 
	KeSetEvent(Irp->UserEvent,0,FALSE); 
	IoFreeIrp(Irp); 
	return STATUS_MORE_PROCESSING_REQUIRED; 
} 
PDEVICE_OBJECT
MyIoGetRelatedDeviceObject(
						   IN PFILE_OBJECT FileObject
						   )
{
	PDEVICE_OBJECT deviceObject;
	if (FileObject->Vpb != NULL && FileObject->Vpb->DeviceObject != NULL&&(!(FileObject->Flags & FO_DIRECT_DEVICE_OPEN))) {
		deviceObject = FileObject->Vpb->DeviceObject;
	} else if (!(FileObject->Flags & FO_DIRECT_DEVICE_OPEN) &&
		FileObject->DeviceObject->Vpb != NULL &&
		FileObject->DeviceObject->Vpb->DeviceObject != NULL) {
			deviceObject = FileObject->DeviceObject->Vpb->DeviceObject;
	} else {
		deviceObject = FileObject->DeviceObject;
	}
	if (deviceObject->AttachedDevice != NULL) {
		deviceObject = IoGetAttachedDevice( deviceObject );
	}
	return deviceObject;
}

ULONG GetDirectoryInfoCount()
{
	ULONG ulCount = 0;
	PDIRECTORY_INFO pDirectoryInfo = g_pDirectoryInfo;

	while (pDirectoryInfo)
	{
		++ulCount;
		pDirectoryInfo = pDirectoryInfo->next;
	}

	KdPrint(("File :GetDirectoryInfoCount is %d", ulCount));
	return ulCount;
}


NTSTATUS FreePagedLookasideListForDirectory()
{
	NTSTATUS status = STATUS_SUCCESS;
	PDIRECTORY_INFO pDirectoryInfo = g_pDirectoryInfo, pNextDirectoryInfo = NULL;

	__try
	{
		if (NULL != g_pPageListDirectory && NULL != g_pDirectoryInfo)
		{
			while (pDirectoryInfo)
			{
				pNextDirectoryInfo = pDirectoryInfo->next;

				ExFreeToPagedLookasideList(g_pPageListDirectory, pDirectoryInfo);

				pDirectoryInfo = pNextDirectoryInfo;
			}

			ExDeletePagedLookasideList(g_pPageListDirectory);
			ExFreePool(g_pPageListDirectory);
			g_pPageListDirectory = NULL;
			g_pDirectoryInfo = NULL;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("SSDT:FreePagedLookasideListForSSDT failed!"));
		status = GetExceptionCode();
	}


	return status;
}

NTSTATUS GetDirectoryInfo(PVOID pInfo)
{
	ULONG ulLength = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDIRECTORY_INFO pDirectoryInfo = g_pDirectoryInfo;

	__try
	{
		while (pDirectoryInfo)
		{

			RtlCopyMemory((PUCHAR)pInfo + ulLength, pDirectoryInfo, sizeof(DIRECTORY_INFO));
			ulLength += sizeof(DIRECTORY_INFO);
			pDirectoryInfo = pDirectoryInfo->next;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//_asm int 3
		KdPrint(("File :GetDirectoryInfo failed"));
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

EXTERN_C NTSTATUS EnumDirectory( char *lpDirName )
{
	NTSTATUS status, fStatus; 
	ULONG dwBytesReturned; 
	OBJECT_ATTRIBUTES objectAttributes; 
	PDEVICE_OBJECT lpDeviceObject;
	IO_STACK_LOCATION iost; 
	PIO_STACK_LOCATION lpsp; 
	IO_STATUS_BLOCK IoStatus; 
	HANDLE hFile = NULL;
	PFILE_DIRECTORY_INFORMATION lpInformation; 
	PDIRECTORY_INFO lpDirInfo = NULL, lpPreDirInfo = NULL; 
	PFILE_OBJECT lpFileObject = NULL;
	UNICODE_STRING unFileName;
	ANSI_STRING anFileName;
	HANDLE  eventHandle = NULL;
	CHAR buffer[1024]; 
	PUCHAR lpNext;

	dwBytesReturned = 0; 
	status = STATUS_UNSUCCESSFUL; 
	RtlZeroMemory(buffer,1024); 
	strcpy(buffer,"\\DosDevices\\"); 
	strcat(buffer,lpDirName); 
	RtlInitAnsiString(&anFileName,buffer); 
	RtlAnsiStringToUnicodeString(&unFileName,&anFileName,TRUE); 
	InitializeObjectAttributes(&objectAttributes,&unFileName,OBJ_CASE_INSENSITIVE + OBJ_KERNEL_HANDLE,NULL,NULL);

	__try
	{
		//打开文件
		fStatus = ZwOpenFile(&hFile,\
			FILE_LIST_DIRECTORY | SYNCHRONIZE | FILE_ANY_ACCESS,\
			&objectAttributes,\
			&IoStatus,\
			FILE_SHARE_READ | FILE_SHARE_WRITE| FILE_SHARE_DELETE, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

		ObReferenceObjectByHandle(hFile, FILE_LIST_DIRECTORY | SYNCHRONIZE, 0, KernelMode, (PVOID *)&lpFileObject, NULL); 

		status = ZwCreateEvent(&eventHandle, GENERIC_ALL, 0, NotificationEvent,
			FALSE);

		lpInformation = (PFILE_DIRECTORY_INFORMATION)ExAllocatePool(PagedPool, 655350); 
		status = ZwQueryDirectoryFile(hFile, eventHandle,0, 0, &IoStatus,
			lpInformation, 655350,
			FileDirectoryInformation, FALSE, NULL,
			FALSE );

		if (!NT_SUCCESS(status) && status != STATUS_PENDING)
		{
			goto LeaveBefore;
		}
		if (status == STATUS_PENDING)
		{
			KeWaitForSingleObject(eventHandle, Executive, KernelMode, TRUE, 0);
		}

		FreePagedLookasideListForDirectory();
		g_pPageListDirectory = (PPAGED_LOOKASIDE_LIST)ExAllocatePool(PagedPool, sizeof(PAGED_LOOKASIDE_LIST));
		ExInitializePagedLookasideList(g_pPageListDirectory, NULL, NULL, 0, sizeof(DIRECTORY_INFO), NULL, 0);

		while(1) 
		{
			lpDirInfo = (PDIRECTORY_INFO)ExAllocateFromPagedLookasideList(g_pPageListDirectory);
			RtlZeroMemory(lpDirInfo, sizeof(DIRECTORY_INFO));

			RtlCopyMemory(lpDirInfo->FileName, lpInformation->FileName, lpInformation->FileNameLength);
			lpDirInfo->AllocationSize = lpInformation->AllocationSize;
			lpDirInfo->FileAttributes = lpInformation->FileAttributes;
			RtlTimeToTimeFields(&(lpInformation->CreationTime), &(lpDirInfo->CreationTime)); 
			RtlTimeToTimeFields(&(lpInformation->LastAccessTime), &(lpDirInfo->LastAccessTime));  
			RtlTimeToTimeFields(&(lpInformation->LastWriteTime), &(lpDirInfo->LastWriteTime));  
			RtlTimeToTimeFields(&(lpInformation->ChangeTime), &(lpDirInfo->ChangeTime));
			lpDirInfo->next = NULL;

			if (NULL == g_pDirectoryInfo)
			{
				g_pDirectoryInfo = lpDirInfo;
				lpPreDirInfo = lpDirInfo;
			}
			else
			{
				lpPreDirInfo->next = lpDirInfo;
				lpPreDirInfo = lpDirInfo;
			}

			if(!lpInformation->NextEntryOffset)
			{
				break;
			}

			lpInformation = (PFILE_DIRECTORY_INFORMATION)((PUCHAR)lpInformation + lpInformation->NextEntryOffset);

		} 
LeaveBefore:
		;
	}
	__finally
	{
		if (NT_SUCCESS(fStatus))
		{
			ZwClose(hFile);
		}
		if (NULL != lpFileObject)
		{
			ObDereferenceObject(lpFileObject);
			lpFileObject = NULL;
		}
		if (NULL != eventHandle)
		{
			ZwClose(eventHandle);
			eventHandle = NULL;
		}

	}

	return status;               
}

NTSTATUS
FORCEINLINE
MyIoCallDriver(
			   IN PDEVICE_OBJECT DeviceObject,
			   IN OUT PIRP Irp
			   )
{
	PIO_STACK_LOCATION irpSp;
	PDRIVER_OBJECT driverObject;
	NTSTATUS status;
	ASSERT( Irp->Type == IO_TYPE_IRP );
	Irp->CurrentLocation--;
	if (Irp->CurrentLocation <= 0) {
		KeBugCheckEx( NO_MORE_IRP_STACK_LOCATIONS, (ULONG_PTR) Irp, 0, 0 ,0);
	}
	irpSp = IoGetNextIrpStackLocation( Irp );
	Irp->Tail.Overlay.CurrentStackLocation = irpSp;
	irpSp->DeviceObject = DeviceObject;
	driverObject = DeviceObject->DriverObject;
	status = driverObject->MajorFunction[irpSp->MajorFunction]( DeviceObject,
		Irp );
	return status;
}

EXTERN_C BOOLEAN EnumHandleCallBack( PHANDLE_TABLE_ENTRY Entry,HANDLE handle,ENUMHANDLE_PARAMETER* Param )
{
	KdPrint(("函数被调用! 句柄：%08x\n",handle));
	if(Param->AccessMask==0)
	{
		if(handle==Param->SorHadnle)
		{
			KdPrint(("获取源权限成功！"));
			Param->AccessMask=Entry->GrantedAccess;
			return TRUE;

		}
	}
	else
	{
		if(handle==Param->DesHandle)
		{
			KdPrint(("修改成功!\n"));
			Entry->GrantedAccess=Param->AccessMask;
			return TRUE;
		}
	}
	return FALSE;
}

NTSTATUS ModifyRight(FILE_RIGHT_MODIFY* Data)
{

	NTSTATUS  Status=STATUS_UNSUCCESSFUL;
	KdPrint(("输入句柄：Des %08x---Sour %08x",Data->DesHandle,Data->SourceHandle));
	ENUMHANDLE_PARAMETER * Param=(ENUMHANDLE_PARAMETER *)ExAllocatePool(NonPagedPool,sizeof(ENUMHANDLE_PARAMETER));
	if(Param>0)
	{
		Param->AccessMask=0;
		Param->DesHandle=Data->DesHandle;
		Param->SorHadnle=Data->SourceHandle;
		KdPrint(("%08x\n",PsGetCurrentProcess()));	
		PVOID TableAddr=(PVOID)GetHandleTableFromProcessXp(PsGetCurrentProcess());
		if(ExEnumHandleTable(TableAddr,
			(EX_ENUMERATE_HANDLE_ROUTINE)EnumHandleCallBack,Param,NULL))
		{
			KdPrint(("Enum Success!\n"));
			if(ExEnumHandleTable((PVOID)GetHandleTableFromProcessXp(PsGetCurrentProcess()),
				(EX_ENUMERATE_HANDLE_ROUTINE)EnumHandleCallBack,Param,NULL))
			{
				Status=STATUS_SUCCESS;
			}

		}
		ExFreePool(Param);
	}

	return Status;
}