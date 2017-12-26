#ifndef _DATADEF_H
#define _DATADEF_H


#define PROCESS_MAX_COUNT 200
#define PROCESS_MODULE_MAX_COUNT 200


#define DRIVER_NAME "SysarkDriver"
#define DRIVER_PATH "Sysark.sys"
#define CONFIG_FILE_NAME "SysarkConf.xml"

//自定义消息
#define WM_STATUS_MSG (WM_USER+100)
#define WM_FILEDLG_LIST_MSG (WM_USER + 101)
#define WM_FILEDLG_TREE_MSG (WM_USER + 102)

//注册表
#define Hive_Registry_Path ("SYSTEM\\CurrentControlSet\\Control\\hivelist")

//标准的rootkey
#define ROOT_HKEY_CLASSES_ROOT		"HKEY_CLASSES_ROOT"
#define ROOT_HKEY_CURRENT_USER		"HKEY_CURRENT_USER"
#define ROOT_HKEY_LOCAL_MACHINE		"HKEY_LOCAL_MACHINE"
#define ROOT_HKEY_USERS				"HKEY_USERS"
#define ROOT_HKEY_CURRENT_CONFIG	"HKEY_CURRENT_CONFIG"

//rootkey对应的hive文件路径的标识
#define TAG_HKEY_LOCAL_MACHINE		"\\REGISTRY\\MACHINE\\"
#define TAG_HKEY_USERS				"\\REGISTRY\\USER\\"

enum LittleDlgType{
	UNKNOWN_DLG = -1,
	PROCESS_MODULE_DLG,
	PROCESS_THREAD_DLG
};
enum DlgType{
	PROCESS_DLG = 0,
	DRIVER_MODULE_DLG,
	SSDT_DLG,
	ShadowSSDT_DLG,
	REGISTRY_DLG,
	FILE_DLG,
	STARTUP_DLG,
	SERVICES_DLG
};
enum KernelHookDlgType{
	KernelHook_SSDT_DLG = 0,
	KernelHook_ShadowSSDT_DLG
};
enum DlgLevel{
	FirstLevel = 0,
	KernelHookLevel
};

//结构体

typedef struct _PROCESS_INFO
{
	ULONG32 bHide;				//是否是隐藏进程
	ULONG32 ProcessId;
	ULONG32 FatherProcessId;
	PCHAR pEProcess;
	CHAR ImageFileName[MAX_PATH];
	WCHAR ImagePathName[MAX_PATH];
	PVOID pNext;
}PROCESS_INFO, *PPROCESS_INFO;

typedef struct _ALL_PROCESSES_INFO
{
	ULONG32 uCount;
	PROCESS_INFO vProcessInf[200];
}ALL_PROCESSES_INFO, *PALL_PROCESSES_INFO;


typedef struct _PROCESS_MODULE_INFO	//进程模块信息
{
	ULONG BaseAddress;
	ULONG size;
	WCHAR ImagePathName[MAX_PATH];
}PROCESS_MODULE_INFO, *PPROCESS_MODULE_INFO;

typedef struct _ALL_PROCESS_MODULE_INFO
{
	ULONG32 uCount;
	PROCESS_MODULE_INFO vModuleInf[PROCESS_MODULE_MAX_COUNT];
}ALL_PROCESS_MODULE_INFO, *PALL_PROCESS_MODULE_INFO;


typedef struct _PROCESS_THREAD_INFO //进程线程信息
{
	ULONG ulHideType;           //0 正常模块，在driver_object中有   1 隐藏模块
	ULONG ulStatus;             //线程状态，0运行，1退出
	ULONG EThread;				//ETHREAD
	ULONG ThreadStart;          //开始
	ULONG Teb;
	ULONG ThreadID;
	CHAR lpszThreadModule[256];
	UCHAR ulPriority;		   //优先级
	ULONG ulContextSwitches;   //切换次数
	
}PROCESS_THREAD_INFO, *PPROCESS_THREAD_INFO;

typedef struct _ALL_PROCESS_THREAD_INFO
{
	ULONG32 uCount;
	PROCESS_THREAD_INFO vThreadInf[PROCESS_MODULE_MAX_COUNT];
}ALL_PROCESS_THREAD_INFO, *PALL_PROCESS_THREAD_INFO;


typedef struct _DRIVER_MODULE_INFO //驱动模块信息
{
	ULONG DllBase;
	ULONG SizeOfImage;
	WCHAR BaseDllName[MAX_PATH];
	WCHAR FullDllName[MAX_PATH];
	
	_DRIVER_MODULE_INFO* next;
	
}DRIVER_MODULE_INFO, *PDRIVER_MODULE_INFO;


typedef struct _SERVICES_INFO //服务信息
{
	WCHAR lpwzSrvName[MAX_PATH];
	WCHAR lpwzImageName[MAX_PATH];
	WCHAR lpwzDLLPath[MAX_PATH];
	WCHAR lpwzDescription[MAX_PATH];
	WCHAR lpwzBootType[MAX_PATH];
	//ULONG RunStatus;     //运行状态
	_SERVICES_INFO* next;
	
}SERVICES_INFO, *PSERVICES_INFO;


typedef struct _STARTUP_INFO //自启动信息
{
	WCHAR lpwzName[MAX_PATH];
	WCHAR lpwzKeyPath[MAX_PATH];
	WCHAR lpwzKeyValue[MAX_PATH];
	
	_STARTUP_INFO* next;
	
}STARTUP_INFO, *PSTARTUP_INFO;

typedef struct _SSDT_FUNC_INFO //SSDT/shadow SSDT函数信息
{
	ULONG ulIndex;					//序号
	ULONG ulOriginalAddress;		//原始函数地址
	ULONG ulCurrenAddress;			//当前函数地址
	CHAR lpwzFuncName[MAX_PATH];	//函数名称
	CHAR lpwzModulePath[MAX_PATH];	//当前函数地址所在模块
	
	_SSDT_FUNC_INFO* next;
}SSDT_FUNC_INFO, *PSSDT_FUNC_INFO;

//
//  Time conversion routines
//

typedef struct _TIME_FIELDS {
    SHORT Year;        // range [1601...]
    SHORT Month;       // range [1..12]
    SHORT Day;         // range [1..31]
    SHORT Hour;        // range [0..23]
    SHORT Minute;      // range [0..59]
    SHORT Second;      // range [0..59]
    SHORT Milliseconds;// range [0..999]
    SHORT Weekday;     // range [0..6] == [Sunday..Saturday]
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;

typedef struct _DIRECTORY_INFO{//目录信息
	WCHAR			FileName[MAX_PATH];      
	LARGE_INTEGER	AllocationSize;
	TIME_FIELDS		CreationTime;
	TIME_FIELDS		LastAccessTime ; 
	TIME_FIELDS     LastWriteTime;  
	TIME_FIELDS		ChangeTime;
	ULONG			FileAttributes ;    
}DIRECTORY_INFO ,*PDIRECTORY_INFO;

typedef struct _DIRECTORY_INFO_EX{
	CString   path;
	DIRECTORY_INFO  DirectoryInfo;
}DIRECTORY_INFO_EX ,*PDIRECTORY_INFO_EX ;


typedef  struct  FILE_RIGHT_
{
	HANDLE DesHandle;
	HANDLE SourceHandle;
}FILE_RIGHT_MODIFY,*PFILE_RIGHT_MODIFY;


void wcharTochar(const wchar_t *wchar, char *chr, int length);

#endif