#ifndef _FILE_HEADER_
#define _FILE_HEADER_

#ifdef __cplusplus
extern "C"
{
#endif
#include <Ntstrsafe.h>
#include "Ioctrls.h"
#include "DataDef.h"
#ifdef __cplusplus
}
#endif 

struct _EX_PUSH_LOCK // 0x4
{
	union
	{
		ULONG Waiting; // +0x0(0x4)
		ULONG Exclusive; // +0x0(0x4)
		ULONG Shared; // +0x0(0x4)
		ULONG Value; // +0x0(0x4)
		void* Ptr; // +0x0(0x4)
	};
};
struct _HANDLE_TABLE_ENTRY_INFO // 0x4
{
	ULONG AuditMask; // +0x0(0x4)
};

typedef struct  tagWIN7_HANDLE_TABLE // 0x3c
{
	ULONG TableCode; // +0x0(0x4)
	PEPROCESS QuotaProcess; // +0x4(0x4)
	void* UniqueProcessId; // +0x8(0x4)
	struct _EX_PUSH_LOCK HandleLock; // +0xc(0x4)
	struct _LIST_ENTRY HandleTableList; // +0x10(0x8)
	struct _EX_PUSH_LOCK HandleContentionEvent; // +0x18(0x4)
	PVOID DebugInfo; // +0x1c(0x4) struct _HANDLE_TRACE_DEBUG_INFO*
	long ExtraInfoPages; // +0x20(0x4)
	ULONG Flags; // +0x24(0x4)
	//  BYTE StrictFIFO; // +0x24(0x1)
	ULONG FirstFreeHandle; // +0x28(0x4)
	struct _HANDLE_TABLE_ENTRY* LastFreeHandleEntry; // +0x2c(0x4)
	ULONG HandleCount; // +0x30(0x4)
	ULONG NextHandleNeedingPool; // +0x34(0x4)
	ULONG HandleCountHighWatermark; // +0x38(0x4)
}WIN7_HANDLE_TABLE,*PWIN7_HANDLE_TABLE;

typedef struct tagXP_HANDLE_TABLE // 0x44
{
	ULONG TableCode; // +0x0(0x4)
	struct _EPROCESS* QuotaProcess; // +0x4(0x4)
	void* UniqueProcessId; // +0x8(0x4)
	struct _EX_PUSH_LOCK HandleTableLock[0x4]; // +0xc(0x10)
	struct _LIST_ENTRY HandleTableList; // +0x1c(0x8)
	struct _EX_PUSH_LOCK HandleContentionEvent; // +0x24(0x4)
	struct _HANDLE_TRACE_DEBUG_INFO* DebugInfo; // +0x28(0x4)
	long ExtraInfoPages; // +0x2c(0x4)
	ULONG FirstFree; // +0x30(0x4)
	ULONG LastFree; // +0x34(0x4)
	ULONG NextHandleNeedingPool; // +0x38(0x4)
	long HandleCount; // +0x3c(0x4)
	ULONG Flags; // +0x40(0x4)
	UCHAR StrictFIFO; // +0x40(0x1)
}XP_HANDLE_TABLE ,*PXP_HANDLE_TABLE;

extern"C"
{
	typedef BOOLEAN (*EX_ENUMERATE_HANDLE_ROUTINE)(
		IN PVOID HandleTableEntry,
		IN HANDLE Handle,
		IN PVOID EnumParameter
		);

	NTKERNELAPI
		bool
		ExEnumHandleTable (
		PVOID HandleTable,
		EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
		PVOID EnumParameter,
		PHANDLE Handle );
}


EXTERN_C VOID DeleteFile(PUNICODE_STRING pFilePath);
EXTERN_C NTSTATUS EnumDirectory(char *lpDirName);
EXTERN_C ULONG GetDirectoryInfoCount();
EXTERN_C NTSTATUS GetDirectoryInfo(PVOID pInfo);
EXTERN_C NTSTATUS FreePagedLookasideListForDirectory();
EXTERN_C NTSTATUS ModifyRight(FILE_RIGHT_MODIFY* Data);
BOOLEAN EnumHandleCallBack(PHANDLE_TABLE_ENTRY Entry,HANDLE handle,ENUMHANDLE_PARAMETER* Param);

NTSTATUS FORCEINLINE MyIoCallDriver( IN PDEVICE_OBJECT DeviceObject, IN OUT PIRP Irp );
PDEVICE_OBJECT MyIoGetRelatedDeviceObject(IN PFILE_OBJECT FileObject);
NTSTATUS EventCompletion( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context );


#define GetHandleTableFromProcessXp(a)  (*(PULONG)((char*)a+0xc4))

#define GetHandleTableFromProcessWin7(a) (*(PULONG)((char*)a+0xf4))

#endif