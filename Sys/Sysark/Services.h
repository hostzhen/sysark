#ifndef _SERVICES_HEADER_
#define _SERVICES_HEADER_
#ifdef __cplusplus
extern "C"
{
#endif
#include "DataDef.h"
#ifdef __cplusplus
}
#endif

EXTERN_C NTSTATUS EnumServices();
EXTERN_C ULONG GetServicesCount();
EXTERN_C NTSTATUS GetServicesInfo(PVOID pInfo);
EXTERN_C NTSTATUS FreePagedLookasideListForServices();

BOOLEAN QueryServiceRunType(PUNICODE_STRING pSrvName, PSERVICES_INFO pServicesInfo);



#endif