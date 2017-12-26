/************************************************************************
* 文件名称:Driver.h                                                 
* 作    者:张帆
* 完成日期:2007-11-1
*************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
#include <NTIFS.h>
#include "Ioctrls.h"
#include "DataDef.h"
#include "Process.h"
#include "Startup.h"
#include "Services.h"
#include "Protection.h"
#include "ProcessModule.h"
#include "ProcessThread.h"
#include "DriverModule.h"
#include "SSDT.h"
#include "ShadowSSDT.h"
#include "File.h"
#ifdef __cplusplus
}
#endif 


#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;
	UNICODE_STRING ustrDeviceName;	//设备名称
	UNICODE_STRING ustrSymLinkName;	//符号链接名
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

//全局变量
PDRIVER_OBJECT g_pDriverObj = NULL;

// 函数声明
VOID FreeAllPageLookasideLists();
NTSTATUS CreateDevice (IN PDRIVER_OBJECT pDriverObject);
VOID IoDriverUnload (IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutine(IN PDEVICE_OBJECT pDevObj,
								 IN PIRP pIrp);
NTSTATUS IoDispatchDeviceControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);
NTSTATUS HOSTZHEN_DispatchCreateClose(IN PDEVICE_OBJECT		DeviceObject, IN PIRP					Irp);