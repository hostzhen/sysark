#ifndef _DRIVER_MODULE_HEADER_
#define _DRIVER_MODULE_HEADER_

#ifdef __cplusplus
extern "C"
{
#endif
#include "DataDef.h"
#ifdef __cplusplus
}
#endif


EXTERN_C NTSTATUS EnumDriverModules(PDRIVER_OBJECT pDriverObj);
EXTERN_C ULONG GetDriverModulesInfoCount();
EXTERN_C NTSTATUS GetDriverModulesInfo(PVOID pInfo);
EXTERN_C NTSTATUS FreePagedLookasideListForDriverModules();
EXTERN_C NTSTATUS GetDriverModulesName(ULONG ulFuncAddr, LPWSTR lpwPath);
EXTERN_C ULONG GetModuleName(PCHAR pModulePath, ULONG ulFuncAddress);




#endif