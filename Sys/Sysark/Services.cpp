#include "Services.h"

//全局变量
PSERVICES_INFO g_pServicesInfo = NULL;
PAGED_LOOKASIDE_LIST g_PageListServices;

NTSTATUS EnumServices()
{
	//_asm int 3
	NTSTATUS status = STATUS_SUCCESS;
	ULONG	SubKeyIndex, ResultLength, ulSize;
	HANDLE					i, HandleRegKey = NULL;
	UNICODE_STRING			RegistryKeyName, KeyValue;
	UNICODE_STRING			KeyName;
	OBJECT_ATTRIBUTES		ObjectAttributes;
	PLIST_ENTRY pListHead = NULL, pListCur = NULL;
	PLDR_DATA_TABLE_ENTRY pLdrDataTable = NULL;
	PSERVICES_INFO pServicesInfo = NULL,\
		pPreServicesInfo = NULL;
	PKEY_BASIC_INFORMATION pKeyBasicInfo = NULL;
	PKEY_FULL_INFORMATION pKeyFullInfo = NULL;
	/************************************************************************/
	/* 
	User-mode Handle		Corresponding Object Name 
	HKEY_LOCAL_MACHINE		\Registry\Machine 
	HKEY_USERS				\Registry\User 
	HKEY_CLASSES_ROOT		No kernel-mode equivalent 
	HKEY_CURRENT_USER		No simple kernel-mode equivalent, but see Registry Run-Time Library Routines 
	*/
	/************************************************************************/

	WCHAR ServiceRegisterPath[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\";

	//pListHead = ((PLIST_ENTRY)pDriverObj->DriverSection)->Flink;
	//pListCur = pListHead;

	__try
	{

		FreePagedLookasideListForServices();
		ExInitializePagedLookasideList(&g_PageListServices, NULL, NULL, 0, sizeof(SERVICES_INFO), NULL, 0);

		RtlInitUnicodeString(&RegistryKeyName, ServiceRegisterPath);
		InitializeObjectAttributes(&ObjectAttributes, 
			&RegistryKeyName,
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
			NULL,    // handle
			NULL);
		status = ZwOpenKey(&HandleRegKey, KEY_READ, &ObjectAttributes);

		// 第一次调用是为了获取需要的长度
		ZwQueryKey(HandleRegKey, KeyFullInformation, NULL, 0, &ulSize);
		pKeyFullInfo = (PKEY_FULL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
		// 第二次调用是为了获取数据
		ZwQueryKey(HandleRegKey, KeyFullInformation, pKeyFullInfo, ulSize, &ulSize);

		//循环遍历各个子项
		for (SubKeyIndex = 0; SubKeyIndex <pKeyFullInfo->SubKeys; SubKeyIndex++)
		{
			ZwEnumerateKey(HandleRegKey, SubKeyIndex, KeyBasicInformation, NULL, 0, &ulSize);
			pKeyBasicInfo = (PKEY_BASIC_INFORMATION)ExAllocatePool(PagedPool, ulSize);
			//获取第I个子项的数据
			ZwEnumerateKey(HandleRegKey, SubKeyIndex, KeyBasicInformation, pKeyBasicInfo, ulSize, &ulSize);

			pServicesInfo = (PSERVICES_INFO)ExAllocateFromPagedLookasideList(&g_PageListServices);
			RtlZeroMemory(pServicesInfo, sizeof(SERVICES_INFO));


			//服务的名称
			RtlCopyMemory(pServicesInfo->lpwzSrvName, pKeyBasicInfo->Name, pKeyBasicInfo->NameLength);
			KeyName.Buffer = (PWCH)ExAllocatePool(PagedPool, RegistryKeyName.Length + pKeyBasicInfo->NameLength);
			KeyName.Length = RegistryKeyName.Length + pKeyBasicInfo->NameLength;
			KeyName.MaximumLength = KeyName.Length;
			RtlZeroMemory(KeyName.Buffer, KeyName.Length);
			RtlCopyMemory(KeyName.Buffer, RegistryKeyName.Buffer, RegistryKeyName.Length);
			RtlCopyMemory((PUCHAR)KeyName.Buffer + RegistryKeyName.Length, pKeyBasicInfo->Name, pKeyBasicInfo->NameLength);
			if (!QueryServiceRunType(&KeyName, pServicesInfo))
			{
				if (NULL != pServicesInfo)
				{
					ExFreeToPagedLookasideList(&g_PageListServices, pServicesInfo);
					pServicesInfo = NULL;
				}
			}
			else
			{
				pServicesInfo->next = NULL;

				if (g_pServicesInfo == NULL)
				{
					g_pServicesInfo = pServicesInfo;
					pPreServicesInfo = pServicesInfo;
				}
				else
				{
					pPreServicesInfo->next = pServicesInfo;
					pPreServicesInfo = pServicesInfo;
				}
			}


			if (KeyName.Buffer != NULL)
			{
				ExFreePool(KeyName.Buffer);
				KeyName.Buffer = NULL;
			}

			if (pKeyBasicInfo != NULL)
			{
				ExFreePool(pKeyBasicInfo);
				pKeyBasicInfo = NULL;
			}
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		FreePagedLookasideListForServices();
		KdPrint(("Services:EnumServices failed!"));
		status = STATUS_UNSUCCESSFUL;
	}

	if (NULL != HandleRegKey)
	{
		ZwClose(HandleRegKey);
		HandleRegKey = NULL;
	}

	if (pKeyBasicInfo != NULL)
	{
		ExFreePool(pKeyBasicInfo);
		pKeyBasicInfo = NULL;
	}

	if (pKeyFullInfo != NULL)
	{
		ExFreePool(pKeyFullInfo);
		pKeyFullInfo = NULL;
	}

	return status;
}


NTSTATUS FreePagedLookasideListForServices()
{
	NTSTATUS status = STATUS_SUCCESS;
	PSERVICES_INFO pServicesInfo = NULL,\
		pNextServicesInfo = NULL;

	__try
	{

		if (g_pServicesInfo != NULL)
		{
			pServicesInfo = g_pServicesInfo;

			while (pServicesInfo != NULL)
			{
				pNextServicesInfo = pServicesInfo->next;

				ExFreeToPagedLookasideList(&g_PageListServices, pServicesInfo);

				pServicesInfo = pNextServicesInfo;
			}

			ExDeletePagedLookasideList(&g_PageListServices);
			g_pServicesInfo = NULL;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("Services:FreePagedLookasideListForDriverModule failed!"));
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

ULONG GetServicesCount()
{
	ULONG ulCount = 0;
	PSERVICES_INFO pServicesInf = g_pServicesInfo;

	while (pServicesInf)
	{
		++ulCount;
		pServicesInf = pServicesInf->next;
	}

	KdPrint(("Services:GetServicesCount is %d", ulCount));
	return ulCount;
}

NTSTATUS GetServicesInfo(PVOID pInfo)
{
	ULONG ulLength = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PSERVICES_INFO pServicesInf = g_pServicesInfo;

	__try
	{
		while (pServicesInf)
		{

			RtlCopyMemory((PUCHAR)pInfo + ulLength, pServicesInf, sizeof(SERVICES_INFO));
			ulLength += sizeof(SERVICES_INFO);
			pServicesInf = pServicesInf->next;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//_asm int 3
		KdPrint(("Services:GetServicesInfo failed"));
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

BOOLEAN QueryServiceRunType( PUNICODE_STRING pSrvName, PSERVICES_INFO pServicesInfo )
{
	OBJECT_ATTRIBUTES ObjectAttributes;
	HANDLE	HandleRegKey = NULL;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	ULONG SubItemIndex, SubItemCount, ulSize, ulItemValue;
	UNICODE_STRING usItemName, usItemValue;
	PKEY_VALUE_FULL_INFORMATION pKeyFullValueInfo = NULL;
	PKEY_FULL_INFORMATION pKeyFullInfo = NULL;
	BOOLEAN bRet = FALSE;

	/************************************************************************/
	/* ServiceType 服务类型：0×10为独立进程服务，0×20为共享进程服务（比如svchost）；
	StartType 启动类型：0 系统引导时加载，1 OS初始化时加载，2 由SCM（服务控制管理器）自动启动，3 手动启动，4 禁用。（注意，0和1只能用于驱动程序）
	ErrorControl 错误控制：0 忽略，1 继续并警告，2 切换到LastKnownGood的设置，3 蓝屏。
	ServiceBinary 服务程序位置：%11%表示system32目录，%10%表示系统目录(WINNT或Windows)，%12%为驱动目录system32\drivers。也可以不用变量，直接使用全路径。
	服务名MentoHUST后面有两个逗号，因为中间省略了一个不常用的参数flags。
	Description、ServiceType、StartType、ErrorControl四项是必须要有的，还有LoadOrderGroup、Dependencies等就不做详述了。                                                                     */
	/************************************************************************/

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
		SubItemCount = pKeyFullInfo->Values;

		//循环遍历各个子项
		for (SubItemIndex = 0; SubItemIndex < SubItemCount; SubItemIndex++)
		{
			//_asm int 3
			ZwEnumerateValueKey(HandleRegKey, SubItemIndex, KeyValueFullInformation, NULL, 0, &ulSize);
			pKeyFullValueInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(PagedPool, ulSize);
			//获取第I个子项的数据
			status = ZwEnumerateValueKey(HandleRegKey, SubItemIndex, KeyValueFullInformation, pKeyFullValueInfo, ulSize, &ulSize);

			if (status == STATUS_SUCCESS)
			{
				usItemName.Length = usItemName.MaximumLength =  (USHORT)pKeyFullValueInfo->NameLength;
				usItemName.Buffer = pKeyFullValueInfo->Name;
				usItemValue.Length = usItemValue.MaximumLength = (USHORT)pKeyFullValueInfo->DataLength;
				usItemValue.Buffer = (PWCHAR)((PUCHAR)pKeyFullValueInfo + pKeyFullValueInfo->DataOffset);


				if (RtlCompareMemory(usItemName.Buffer, L"Type", usItemName.Length))
				{
					ulItemValue = *(PULONG)usItemValue.Buffer;
					//RtlUnicodeStringToInteger(&usItemValue, 16, &ulItemValue);
					if (0x10 != ulItemValue && 0x20 != ulItemValue && 0x110 != ulItemValue && 0x120 != ulItemValue)
					{
						bRet = FALSE;
						break;
					}
					else
					{
						bRet = TRUE;
						//_asm int 3
					}
				}
				else if (RtlCompareMemory(usItemName.Buffer, L"DisplayName", usItemName.Length) == usItemName.Length)
				{
					RtlCopyMemory(pServicesInfo->lpwzSrvName, usItemValue.Buffer, usItemValue.Length);
				}
				else if (RtlCompareMemory(usItemName.Buffer, L"ImagePath", usItemName.Length) == usItemName.Length)
				{
					RtlCopyMemory(pServicesInfo->lpwzImageName, usItemValue.Buffer, usItemValue.Length);
				}
				else if (RtlCompareMemory(usItemName.Buffer, L"Description", usItemName.Length) == usItemName.Length)
				{
					RtlCopyMemory(pServicesInfo->lpwzDescription, usItemValue.Buffer, usItemValue.Length);
				}
				else if (RtlCompareMemory(usItemName.Buffer, L"Start", usItemName.Length) == usItemName.Length)
				{
					ulItemValue = *(PULONG)usItemValue.Buffer;
					switch(ulItemValue)
					{ 
					case(SERVICE_AUTO_START):
						RtlCopyMemory(pServicesInfo->lpwzBootType, L"自动", sizeof(L"自动"));
						break;
					case(SERVICE_BOOT_START):
						RtlCopyMemory(pServicesInfo->lpwzBootType, L"引导", sizeof(L"引导"));
						break;
					case(SERVICE_DEMAND_START):
						RtlCopyMemory(pServicesInfo->lpwzBootType, L"手动", sizeof(L"手动"));
						break;
					case(SERVICE_DISABLED):
						RtlCopyMemory(pServicesInfo->lpwzBootType, L"已禁用", sizeof(L"已禁用"));
						break;
					case(SERVICE_SYSTEM_START):
						RtlCopyMemory(pServicesInfo->lpwzBootType, L"系统", sizeof(L"系统"));
						break;
					default:
						RtlCopyMemory(pServicesInfo->lpwzBootType, L"未知", sizeof(L"未知"));
						break;
					}
				}
				if (pKeyFullValueInfo->Type == REG_SZ)
				{
					KdPrint(("The sub value type:REG_SZ\n"));
				}
				else if (pKeyFullValueInfo->Type == REG_EXPAND_SZ)
				{
					KdPrint(("The sub value type:REG_EXPAND_SZ\n"));
				}
				else if (pKeyFullValueInfo->Type == REG_DWORD)
				{
					KdPrint(("The sub value type:REG_DWORD\n"));
				}
				else if (pKeyFullValueInfo->Type == REG_BINARY)
				{
					KdPrint(("The sub value type:REG_BINARY\n"));
				}
			}

			if (NULL != pKeyFullValueInfo)
			{
				ExFreePool(pKeyFullValueInfo);
				pKeyFullValueInfo = NULL;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		bRet = FALSE;

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

	return bRet;
}