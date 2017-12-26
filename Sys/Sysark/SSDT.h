#ifndef _SSDT_HEADER
#define _SSDT_HEADER
#ifdef __cplusplus
extern "C"
{
#endif
#include <ntifs.h>
#include "DataDef.h"
#include "Services.h"
#include "DriverModule.h"
#ifdef __cplusplus
}
#endif

EXTERN_C ULONG GetSSDTCount();
EXTERN_C VOID SetSSDT(PVOID pInfo);
EXTERN_C NTSTATUS EnumSSDT(PVOID pOuputBuufer);;
EXTERN_C NTSTATUS FreePagedLookasideListForSSDT();

NTSTATUS GetSSDTFuncInfo(PVOID pInfo);
ULONG GetOriginalSSDTAddress(ULONG ulSSDTCount);





#endif