#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include "DataDef.h"
#ifdef __cplusplus
}
#endif


EXTERN_C VOID EnumProcessModuleByVM(PEPROCESS pEProcess, PALL_PROCESS_MODULE_INFO pAllModuleInf);

EXTERN_C VOID EnumProcessModuleByPEB( IN PEPROCESS pEProcess, IN PALL_PROCESS_MODULE_INFO pAllModuleInf);

EXTERN_C NTSTATUS UnmapViewOfModule ( IN HANDLE hProcess, IN PVOID lpBaseAddr );