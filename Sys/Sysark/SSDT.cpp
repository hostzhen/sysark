#include "SSDT.h"

//全局变量
PSSDT_FUNC_INFO g_pSSDTFuncInfo = NULL;
PPAGED_LOOKASIDE_LIST g_pPageListSSDT = NULL;
PULONG g_pOriginalSSDT = NULL; //保存原始SSDT RVA


EXTERN_C ULONG GetSSDTCount()
{
	return KeServiceDescriptorTable->NumberOfServices;

}


EXTERN_C VOID SetSSDT(PVOID pInfo)
{
	ASSERT(NULL != pInfo);

	PSSDT_INFO pSSDTInfo = (PSSDT_INFO)pInfo;

	*(ULONG*)((ULONG)KeServiceDescriptorTable + pSSDTInfo->ulIndex) = pSSDTInfo->ulAddress;

}

EXTERN_C NTSTATUS EnumSSDT(PVOID pOuputBuufer)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSSDT_FUNC_INFO pSSDTFuncInfo = NULL, pPreSSDTFuncInfo = NULL;
	ULONG ulSSDTCount, ulIndex;
	HANDLE hNtSection;
	UNICODE_STRING wsNtDllString;
	ULONG ulNtDllModuleBase;
	CHAR lpszFunction[MAX_PATH];

	__try
	{
		FreePagedLookasideListForSSDT();
		g_pPageListSSDT = (PPAGED_LOOKASIDE_LIST)ExAllocatePool(PagedPool, sizeof(PAGED_LOOKASIDE_LIST));
		ExInitializePagedLookasideList(g_pPageListSSDT, NULL, NULL, 0, sizeof(SSDT_FUNC_INFO), NULL, 0);
		ulSSDTCount = GetSSDTCount();
		g_pOriginalSSDT = (PULONG)ExAllocatePool(PagedPool, ulSSDTCount * sizeof(ULONG));

		GetOriginalSSDTAddress(ulSSDTCount);


		RtlInitUnicodeString(&wsNtDllString,L"\\SystemRoot\\System32\\ntdll.dll");
		hNtSection = MapFileAsSection(&wsNtDllString, (PVOID*)(&ulNtDllModuleBase));  //载入到内存，后面为了获取函数的名称
		if (!hNtSection)
		{
			//if (DebugOn)
				KdPrint(("MapFileAsSection failed"));

			return FALSE;
		}
		ZwClose(hNtSection);

		for (ulIndex = 0; ulIndex < ulSSDTCount; ulIndex++)
		{
			pSSDTFuncInfo = (PSSDT_FUNC_INFO)ExAllocateFromPagedLookasideList(g_pPageListSSDT);
			RtlZeroMemory(pSSDTFuncInfo, sizeof(SSDT_FUNC_INFO));

			pSSDTFuncInfo->ulIndex = ulIndex;
			pSSDTFuncInfo->ulCurrenAddress = *(PULONG)(KeServiceDescriptorTable->ServiceTableBase + ulIndex);
			pSSDTFuncInfo->ulOriginalAddress = g_pOriginalSSDT[ulIndex];
			//获取ulCurrenAddress所在模块的路径
			GetModuleName(pSSDTFuncInfo->lpModulePath, pSSDTFuncInfo->ulCurrenAddress);

			//获取函数的名称
			RtlZeroMemory(lpszFunction, sizeof(lpszFunction));
			GetFunctionNameByIndex(ulNtDllModuleBase, &ulIndex, lpszFunction);
			RtlCopyMemory(pSSDTFuncInfo->lpwzFuncName, lpszFunction, MAX_PATH);

			if (g_pSSDTFuncInfo == NULL)
			{
				g_pSSDTFuncInfo = pSSDTFuncInfo;
				pPreSSDTFuncInfo = pSSDTFuncInfo;
			}
			else
			{
				pPreSSDTFuncInfo->next = pSSDTFuncInfo;
				pPreSSDTFuncInfo = pSSDTFuncInfo;
			}
		}
		GetSSDTFuncInfo(pOuputBuufer);
		FreePagedLookasideListForSSDT();
		status = STATUS_SUCCESS;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		status = STATUS_UNSUCCESSFUL;

	}

	return status;
}

NTSTATUS FreePagedLookasideListForSSDT()
{
	NTSTATUS status = STATUS_SUCCESS;
	PSSDT_FUNC_INFO pSSDTFuncInfo = g_pSSDTFuncInfo, pNextSSDTFuncInfo = NULL;

	__try
	{
		if (NULL != g_pPageListSSDT && NULL != g_pSSDTFuncInfo)
		{
			while (pSSDTFuncInfo)
			{
				pNextSSDTFuncInfo = pSSDTFuncInfo->next;

				ExFreeToPagedLookasideList(g_pPageListSSDT, pSSDTFuncInfo);

				pSSDTFuncInfo = pNextSSDTFuncInfo;
			}

			ExDeletePagedLookasideList(g_pPageListSSDT);
			ExFreePool(g_pPageListSSDT);
			g_pPageListSSDT = NULL;
			g_pSSDTFuncInfo = NULL;
		}
		if (NULL != g_pOriginalSSDT)
		{
			ExFreePool(g_pOriginalSSDT);
			g_pOriginalSSDT = NULL;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("SSDT:FreePagedLookasideListForSSDT failed!"));
		status = GetExceptionCode();
	}


	return status;
}

NTSTATUS GetSSDTFuncInfo(PVOID pInfo)
{
	ULONG ulLength = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSSDT_FUNC_INFO pSSDTInf = g_pSSDTFuncInfo;

	__try
	{
		while (pSSDTInf)
		{

			RtlCopyMemory((PUCHAR)pInfo + ulLength, pSSDTInf, sizeof(SSDT_FUNC_INFO));
			ulLength += sizeof(SSDT_FUNC_INFO);
			pSSDTInf = pSSDTInf->next;
		}
		status = STATUS_SUCCESS;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//_asm int 3
		KdPrint(("SSDT:GetSSDTFuncInfo failed"));
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}



ULONG
GetKiServiceTable(
					   IN PVOID Buffer,
					   IN ULONG Offset)
					   /*++

					   Routine Description:

					   get KiServiceTable's address with KeServiceDescriptorTable's
					   offset in file.

					   Arguments:

					   Buffer - pointer to buffer filled with nt file.
					   Offset - offset of KeServiceDescriptorTable.

					   Return Value:

					   the address of KiServiceTable.
					   --*/
{
	ULONG RetValue;
	ULONG pReloc;
	ULONG RelocSize;
	ULONG RelocBase;
	ULONG TableSize;
	ULONG Temp;
	ULONG Number;
	ULONG i;

	//
	//	get the offset of relocation table in file 
	//

	pReloc = (ULONG)((PCHAR)Buffer + \
		((PIMAGE_NT_HEADERS)((PCHAR)Buffer + ((PIMAGE_DOS_HEADER)Buffer)->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	RelocSize = (ULONG)((PIMAGE_NT_HEADERS)((PCHAR)Buffer + ((PIMAGE_DOS_HEADER)Buffer)->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

	//pReloc = (ULONG)RtlImageDirectoryEntryToData( Buffer,
	//	FALSE,
	//	IMAGE_DIRECTORY_ENTRY_BASERELOC,
	//	&RelocSize);

	if(pReloc == 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	//
	//	look over the relocation table to find KiServiceTable
	//

	for(;;)
	{
		RelocBase = *(PULONG)(pReloc);
		TableSize = *(PULONG)(pReloc + 0x04);

		if(TableSize == 0)
		{
			break;
		}

		Number = (TableSize - 0x08) >> 1;

		for( i = 0; i < Number; i++)
		{
			Temp = *(PUSHORT)(pReloc + 0x08 + i * 0x02) & 0x0FFF;
			Temp += RelocBase;//RVA
			Temp += (ULONG)Buffer;//VA

			//
			//	C7 05 XX XX XX XX YY YY  YY YY
			//	mov  ds:_KeServiceDescriptorTable, offset _KiServiceTable
			//

			if(*(PULONG)Temp == Offset)
			{
				if(*(PUSHORT)(Temp - 2) == 0x05C7)
				{
					RetValue = *(PULONG)(Temp + 4);
					return RetValue;
				}
			}
		}
		pReloc += TableSize;
	}

	return 0;
}


ULONG
GetExportData(
			  IN PVOID FileBase,
			  IN PCHAR DataName)
			  /*++

			  Routine Description:

			  get data offset from unmapped file.

			  Arguments:

			  FileBase - pointer to buffer filled with file.
			  DataName - name of data 

			  Return Value:

			  offset of data.

			  --*/
{
	ULONG nFunctions;
	ULONG nNames;
	ULONG Index;
	ULONG ExpSize;
	LONG RetValue = 0;
	ULONG nSerial;
	ULONG MappedExpBase;
	ULONG FuncName;
	PUSHORT AddressOfNameOrdinals;
	PULONG AddressOfNames;
	PULONG AddressOfFunctions;
	PIMAGE_EXPORT_DIRECTORY pExport;


	pExport = (PIMAGE_EXPORT_DIRECTORY)((PCHAR)FileBase + \
		((PIMAGE_NT_HEADERS)((PCHAR)FileBase + ((PIMAGE_DOS_HEADER)FileBase)->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	ExpSize = (ULONG)((PIMAGE_NT_HEADERS)((PCHAR)FileBase + ((PIMAGE_DOS_HEADER)FileBase)->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;


	if (pExport == NULL)
	{
		return 0;
	}


	//
	//	binary search
	//

	nNames = pExport->NumberOfNames;
	nFunctions = pExport->NumberOfFunctions;


	AddressOfNameOrdinals = (PUSHORT)((PCHAR)FileBase + pExport->AddressOfNameOrdinals);
	AddressOfFunctions = (PULONG)((PCHAR)FileBase + pExport->AddressOfFunctions);
	AddressOfNames = (PULONG)((PCHAR)FileBase + pExport->AddressOfNames);
	//_asm int 3

	for (Index = 0; Index < nNames; Index++)
	{
		nSerial = * (AddressOfNameOrdinals + Index);
		FuncName = (ULONG)((PCHAR)FileBase + *(AddressOfNames + Index));

		if(strcmp( (PCHAR)FuncName, DataName ) == 0)
		{
			if (nSerial <= nFunctions)
			{
				RetValue = *(AddressOfFunctions + nSerial - pExport->Base);
			}

			break;
		}
	}

	return RetValue;
}



NTSTATUS
GetNtos(
				 OUT PCHAR ModuleName,
				 OUT PULONG ModuleBase)
				 /*++

				 Routine Description:

				 get loaded module's path and base

				 Arguments:

				 Index - index of module. the 0x00 is nt
				 ModuleName - return module path
				 ModuleBase - return module base

				 Return Value:

				 NT Status code.

				 --*/
{
	NTSTATUS Status;
	PVOID Buffer;
	ULONG BufferSize;
	ULONG Count;
	PSYSTEM_MODULE_INFORMATION_ENTRY pModuleInfo;
	ULONG i;

	//
	//	the second argument is null, the function
	//	return the size of buffer
	//

	Status = ZwQuerySystemInformation( SystemModuleInformation,
		NULL,
		0,
		&BufferSize);

	if(!NT_SUCCESS(Status) && Status != STATUS_INFO_LENGTH_MISMATCH)
	{
		return Status;
	}

	Buffer = ExAllocatePool( NonPagedPool, BufferSize);

	if(Buffer == NULL)
	{
		return STATUS_UNSUCCESSFUL;
	}

	Status = ZwQuerySystemInformation( SystemModuleInformation,
		Buffer,
		BufferSize,
		NULL);

	if(!NT_SUCCESS(Status))
	{
		ExFreePool(Buffer);
		return Status;
	}

	Count = *(PULONG)Buffer;

	pModuleInfo = (PSYSTEM_MODULE_INFORMATION_ENTRY)((PCHAR)Buffer + 4);

	strcat(ModuleName, pModuleInfo->ImageName);
	*ModuleBase = (ULONG)pModuleInfo->Base;

	ExFreePool(Buffer);
	return STATUS_SUCCESS;
}


NTSTATUS
LoadFile(
		 IN PUNICODE_STRING ImagePath,
		 OUT PVOID * FileBuffer)
		 /*++

		 Routine Description:

		 this routine load file into memory.

		 Arguments:

		 FilePath - the path of file to be loaded.
		 FileBuffer - pointer to buffer of file.

		 Return Value:

		 NT Status code

		 Notice:

		 the buffer is allocated in this routine ,but should
		 be free in the other routine.

		 --*/
{
	NTSTATUS Status;
	OBJECT_ATTRIBUTES ObjectAttributes;
	IO_STATUS_BLOCK IoStatusBlock;
	HANDLE hFile;
	FILE_STANDARD_INFORMATION FileSize;
	PVOID Buffer;

	//
	//	create file
	//

	InitializeObjectAttributes(&ObjectAttributes,
		ImagePath,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL);

	Status = ZwCreateFile( &hFile,
		GENERIC_READ,
		&ObjectAttributes,
		&IoStatusBlock,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0);

	if(!NT_SUCCESS(Status))
	{
		return Status;
	}

	do
	{
		//
		//	get file size
		//

		Status = ZwQueryInformationFile( hFile,
			&IoStatusBlock,
			&FileSize,
			sizeof(FileSize),
			FileStandardInformation);

		if(!NT_SUCCESS(Status))
		{
			break;
		}

		//
		//	allocate buffer
		//

		Buffer = ExAllocatePoolWithTag(	NonPagedPool, 
			FileSize.AllocationSize.QuadPart, 
			'Dll ');

		if(Buffer == NULL)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		//
		//	read file
		//

		Status = ZwReadFile( hFile,
			NULL,
			NULL,
			NULL,
			&IoStatusBlock,
			Buffer,
			(LONG)FileSize.AllocationSize.QuadPart,
			NULL,
			NULL);

		//
		//	if faild, free the buffer
		//

		if(!NT_SUCCESS(Status))
		{
			ExFreePool(Buffer);
			Buffer = NULL;
		}

	}while(FALSE);

	if(hFile != NULL)
	{
		ZwClose(hFile);
		hFile = NULL;
	}

	*FileBuffer = Buffer;

	return Status;
}


/************************************
* 函数名称:  GetOriginalSSDTAddress
* 功能描述:  获取原始SSDT
			INIT:005B9886                 mov     ds:dword_47BEF0, esi
			INIT:005B988C                 mov     ds:_KeServiceDescriptorTable, offset _KiServiceTable
			INIT:005B9896                 mov     ds:dword_47BFA4, esi
			INIT:005B989C                 mov     ds:dword_47BFAC, offset _KiArgumentTable
* 参数列表:  ULONG ulSSDTCount
* 返回 值:   ULONG
/************************************/
ULONG GetOriginalSSDTAddress(ULONG ulSSDTCount)
{

	NTSTATUS Status;
	ULONG SSDTOffset;
	ULONG ImageBase;
	ULONG ulKiServAddr;
	ULONG ulNtosAddr;
	PVOID FileBuffer;
	CHAR pNtosPath[MAX_PATH] = "\\??\\c:";
	UNICODE_STRING usNtosPath;
	ANSI_STRING asNtosPath;


	GetNtos(pNtosPath, &ulNtosAddr);

	RtlInitAnsiString(&asNtosPath, pNtosPath);

	RtlAnsiStringToUnicodeString(&usNtosPath, &asNtosPath, TRUE);

	LoadFile(&usNtosPath, &FileBuffer);

	if (FileBuffer == NULL)
	{
		return 0;
	}

	//
	// get KeServiceDescriptorTable's offset from file
	//

	SSDTOffset = GetExportData( FileBuffer, "KeServiceDescriptorTable");

	if( SSDTOffset == 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	//
	//	get image base from file
	//

	ImageBase = (ULONG)(ULONG)((PIMAGE_NT_HEADERS)((PCHAR)FileBuffer + ((PIMAGE_DOS_HEADER)FileBuffer)->e_lfanew))->OptionalHeader.ImageBase;
	//ImageBase = GetImageBase( FileBuffer );

	if(ImageBase == 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	//
	//	get KiServiceTable's offset
	//

	ulKiServAddr = GetKiServiceTable( FileBuffer, ImageBase + SSDTOffset);

	if(ulKiServAddr == 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	//
	// get function address
	//

	RtlCopyMemory(g_pOriginalSSDT, (PVOID)(ulKiServAddr + (ULONG)FileBuffer - ImageBase), ulSSDTCount * 0x04);


	//
	//	relocate
	//

	for(int i = 0; i < ulSSDTCount; i++)
	{
		*(g_pOriginalSSDT + i) -= ImageBase;
		*(g_pOriginalSSDT + i) += ulNtosAddr;
	}

	RtlFreeUnicodeString(&usNtosPath);

	return 0;
}

