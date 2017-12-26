#ifndef _PROTECTION_HEADER
#define _PROTECTION_HEADER
#ifdef __cplusplus
extern "C"
{
#endif
#include "DataDef.h"
#ifdef __cplusplus
}
#endif

EXTERN_C VOID EnableWriteProtection();
EXTERN_C VOID DisableWriteProtection();

#endif