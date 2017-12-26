#ifndef _PROCESS_H
#define _PROCESS_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "DataDef.h"
#ifdef __cplusplus
}
#endif


//全局变量
//extern PLOG_BUF	gPLog;


typedef NTSTATUS (*PZwQueryInformationProcess) (
										__in HANDLE ProcessHandle,
										__in PROCESSINFOCLASS ProcessInformationClass,
										__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
										__in ULONG ProcessInformationLength,
										__out_opt PULONG ReturnLength
										);



PETHREAD
NTAPI GetNextProcessThread(IN PEPROCESS Process, IN PETHREAD Thread OPTIONAL);
EXTERN_C NTSTATUS TerminateProcess( PEPROCESS Process );
EXTERN_C NTSTATUS SuspendProcess( PEPROCESS Process);
PVOID GetPspTerminateThread();
PETHREAD
NTAPI
GetNextProcessThread(IN PEPROCESS Process,
					 IN PETHREAD Thread OPTIONAL);


ULONG GetPlantformDependentInfo(ULONG dwFlag);

NTSTATUS GetProcessImagePath(IN ULONG32 dwPId, OUT PWCHAR ProcessImagePath);
VOID EnumByCurrentProcessHandleTable();
VOID FindByActiveProcessLinks();

NTSTATUS IsHideProcess(PEPROCESS pEprocess);
NTSTATUS IsExitProcess(PEPROCESS pEprocess);
BOOLEAN ProcessIsFind(ULONG uPID, PALL_PROCESSES_INFO pAllProcessInfo);
VOID RecordProcessInf(IN PVOID pInfo, IN PALL_PROCESSES_INFO pAllProcessInfo);
PCHAR GetPspTerminateThreadByPointer();

EXTERN_C VOID EnumProcessByActiveProcess(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp, IN PALL_PROCESSES_INFO pAllProcessInfo);
EXTERN_C VOID EnumProcessByThreadList( IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp, IN PALL_PROCESSES_INFO pAllProcessInfo );
EXTERN_C NTSTATUS ScanXpHandleTable( IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp, IN PALL_PROCESSES_INFO pAllProcessInfo );

#endif