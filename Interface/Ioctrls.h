#ifndef IOCTLS_H
#define IOCTLS_H
#include <winioctl.h>

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#pragma message("CTL_CODE undefined. Include winioctl.h or wdm.h")
#endif

#define	FILE_DRIVER_SYSARK	0x0000420
#define IOCTL_FUNCTION_BASE 0X8000
#define	SystemModuleInfo	0x0B




//////////////////////////////////////////////////////////////////////////
//IO 控制码

#define CTL_CODE_SYSARK(i) CTL_CODE(FILE_DRIVER_SYSARK, IOCTL_FUNCTION_BASE+i, METHOD_IN_DIRECT, FILE_ANY_ACCESS)

//获取SSDT结构
#define IOCTL_GETSSDT			(ULONG)CTL_CODE_SYSARK(1)

//设置SSDT结构
#define IOCTL_SETSSDT			(ULONG)CTL_CODE_SYSARK(2)

//查询SSDT HOOK函数地址
#define IOCTL_GETHOOK			(ULONG)CTL_CODE_SYSARK(3)

//设置SSDT HOOK函数地址
#define IOCTL_SETHOOK			(ULONG)CTL_CODE_SYSARK(4)

//把NtQuerySystemInformation地址传到驱动
#define IOCTL_QSIADDR			(ULONG)CTL_CODE_SYSARK(5)

//把NtDeviceIoControlFile地址传到驱动
#define IOCTL_DICFADDR			(ULONG)CTL_CODE_SYSARK(6)

//强杀进程
#define IOCTL_KILL_PROCESS		(ULONG)CTL_CODE_SYSARK(7)

//开启注册表保护
#define IOCTL_REG_PROTECTION	(ULONG)CTL_CODE_SYSARK(8)

//停止注册表保护
#define IOCTL_STOP_PROTECTION   (ULONG)CTL_CODE_SYSARK(9)

//把事件传到驱动
#define IOCTL_SAVE_EVENT		(ULONG)CTL_CODE_SYSARK(10)

//获得注册表信息
#define IOCTL_REGISTRY_INFO		(ULONG)CTL_CODE_SYSARK(11)

//允许修改
#define IOCTL_ALLOW_MODIFY		(ULONG)CTL_CODE_SYSARK(12)

//枚举TCP
#define IOCTL_ENUMTCP			(ULONG)CTL_CODE_SYSARK(13)

//枚举UDP
#define IOCTL_ENUMUDP			(ULONG)CTL_CODE_SYSARK(14)

#define IOCTL_HOSTZHEN_OPERATION  (ULONG)CTL_CODE_SYSARK(15)

#define IOCTL_ENUM_PROCESS		(ULONG)CTL_CODE_SYSARK(16)//获取进程列表

#define IOCTL_ENUM_PROCESS_THREAD  (ULONG)CTL_CODE_SYSARK(17)//获取线程列表

#define IOCTL_ENUM_PROCESS_MODULE  (ULONG)CTL_CODE_SYSARK(18)//获取进程模块列表


//删除文件
#define IOCTL_DELETE_FILE		(ULONG)CTL_CODE_SYSARK(19)


#define IOCTL_GET_DRIVER_MODULE_INFO_SIZE				(ULONG)CTL_CODE_SYSARK(20)

#define IOCTL_ENUM_DRIVER_MODULE						(ULONG)CTL_CODE_SYSARK(21)

#define IOCTL_GET_SERVICES_COUNT						(ULONG)CTL_CODE_SYSARK(22)

#define IOCTL_ENUM_SERVICES								(ULONG)CTL_CODE_SYSARK(23)

#define IOCTL_GET_STARTUP_INFO_COUNT					(ULONG)CTL_CODE_SYSARK(24)

#define IOCTL_ENUM_STARTUP								(ULONG)CTL_CODE_SYSARK(25)

//获取SSDT结构
#define IOCTL_GET_SSDT_COUNT							(ULONG)CTL_CODE_SYSARK(26)

#define IOCTL_ENUM_SSDT									(ULONG)CTL_CODE_SYSARK(27)

#define IOCTL_SET_SSDT									(ULONG)CTL_CODE_SYSARK(28)

//查询SSDT HOOK函数地址
#define IOCTL_GET_SHADOW_HOOK_COUNT						(ULONG)CTL_CODE_SYSARK(29)

#define IOCTL_ENUM_SHADOW_HOOK							(ULONG)CTL_CODE_SYSARK(30)

#define IOCTL_SET_SHADOW_HOOK							(ULONG)CTL_CODE_SYSARK(31)

//查询目录信息
#define IOCTL_GET_DIRECTORY_INFO_COUNT					(ULONG)CTL_CODE_SYSARK(32)

#define IOCTL_ENUM_DIRECTORY_INFO						(ULONG)CTL_CODE_SYSARK(33)

//修改文件权限
#define IOCTL_MODIFY_FILE_RIGHT							(ULONG)CTL_CODE_SYSARK(34)

#endif
