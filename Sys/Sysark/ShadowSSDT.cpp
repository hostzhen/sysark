#include "ShadowSSDT.h"

PServiceDescriptorTable g_KeServiceDescriptorTableShadow;
//全局变量
PSSDT_FUNC_INFO g_pShadowSSDTFuncInfo = NULL;
PPAGED_LOOKASIDE_LIST g_pPageListShadowSSDT = NULL;
PULONG g_pOriginalSSDTShadow = NULL;



EXTERN_C PVOID GetKeServiceDescriptorTableShadow()
{
	PULONG p;

	//XP
	//nt!KeAddSystemServiceTable+0x1a:
	//8059779e 8d88613f5580    lea     ecx,nt!g_KeServiceDescriptorTableShadow+0x1 (80553f61)[eax]
	//805977a4 833900          cmp     dword ptr [ecx],0
	//	805977a7 7546            jne     nt!KeAddSystemServiceTable+0x6b (805977ef)
	//8d88两个字节，所以+2
	p = (PULONG)((ULONG)KeAddSystemServiceTable + 0x1a + 2);


	return (PVOID)((PCHAR)(*p));
}

EXTERN_C ULONG GetShadowSSDTCount()
{
	return (ULONG)g_KeServiceDescriptorTableShadow[1].NumberOfServices;
}


EXTERN_C VOID SetShadowSSDT(PVOID pInfo)
{
	ASSERT(NULL != pInfo);

	PSHADOW_SSDT_INFO pShadowSSDTInfo = (PSHADOW_SSDT_INFO)pInfo;

	*(ULONG*)((ULONG)g_KeServiceDescriptorTableShadow[1].ServiceTableBase + pShadowSSDTInfo->ulIndex) = pShadowSSDTInfo->ulAddress;

}

EXTERN_C NTSTATUS EnumShadowSSDT( PVOID pOuputBuufer )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSSDT_FUNC_INFO pShadowSSDTFuncInfo = NULL, pPreShadowSSDTFuncInfo = NULL;
	ULONG ulShadowCount, ulIndex;
	HANDLE hNtSection;
	ULONG ulNtDllModuleBase;
	UNICODE_STRING wsNtDllString;
	CHAR lpszFunction[MAX_PATH];

	__try
	{
		FreePagedLookasideListForShadowSSDT();
		g_pPageListShadowSSDT = (PPAGED_LOOKASIDE_LIST)ExAllocatePool(PagedPool, sizeof(PAGED_LOOKASIDE_LIST));
		ExInitializePagedLookasideList(g_pPageListShadowSSDT, NULL, NULL, 0, sizeof(SSDT_FUNC_INFO), NULL, 0);
		ulShadowCount = GetShadowSSDTCount();
		g_pOriginalSSDTShadow = (PULONG)ExAllocatePool(PagedPool, ulShadowCount * sizeof(ULONG));

		GetOriginalSSDTShadowAddress(ulShadowCount);

		RtlInitUnicodeString(&wsNtDllString,L"\\SystemRoot\\System32\\ntdll.dll");
		hNtSection = MapFileAsSection(&wsNtDllString,(PVOID*)(&ulNtDllModuleBase));  //载入到内存
		if (!hNtSection)
		{
			//if (DebugOn)
			KdPrint(("MapFileAsSection failed"));

			return FALSE;
		}
		ZwClose(hNtSection);

		for (ulIndex = 0; ulIndex < ulShadowCount; ulIndex++)
		{
			pShadowSSDTFuncInfo = (PSSDT_FUNC_INFO)ExAllocateFromPagedLookasideList(g_pPageListShadowSSDT);
			RtlZeroMemory(pShadowSSDTFuncInfo, sizeof(SSDT_FUNC_INFO));

			pShadowSSDTFuncInfo->ulIndex = ulIndex;
			pShadowSSDTFuncInfo->ulCurrenAddress = *(PULONG)((ULONG)g_KeServiceDescriptorTableShadow[1].ServiceTableBase + ulIndex * 4);
			pShadowSSDTFuncInfo->ulOriginalAddress = g_pOriginalSSDTShadow[ulIndex];
			//获取ulCurrenAddress所在模块的路径
			GetModuleName(pShadowSSDTFuncInfo->lpModulePath, pShadowSSDTFuncInfo->ulCurrenAddress);


			//获取函数的名称
			RtlZeroMemory(lpszFunction, sizeof(lpszFunction));
			GetFunctionNameByIndex(ulNtDllModuleBase, &ulIndex, lpszFunction);
			RtlCopyMemory(pShadowSSDTFuncInfo->lpwzFuncName, lpszFunction, MAX_PATH);

			if (g_pShadowSSDTFuncInfo == NULL)
			{
				g_pShadowSSDTFuncInfo = pShadowSSDTFuncInfo;
				pPreShadowSSDTFuncInfo = pShadowSSDTFuncInfo;
			}
			else
			{
				pPreShadowSSDTFuncInfo->next = pShadowSSDTFuncInfo;
				pPreShadowSSDTFuncInfo = pShadowSSDTFuncInfo;
			}
		}
		GetShadowSSDTFuncInfo(pOuputBuufer);
		FreePagedLookasideListForShadowSSDT();
		status = STATUS_SUCCESS;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
	}

	return status;
}

NTSTATUS FreePagedLookasideListForShadowSSDT()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSSDT_FUNC_INFO pSSDTFuncInfo = g_pShadowSSDTFuncInfo, pNextSSDTFuncInfo = NULL;

	__try
	{
		if (NULL != g_pShadowSSDTFuncInfo && NULL != g_pPageListShadowSSDT)
		{
			while (pSSDTFuncInfo)
			{
				pNextSSDTFuncInfo = pSSDTFuncInfo->next;

				ExFreeToPagedLookasideList(g_pPageListShadowSSDT, pSSDTFuncInfo);

				pSSDTFuncInfo = pNextSSDTFuncInfo;
			}

			ExDeletePagedLookasideList(g_pPageListShadowSSDT);
			ExFreePool(g_pPageListShadowSSDT);
			g_pPageListShadowSSDT = NULL;
			g_pShadowSSDTFuncInfo = NULL;
		}
		status = STATUS_SUCCESS;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("SSDT:FreePagedLookasideListForSSDT failed!"));
		status = GetExceptionCode();
	}


	return status;
}

NTSTATUS GetShadowSSDTFuncInfo(PVOID pInfo)
{
	ULONG ulLength = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSSDT_FUNC_INFO pSSDTInf = g_pShadowSSDTFuncInfo;

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




//根据服务号获得原始函数地址
//ULONG GetOldShadowSSDTAddress(ULONG ulIndex)
//{
//	NTSTATUS status;
//	HANDLE hFile;
//
//	ULONG ulWin32kBase;
//	ULONG ulShadowBase;
//	//相对偏移量
//	ULONG ulShadowRVA;
//
//	UNICODE_STRING ustrFileName;
//	OBJECT_ATTRIBUTES  objectAttributes;
//	IO_STATUS_BLOCK io_status = {0};
//
//	//读取的位置
//	ULONG ulLocation;
//	LARGE_INTEGER offset;
//
//	//返回的函数地址
//	ULONG ulFuncAddress;
//
//	//获得Win32k基址
//	ulWin32kBase = GetWin32kBase();
//
//	//输出调试信息
//	DbgPrint("shadow.c GetOldSSDTAddress() ulWin32kBase:%08X",ulWin32kBase);
//
//	//获得内存中的Shadow SSDT基址
//	ulShadowBase = *(PULONG)&g_KeServiceDescriptorTableShadow[1].ServiceTableBase;
//
//	//输出调试信息
//	DbgPrint("shadow.c GetOldSSDTAddress() ulShadowBase:%08X",ulShadowBase);
//
//	//得到Shadow相对偏移量
//	ulShadowRVA = ulShadowBase - ulWin32kBase;
//
//	//输出调试信息
//	DbgPrint("shadow.c GetOldSSDTAddress() ulShadowRVA:%08X",ulShadowRVA);
//
//	//读取的位置
//	ulLocation = ulShadowRVA + ulIndex * 4;
//	offset.QuadPart = ulLocation;
//
//	//利用ZwReadFile读取文件
//	//初始化OBJECT_ATTRIBUTES结构
//	RtlInitUnicodeString(&ustrFileName, L"\\SystemRoot\\system32\\win32k.sys");
//
//	//初始化结构体
//	InitializeObjectAttributes( 
//		&objectAttributes,
//		&ustrFileName,
//		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
//		NULL,
//		NULL);
//
//	//打开文件
//	status = ZwCreateFile(
//		&hFile,
//		FILE_EXECUTE | SYNCHRONIZE,
//		&objectAttributes,
//		&io_status,
//		NULL,
//		FILE_ATTRIBUTE_NORMAL,
//		FILE_SHARE_READ,
//		FILE_OPEN,
//		FILE_NON_DIRECTORY_FILE |
//		FILE_RANDOM_ACCESS |
//		FILE_SYNCHRONOUS_IO_NONALERT,
//		NULL,
//		0);
//
//	if(!NT_SUCCESS(status))
//	{
//		//输出调试信息
//		DbgPrint("shadow.c GetOldSSDTAddress() ZwCreateFile error!");
//		ZwClose(hFile);
//		return NULL;
//	}
//
//	//读文件
//	status = ZwReadFile(
//		hFile,
//		NULL,
//		NULL,
//		NULL,
//		NULL,
//		&ulFuncAddress,
//		sizeof(ULONG),
//		&offset,
//		NULL);
//
//	if(!NT_SUCCESS(status))
//	{
//		//输出调试信息
//		DbgPrint("shadow.c GetOldSSDTAddress() ZwReadFile error!");
//		ZwClose(hFile);
//		return NULL;
//	}
//
//	//关闭
//	ZwClose(hFile);
//
//	//输出调试信息
//	DbgPrint("shadow.c GetOldSSDTAddress() ulFuncAddress:%d",ulFuncAddress);
//
//	return ulFuncAddress;
//}
//
//
//

ULONG
GetW32pServiceTable(
				  IN PVOID Buffer,
				  IN ULONG Offset)
{
	ULONG RetValue;
	PIMAGE_BASE_RELOCATION pReloc;
	ULONG RelocSize;
	ULONG RelocBase;
	ULONG TableSize;
	ULONG Temp;
	ULONG Number;
	ULONG i;

	//
	//	get the offset of relocation table in file 
	//

	pReloc = (PIMAGE_BASE_RELOCATION)((PCHAR)Buffer + \
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
		RelocBase = pReloc->VirtualAddress;
		TableSize = pReloc->SizeOfBlock;

		if(TableSize == 0)
		{
			break;
		}

		if (RelocBase == 0)
		{
			pReloc = (PIMAGE_BASE_RELOCATION)((PCHAR)pReloc + TableSize);
			continue;
		}

		Number = (TableSize - 0x08) >> 1;	//重定位数据的个数

		for( i = 0; i < Number; i++)
		{
			Temp = *(PUSHORT)((ULONG)pReloc + 0x08 + i * 0x02) & 0x0FFF;
			Temp += RelocBase;//RVA
			Temp += (ULONG)Buffer;//VA

			//
			//push    offset _W32pServiceTable
			//FF 15 XX XX XX XX
			//call    ds:__imp__KeAddSystemServiceTable@20 ; KeAddSystemServiceTable(x,x,x,x,x)

			//

			if(*(PULONG)Temp == Offset)
			{
				if(*(PUSHORT)(Temp - 2) == 0x15FF)
				{
					RetValue = *(PULONG)(Temp - 6);
					return RetValue;
				}
			}
		}
		pReloc = (PIMAGE_BASE_RELOCATION)((PCHAR)pReloc + TableSize);
	}

	return 0;
}

//返回存放输入表某个函数地址的地址的偏移
ULONG
GetImportData(
			  IN PVOID FileBase,
			  IN PCHAR DataName)
{
	ULONG Left;
	ULONG Right;
	ULONG Index;
	ULONG ImpSize;
	LONG RetValue;
	ULONG Temp;
	ULONG MappedExpBase;
	ULONG Name;
	ULONG ulFuncItem;
	PUSHORT AddressOfNameOrdinals;
	PULONG AddressOfNames;
	PULONG AddressOfFunctions;
	PIMAGE_IMPORT_DESCRIPTOR pImport;
	PIMAGE_THUNK_DATA pINT;
	PIMAGE_IMPORT_BY_NAME pImportByName;
	PULONG pIAT;
	PCHAR pImpDllName = NULL;


	pImport = (PIMAGE_IMPORT_DESCRIPTOR)((PCHAR)FileBase + \
		((PIMAGE_NT_HEADERS)((PCHAR)FileBase + ((PIMAGE_DOS_HEADER)FileBase)->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	ImpSize = (ULONG)((PIMAGE_NT_HEADERS)((PCHAR)FileBase + ((PIMAGE_DOS_HEADER)FileBase)->e_lfanew))->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;


	if (pImport == NULL)
	{
		return 0;
	}


	while (pImport != NULL)
	{
		pImpDllName = (PCHAR)((PCHAR)FileBase + pImport->Name);
		if (strcmp(pImpDllName, "ntoskrnl.exe") == NULL)
		{
			pINT = (PIMAGE_THUNK_DATA)((PCHAR)FileBase + pImport->OriginalFirstThunk);
			pIAT = (PULONG)((PCHAR)FileBase + pImport->FirstThunk);
			ulFuncItem = 0;
			while (pINT != NULL)
			{
				if (!(pINT->u1.AddressOfData & 0xF0000000)) //函数名方式
				{
					pImportByName = (PIMAGE_IMPORT_BY_NAME)((PCHAR)FileBase + pINT->u1.AddressOfData);
					if (strcmp((PCHAR)pImportByName->Name, DataName) == NULL)
					{
						break;
					}
				}
				ulFuncItem++;
				pINT++;
			}
			RetValue = (ULONG)pIAT + ulFuncItem*4 - (ULONG)FileBase;
			return RetValue;
		}
		pImport++;
	}

	return 0;
}



NTSTATUS
GetWin32k(
		OUT PCHAR ModuleName,
		OUT PULONG ModuleBase)
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

	while (pModuleInfo != NULL)
	{
		if (strstr(pModuleInfo->ImageName, "win32k.sys") > 0)
		{
			strcpy(ModuleName, pModuleInfo->ImageName);
			*ModuleBase = (ULONG)pModuleInfo->Base;
			Status = STATUS_SUCCESS;
			break;
		}
		pModuleInfo++;
	}


	ExFreePool(Buffer);
	return Status;
}


NTSTATUS
LoadFileInShadow(
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
* 功能描述:  获取原始SSDT shadow
INIT:BF9B505E                 mov     _countTable, esi
INIT:BF9B5064                 push    esi
INIT:BF9B5065                 push    offset _W32pServiceTable
INIT:BF9B506A                 call    ds:__imp__KeAddSystemServiceTable@20 ; KeAddSystemServiceTable(x,x,x,x,x)
* 参数列表:  ULONG ulSSDTCount
* 返回 值:   ULONG
/************************************/
ULONG GetOriginalSSDTShadowAddress(ULONG ulSSDTCount)
{

	NTSTATUS Status;
	ULONG ulImportFuncAddr;
	ULONG ImageBase;
	ULONG ulW32pServiceTable;
	ULONG ulWin32kAddr;
	PVOID FileBuffer;
	CHAR pWin32kPath[MAX_PATH] = "\\??\\c:";
	UNICODE_STRING usWin32kPath;
	ANSI_STRING asWin32kPath;


	GetWin32k(pWin32kPath, &ulWin32kAddr);

	RtlInitAnsiString(&asWin32kPath, pWin32kPath);

	RtlAnsiStringToUnicodeString(&usWin32kPath, &asWin32kPath, TRUE);

	LoadFileInShadow(&usWin32kPath, &FileBuffer);

	if (FileBuffer == NULL)
	{
		return 0;
	}

	//
	// get KeServiceDescriptorTable's offset from file
	//

	ulImportFuncAddr = GetImportData( FileBuffer, "KeAddSystemServiceTable");

	if( ulImportFuncAddr == 0)
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

	ulW32pServiceTable = GetW32pServiceTable( FileBuffer, ImageBase + ulImportFuncAddr);

	if(ulW32pServiceTable == 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	//
	// get function address
	//

	RtlCopyMemory(g_pOriginalSSDTShadow, (PVOID)(ulW32pServiceTable + (ULONG)FileBuffer - ImageBase), ulSSDTCount * 0x04);


	//
	//	relocate
	//

	for(int i = 0; i < ulSSDTCount; i++)
	{
		*(g_pOriginalSSDTShadow + i) -= ImageBase;
		*(g_pOriginalSSDTShadow + i) += ulWin32kAddr;
	}

	RtlFreeUnicodeString(&usWin32kPath);

	return 0;
}

