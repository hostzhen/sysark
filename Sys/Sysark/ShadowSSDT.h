#ifndef _SHADOW_SSDT_HEADER
#define _SHADOW_SSDT_HEADER
#ifdef __cplusplus
extern "C"
{
#endif
#include "DataDef.h"
#include "Services.h"
#include "DriverModule.h"
#ifdef __cplusplus
}
#endif

EXTERN_C PVOID GetKeServiceDescriptorTableShadow();
EXTERN_C ULONG GetShadowSSDTCount();
EXTERN_C VOID SetShadowSSDT(PVOID pInfo);
EXTERN_C NTSTATUS EnumShadowSSDT(PVOID pOuputBuufer);
EXTERN_C NTSTATUS FreePagedLookasideListForShadowSSDT();


NTSTATUS GetShadowSSDTFuncInfo(PVOID pInfo);

//获得win32k基址
ULONG GetWin32kBase();

//根据服务号获得原始函数地址
ULONG GetOldShadowSSDTAddress(ULONG ulIndex);


ULONG GetOriginalSSDTShadowAddress(ULONG ulSSDTCount);


#endif
