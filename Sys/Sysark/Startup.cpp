#include "Startup.h"


//全局变量
PSTARTUP_INFO g_pSystemStartupInfo = NULL, g_pPreSystemStartupInfo = NULL;
PAGED_LOOKASIDE_LIST g_PageListSystemStartup;


EXTERN_C NTSTATUS EnumSystemStartupInfo()
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING usSrvName, usItemName;


	FreePagedLookasideListForStartup();
	ExInitializePagedLookasideList(&g_PageListSystemStartup, NULL, NULL, 0, sizeof(STARTUP_INFO), NULL, 0);


	RtlInitUnicodeString(&usSrvName, L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify\\");
	RtlInitUnicodeString(&usItemName, L"DllName");
	QuerySubKeyRegistryInfo(&usSrvName, &usItemName);

	RtlInitUnicodeString(&usSrvName, L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\");
	RtlInitUnicodeString(&usItemName, L"Userinit");
	QuerySubKeyRegistryInfo(&usSrvName, &usItemName);

	RtlInitUnicodeString(&usSrvName, L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\");
	RtlInitUnicodeString(&usItemName, L"UIHost");
	QuerySubKeyRegistryInfo(&usSrvName, &usItemName);

	RtlInitUnicodeString(&usSrvName, L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\");
	RtlInitUnicodeString(&usItemName, L"Shell");
	QuerySubKeyRegistryInfo(&usSrvName, &usItemName);

	RtlInitUnicodeString(&usSrvName, L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\");
	RtlInitUnicodeString(&usItemName, L"Stubpath");
	QuerySubKeyRegistryInfo(&usSrvName, &usItemName);

	RtlInitUnicodeString(&usSrvName, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors\\");
	RtlInitUnicodeString(&usItemName, L"Driver");
	QuerySubKeyRegistryInfo(&usSrvName, &usItemName);

	RtlInitUnicodeString(&usSrvName, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Print\\Providers\\");
	RtlInitUnicodeString(&usItemName, L"Name");
	QuerySubKeyRegistryInfo(&usSrvName, &usItemName);

	RtlInitUnicodeString(&usSrvName, L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run\\");
	RtlInitUnicodeString(&usItemName, L"");
	QueryRegistryInfo(&usSrvName, &usItemName);

	//RtlInitUnicodeString(&usSrvName, L"");
	//RtlInitUnicodeString(&usItemName, L"");
	//QuerySubKeyRegistryInfo(&usSrvName, &usItemName, pStartupInfo);
	return status;
}


EXTERN_C NTSTATUS FreePagedLookasideListForStartup()
{
	NTSTATUS status = STATUS_SUCCESS;
	PSTARTUP_INFO pStartupInfo = NULL,\
		pNextStartupInfo = NULL;

	__try
	{

		if (g_pSystemStartupInfo != NULL)
		{
			pStartupInfo = g_pSystemStartupInfo;

			while (pStartupInfo != NULL)
			{
				pNextStartupInfo = pStartupInfo->next;

				ExFreeToPagedLookasideList(&g_PageListSystemStartup, pStartupInfo);

				pStartupInfo = pNextStartupInfo;
			}

			ExDeletePagedLookasideList(&g_PageListSystemStartup);
			g_pSystemStartupInfo = NULL;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("Services:FreePagedLookasideListForDriverModule failed!"));
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

EXTERN_C ULONG GetSystemStartupInfoCount()
{
	ULONG ulCount = 0;
	PSTARTUP_INFO pStartupInf = g_pSystemStartupInfo;

	while (pStartupInf)
	{
		++ulCount;
		pStartupInf = pStartupInf->next;
	}

	KdPrint(("Services:GetServicesCount is %d", ulCount));
	return ulCount;
}

EXTERN_C NTSTATUS GetSystemStartupInfo(PVOID pInfo)
{
	ULONG ulLength = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSTARTUP_INFO pStartupInf = g_pSystemStartupInfo;

	__try
	{
		while (pStartupInf)
		{

			RtlCopyMemory((PUCHAR)pInfo + ulLength, pStartupInf, sizeof(STARTUP_INFO));
			ulLength += sizeof(STARTUP_INFO);
			pStartupInf = pStartupInf->next;
		}
		status = STATUS_SUCCESS;

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//_asm int 3
		KdPrint(("Services:GetServicesInfo failed"));
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}


/************************************
* 函数名称:  QueryRegistryInfo
* 功能描述:  查询pSrvName键是否有名称为pItemName的项
* 参数列表:  PUNICODE_STRING pSrvName
* 参数列表:  PUNICODE_STRING pItemName
* 返回 值:   NTSTATUS
/************************************/
NTSTATUS QueryRegistryInfo( PUNICODE_STRING pSrvName, PUNICODE_STRING pItemName )
{
	NTSTATUS status = STATUS_SUCCESS;

	QuerySubKeyInfo(pSrvName, pItemName);

	return status;
}


/************************************
* 函数名称:  QuerySubKeyRegistryInfo
* 功能描述:  查询pSrvName下子键是否有名称为pItemName的项
* 参数列表:  PUNICODE_STRING pSrvName
* 参数列表:  PUNICODE_STRING pItemName
* 返回 值:   NTSTATUS
/************************************/
NTSTATUS QuerySubKeyRegistryInfo( PUNICODE_STRING pSrvName, PUNICODE_STRING pItemName )
{
	NTSTATUS status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES ObjectAttributes;
	HANDLE	HandleRegKey = NULL;
	ULONG SubKeyIndex, SubKeyCount, ulSize, ulItemValue;
	UNICODE_STRING usKeyName;
	PKEY_FULL_INFORMATION pKeyFullInfo = NULL;
	PKEY_BASIC_INFORMATION pKeyBasicInfo = NULL;

	__try
	{
		InitializeObjectAttributes(&ObjectAttributes, 
			pSrvName,
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
			NULL,    // handle
			NULL);
		status = ZwOpenKey(&HandleRegKey, KEY_READ, &ObjectAttributes);


		// 第一次调用是为了获取需要的长度
		ZwQueryKey(HandleRegKey, KeyFullInformation, NULL, 0, &ulSize);
		pKeyFullInfo = (PKEY_FULL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
		// 第二次调用是为了获取数据
		ZwQueryKey(HandleRegKey, KeyFullInformation, pKeyFullInfo, ulSize, &ulSize);
		SubKeyCount = pKeyFullInfo->SubKeys;

		//循环遍历各个子键
		for (SubKeyIndex = 0; SubKeyIndex < SubKeyCount; SubKeyIndex++)
		{
			//_asm int 3
			ZwEnumerateKey(HandleRegKey, SubKeyIndex, KeyBasicInformation, NULL, 0, &ulSize);
			pKeyBasicInfo = (PKEY_BASIC_INFORMATION)ExAllocatePool(PagedPool, ulSize);
			//获取第I个子键的数据
			ZwEnumerateKey(HandleRegKey, SubKeyIndex, KeyBasicInformation, pKeyBasicInfo, ulSize, &ulSize);


			//服务的名称
			//RtlCopyMemory(pServicesInfo->lpwzSrvName, pKeyBasicInfo->Name, pKeyBasicInfo->NameLength);
			usKeyName.Buffer = (PWCH)ExAllocatePool(PagedPool, pSrvName->Length + pKeyBasicInfo->NameLength);
			usKeyName.Length = pSrvName->Length + pKeyBasicInfo->NameLength;
			usKeyName.MaximumLength = usKeyName.Length;
			RtlZeroMemory(usKeyName.Buffer, usKeyName.Length);
			RtlCopyMemory(usKeyName.Buffer, pSrvName->Buffer, pSrvName->Length);
			RtlCopyMemory((PUCHAR)usKeyName.Buffer + pSrvName->Length, pKeyBasicInfo->Name, pKeyBasicInfo->NameLength);

			QuerySubKeyInfo(&usKeyName, pItemName);


			if (NULL != pKeyBasicInfo)
			{
				ExFreePool(pKeyBasicInfo);
				pKeyBasicInfo = NULL;
			}
			if (usKeyName.Buffer != NULL)
			{
				ExFreePool(usKeyName.Buffer);
				usKeyName.Buffer = NULL;
			}
		}

		if (NULL != pKeyFullInfo)
		{
			ExFreePool(pKeyFullInfo);
			pKeyFullInfo = NULL;
		}
		if (NULL != HandleRegKey)
		{
			ZwClose(HandleRegKey);
			HandleRegKey = NULL;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		status = STATUS_UNSUCCESSFUL;

	}

	if (NULL != pKeyFullInfo)
	{
		ExFreePool(pKeyFullInfo);
		pKeyFullInfo = NULL;
	}
	if (NULL != pKeyBasicInfo)
	{
		ExFreePool(pKeyBasicInfo);
		pKeyBasicInfo = NULL;
	}
	if (NULL != HandleRegKey)
	{
		ZwClose(HandleRegKey);
		HandleRegKey = NULL;
	}



	return status;
}

PVOID QuerySubKeyInfo( PUNICODE_STRING pKeyName, PUNICODE_STRING pItemName )
{
	NTSTATUS status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES ObjectAttributes;
	HANDLE	HandleRegKey = NULL;
	ULONG SubItemIndex, SubItemCount, ulSize, ulItemValue;
	UNICODE_STRING usItemName, usItemValue;
	PKEY_VALUE_FULL_INFORMATION pKeyFullValueInfo = NULL;
	PKEY_FULL_INFORMATION pKeyFullInfo = NULL;
	PKEY_VALUE_PARTIAL_INFORMATION pKeyPartialValueInfo = NULL;
	BOOLEAN bRet = FALSE;
	PSTARTUP_INFO pCurStartupInfo = NULL;


	__try
	{

		InitializeObjectAttributes(&ObjectAttributes, 
			pKeyName,
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
			NULL,    // handle
			NULL);
		status = ZwOpenKey(&HandleRegKey, KEY_READ, &ObjectAttributes);


		if (NULL != pItemName)
		{
			//查询项名称为pItemName的值
			ZwQueryValueKey(HandleRegKey, pItemName, KeyValuePartialInformation, NULL, 0, &ulSize);
			pKeyPartialValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
			status = ZwQueryValueKey(HandleRegKey, pItemName, KeyValuePartialInformation, pKeyPartialValueInfo, ulSize, &ulSize);
			if (NT_SUCCESS(status))
			{
				pCurStartupInfo = (PSTARTUP_INFO)ExAllocateFromPagedLookasideList(&g_PageListSystemStartup);
				RtlZeroMemory(pCurStartupInfo, sizeof(STARTUP_INFO));

				usItemValue.Length = usItemValue.MaximumLength = (USHORT)pKeyPartialValueInfo->DataLength;
				usItemValue.Buffer = (PWCHAR)(pKeyPartialValueInfo->Data);

				RtlCopyMemory(pCurStartupInfo->lpwzKeyPath, usItemValue.Buffer, usItemValue.Length);
				RtlCopyMemory(pCurStartupInfo->lpwzName, pItemName->Buffer, pItemName->Length);


				if (NULL == g_pSystemStartupInfo)
				{
					g_pSystemStartupInfo = pCurStartupInfo;
					g_pPreSystemStartupInfo = pCurStartupInfo;
				}
				else
				{
					g_pPreSystemStartupInfo->next = pCurStartupInfo;
					g_pPreSystemStartupInfo = pCurStartupInfo;
				}

			}

			if (NULL != pKeyPartialValueInfo)
			{
				ExFreePool(pKeyPartialValueInfo);
				pKeyPartialValueInfo = NULL;
			}

		}
		else
		{
			// 第一次调用是为了获取需要的长度
			ZwQueryKey(HandleRegKey, KeyFullInformation, NULL, 0, &ulSize);
			pKeyFullInfo = (PKEY_FULL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
			// 第二次调用是为了获取数据
			ZwQueryKey(HandleRegKey, KeyFullInformation, pKeyFullInfo, ulSize, &ulSize);
			SubItemCount = pKeyFullInfo->Values;

			//循环遍历各个子项
			for (SubItemIndex = 0; SubItemIndex < SubItemCount; SubItemIndex++)
			{
				//_asm int 3
				ZwEnumerateValueKey(HandleRegKey, SubItemIndex, KeyValueFullInformation, NULL, 0, &ulSize);
				pKeyFullValueInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
				//获取第I个子项的数据
				status = ZwEnumerateValueKey(HandleRegKey, SubItemIndex, KeyValueFullInformation, pKeyFullValueInfo, ulSize, &ulSize);

				if (NT_SUCCESS(status))
				{
					pCurStartupInfo = (PSTARTUP_INFO)ExAllocateFromPagedLookasideList(&g_PageListSystemStartup);
					RtlZeroMemory(pCurStartupInfo, sizeof(STARTUP_INFO));

					usItemName.Length = usItemName.MaximumLength =  (USHORT)pKeyFullValueInfo->NameLength;
					usItemName.Buffer = pKeyFullValueInfo->Name;
					usItemValue.Length = usItemValue.MaximumLength = (USHORT)pKeyFullValueInfo->DataLength;
					usItemValue.Buffer = (PWCHAR)((PUCHAR)pKeyFullValueInfo + pKeyFullValueInfo->DataOffset);

					RtlCopyMemory(pCurStartupInfo->lpwzKeyPath, usItemValue.Buffer, usItemValue.Length);
					RtlCopyMemory(pCurStartupInfo->lpwzName, usItemName.Buffer, usItemName.Length);

					if (NULL == g_pSystemStartupInfo)
					{
						g_pSystemStartupInfo = pCurStartupInfo;
						g_pPreSystemStartupInfo = pCurStartupInfo;
					}
					else
					{
						g_pPreSystemStartupInfo->next = pCurStartupInfo;
						g_pPreSystemStartupInfo = pCurStartupInfo;
					}
				}

				if (NULL != pKeyFullValueInfo)
				{
					ExFreePool(pKeyFullValueInfo);
					pKeyFullValueInfo = NULL;
				}

			}
		}

	
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		status = STATUS_UNSUCCESSFUL;
	}

	if (NULL != pKeyFullInfo)
	{
		ExFreePool(pKeyFullInfo);
		pKeyFullInfo = NULL;
	}
	if (NULL != pKeyFullValueInfo)
	{
		ExFreePool(pKeyFullValueInfo);
		pKeyFullValueInfo = NULL;
	}
	if (NULL != pKeyPartialValueInfo)
	{
		ExFreePool(pKeyPartialValueInfo);
		pKeyPartialValueInfo = NULL;
	}

	return (PVOID)pCurStartupInfo;
}