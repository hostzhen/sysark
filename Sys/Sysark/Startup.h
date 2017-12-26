#ifndef _STARTUP_HEADER_
#define _STARTUP_HEADER_
#ifdef __cplusplus
extern "C"
{
#endif
#include "DataDef.h"
#include "Services.h"
#ifdef __cplusplus
}
#endif


EXTERN_C NTSTATUS EnumSystemStartupInfo();
EXTERN_C ULONG GetSystemStartupInfoCount();
EXTERN_C NTSTATUS GetSystemStartupInfo(PVOID pInfo);
EXTERN_C NTSTATUS FreePagedLookasideListForStartup();

NTSTATUS QueryRegistryInfo( PUNICODE_STRING pSrvName, PUNICODE_STRING pItemName );
NTSTATUS QuerySubKeyRegistryInfo(PUNICODE_STRING pSrvName, PUNICODE_STRING pItemName);
PVOID QuerySubKeyInfo(PUNICODE_STRING pKeyName, PUNICODE_STRING pItemName);




#endif