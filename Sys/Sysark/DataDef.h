#ifndef _DATADEF_H
#define _DATADEF_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <NTIFS.h>
#include <Ntstrsafe.h>
#include <ntimage.h>
#include "Ioctrls.h"
#ifdef __cplusplus
}
#endif 

#pragma comment(lib, "Ntstrsafe.lib")

//宏定义
#define MUTEX_INIT(v)      KeInitializeMutex(&v,0)
#define MUTEX_WAIT(v)         KeWaitForMutexObject(&v,Executive,KernelMode,FALSE,NULL)
#define MUTEX_RELEASE(v)         KeReleaseMutex(&v,FALSE)

#define MAX_MSG_LEN (1024*64 - 16)
#define MAX_PROCESS_COUNT 200	//预示的最大进程数


#define PAGEDCODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT")

#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")


#define EPROCESS_SIZE 0
#define PEB_OFFSET 1
#define FILE_NAME_OFFSET 2
#define PROCESS_LINK_OFFSET 3
#define PROCESS_ID_OFFSET 4
#define PROCESS_FATHER_ID_OFFSET 5
#define EXIT_TIME_OFFSET 6

#define PROCESS_MAX_COUNT 200
#define PROCESS_MODULE_MAX_COUNT 200
#define MAX_PATH 260
#define SECTION_ALIGNMENT 0X1000


#define TABLE_LEVEL_MASK 0x3
#define TABLE_ENTRY_LOCK_BIT 0x80000000

#define OBJECT_TO_OBJECT_HEADER( o )\
CONTAINING_RECORD( (o), OBJECT_HEADER, Body ) 


//成员变量偏移
#define InheritedFromUniqueProcessId_EPROCESS		0x14c		//EPROCESS中InheritedFromUniqueProcessId偏移
#define UniqueProcessId_EPROCESS					0X84        //EPROCESS中UniqueProcessId偏移
#define ActiveProcessLinks_EPROCESS					0x88		//EPROCESS中ActiveProcessLinks偏移
#define ObjectTable_EPROCESS						0xc4		//EPROCESS中ObjectTable偏移
#define Flags_EPROCESS								0x248		//EPROCESS中Flags偏移
#define ImageFileName_EPROCESS						0x174		//EPROCESS中ImageFileName偏移
#define ThreadListHead_KPROCESS						0x50		//KPROCESS中ThreadListHead偏移
#define HandleTableList_ObjectTable					0x1c		//ObjectTable中HandleTableList偏移
#define UniqueProcessId_ObjectTable					0x008		//ObjectTable中UniqueProcessId偏移
#define QuotaProcess_ObjectTable					0x004		//ObjectTable中QuotaProcess偏移
#define ThreadListEntry_ETHREAD						0x22c		//ETHREAD中ThreadListEntry偏移
#define StartAddress_ETHREAD						0x224		//ETHREAD中StartAddress偏移
#define UniqueThread_ETHREAD						0x1f0		//ETHREAD中UniqueThread偏移
#define Priority_KTHREAD							0x033		//KTHREAD中Priority偏移
#define ContextSwitches_KTHREAD						0x04c		//KTHREAD中Priority偏移
#define Teb_KTHREAD									0x020		//KTHREAD中Teb偏移
#define ThreadListEntry_KTHREAD						0x1b0		//KTHREAD中ThreadListEntry偏移


//自定义结构体

typedef struct _PROCESS_INFO	//进程信息
{
	ULONG32 bHide;				//是否是隐藏进程
	ULONG32 ProcessId;
	ULONG32 FatherProcessId;
	PEPROCESS pEProcess;
	CHAR ImageFileName[MAX_PATH];
	WCHAR ImagePathName[MAX_PATH];
	_PROCESS_INFO* pNext;
}PROCESS_INFO, *PPROCESS_INFO;

typedef struct _ALL_PROCESSES_INFO
{
	ULONG32 uCount;
	PROCESS_INFO vProcessInf[PROCESS_MAX_COUNT];
}ALL_PROCESSES_INFO, *PALL_PROCESSES_INFO;

typedef struct _PROCESS_MODULE_INFO	//进程模块信息
{
	ULONG32 BaseAddress;
	ULONG32 size;
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
	ULONG ulPriority;		   //优先级
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
	ULONG ulIndex;
	ULONG ulOriginalAddress;
	ULONG ulCurrenAddress;
	CHAR lpwzFuncName[MAX_PATH];
	CHAR lpModulePath[MAX_PATH];

	_SSDT_FUNC_INFO* next;
}SSDT_FUNC_INFO, *PSSDT_FUNC_INFO;

//ssdt恢复钩子时要用的结构体
typedef struct _SSDT_INFO
{
	ULONG ulIndex;
	ULONG ulAddress;
}SSDT_INFO, *PSSDT_INFO;

//shadow恢复钩子时要用的结构体
typedef struct _ShadowSSDT_INFO
{
	ULONG ulIndex;
	ULONG ulAddress;
}SHADOW_SSDT_INFO,*PSHADOW_SSDT_INFO;

typedef struct _DIRECTORY_INFO{//目录信息
	WCHAR			FileName[MAX_PATH];      
	LARGE_INTEGER	AllocationSize;
	TIME_FIELDS		CreationTime;
	TIME_FIELDS		LastAccessTime ; 
	TIME_FIELDS     LastWriteTime;  
	TIME_FIELDS		ChangeTime;
	ULONG			FileAttributes ;
	_DIRECTORY_INFO *next;
}DIRECTORY_INFO ,*PDIRECTORY_INFO;


typedef  struct  FILE_RIGHT_
{
	HANDLE DesHandle;
	HANDLE SourceHandle;
}FILE_RIGHT_MODIFY,*PFILE_RIGHT_MODIFY;

typedef struct tagENUMHANDLE_PARAMETER
{
	HANDLE DesHandle;
	HANDLE SorHadnle;
	ACCESS_MASK AccessMask;
}ENUMHANDLE_PARAMETER;



//SDT结构体
typedef struct ServiceDescriptorTable {
	unsigned int *ServiceTableBase; //“系统服务描述符表”的基地址
	unsigned int *ServiceCounterTable; //指向另一个索引表，该表包含了每个服务表项被调用的次数
	unsigned int NumberOfServices; //当前系统所支持的服务个数
	unsigned int *ParamTableBase; //指向SSPT（System Service Parameter Table，即系统服务参数表）
}ServiceDescriptorTable,*PServiceDescriptorTable;

//// Information Class 11
typedef struct _SYSTEM_MODULE_INFORMATION { 
	ULONG Reserved[2];
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT Index;
	USHORT Unknown;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

//系统模块列表 
typedef struct _SYSTEM_MODULE_LIST{ 
	ULONG ulCount; 
	SYSTEM_MODULE_INFORMATION smi[1]; 
} SYSTEM_MODULE_LIST, *PSYSTEM_MODULE_LIST; 


//获得SSDT在内核中的地址
extern PServiceDescriptorTable KeServiceDescriptorTable;

//Shadow SSDT
extern PServiceDescriptorTable g_KeServiceDescriptorTableShadow;


//未文档化结构体

typedef struct _HANDLE_TABLE
{
	ULONG                    TableCode;
	PEPROCESS                QuotaProcess;
	PVOID                    UniqueProcessId;
	PVOID		             HandleTableLock[4]; //EX_PUSH_LOCK
	LIST_ENTRY               HandleTableList; 
	PVOID		             HandleContentionEvent; //EX_PUSH_LOCK
	PVOID					 DebugInfo;	//PHANDLE_TRACE_DEBUG_INFO
	LONG                     ExtraInfoPages;
	ULONG                    FirstFree;
	ULONG                    LastFree;
	ULONG                    NextHandleNeedingPool;
	LONG                     HandleCount;
	LONG                     Flags;
	UCHAR                    StrictFIFO;
} HANDLE_TABLE, *PHANDLE_TABLE;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union {
		LIST_ENTRY HashLinks;
		struct {
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union {
		struct {
			ULONG TimeDateStamp;
		};
		struct {
			PVOID LoadedImports;
		};
	};
	PVOID EntryPointActivationContext;
	PVOID PatchInformation;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _HANDLE_TABLE_ENTRY
{
	union
	{
		PVOID                    Object;
		ULONG                    ObAttributes;
		PVOID					 InfoTable; //PHANDLE_TABLE_ENTRY_INFO
		ULONG                    Value;
	};

	union
	{
		union
		{
			ACCESS_MASK GrantedAccess;
			struct
			{
				USHORT GrantedAccessIndex;
				USHORT CreatorBackTraceIndex;
			};
		};

		LONG NextFreeTableEntry;
	};
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;



//
// Client impersonation information
//

typedef struct _PS_IMPERSONATION_INFORMATION {
	PACCESS_TOKEN Token;
	BOOLEAN CopyOnOpen;
	BOOLEAN EffectiveOnly;
	SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
} PS_IMPERSONATION_INFORMATION, *PPS_IMPERSONATION_INFORMATION;

#pragma pack(push, 1)
typedef struct _KTHREAD {

	//
	// The dispatcher header and mutant listhead are fairly infrequently
	// referenced, but pad the thread to a 32-byte boundary (assumption
	// that pool allocation is in units of 32-bytes).
	//

	DISPATCHER_HEADER Header;
	LIST_ENTRY MutantListHead;

	//
	// The following fields are referenced during trap, interrupts, or
	// context switches.
	//
	// N.B. The Teb address and TlsArray are loaded as a quadword quantity
	//      on MIPS and therefore must be on a quadword boundary.
	//

	PVOID InitialStack;
	PVOID StackLimit;
#if defined(_IA64_)
	PVOID InitialBStore;
	PVOID BStoreLimit;
#endif
	PVOID Teb;
	PVOID TlsArray;
	PVOID KernelStack;
#if defined(_IA64_)
	PVOID KernelBStore;
#endif
	BOOLEAN DebugActive;
	UCHAR State;
	BOOLEAN Alerted[MaximumMode];
	UCHAR Iopl;
	UCHAR NpxState;
	CHAR Saturation;
	SCHAR Priority;
	KAPC_STATE ApcState;
	ULONG ContextSwitches;
	UCHAR IdleSwapBlock;
	UCHAR Spare0[3];

	//
	// The following fields are referenced during wait operations.
	//

	LONG_PTR WaitStatus;
	KIRQL WaitIrql;
	KPROCESSOR_MODE WaitMode;
	BOOLEAN WaitNext;
	UCHAR WaitReason;
	PRKWAIT_BLOCK WaitBlockList;
	LIST_ENTRY WaitListEntry;
	SINGLE_LIST_ENTRY SwapListEntry;
	ULONG WaitTime;
	SCHAR BasePriority;
	UCHAR DecrementCount;
	SCHAR PriorityDecrement;
	SCHAR Quantum;
	KWAIT_BLOCK WaitBlock[THREAD_WAIT_OBJECTS + 1];
	PVOID LegoData;
	ULONG KernelApcDisable;
	KAFFINITY UserAffinity;
	BOOLEAN SystemAffinityActive;
	UCHAR PowerState;
	UCHAR NpxIrql;
	UCHAR InitialNode;
	PVOID ServiceTable;
	//    struct _ECHANNEL *Channel;
	//    PVOID Section;
	//    PCHANNEL_MESSAGE SystemView;
	//    PCHANNEL_MESSAGE ThreadView;

	//
	// The following fields are referenced during queue operations.
	//

	PVOID Queue;//KQUEUE
	KSPIN_LOCK ApcQueueLock;
	KTIMER Timer;
	LIST_ENTRY QueueListEntry;

	//
	// The following fields are referenced during read and find ready
	// thread.
	//
	KAFFINITY SoftAffinity;
	KAFFINITY Affinity;
	BOOLEAN Preempted;
	BOOLEAN ProcessReadyQueue;
	BOOLEAN KernelStackResident;
	UCHAR NextProcessor;

	//
	// The following fields are referenced during system calls.
	//

	PVOID CallbackStack;
#if defined(_IA64_)
	PVOID CallbackBStore;
#endif
	PVOID Win32Thread;
	PVOID TrapFrame;
	PVOID ApcStatePointer[2];//KAPC_STATE
	CCHAR PreviousMode;
	UCHAR EnableStackSwap;
	UCHAR LargeStack;
	UCHAR ResourceIndex;

	//
	// The following entries are referenced during clock interrupts.
	//

	ULONG KernelTime;
	ULONG UserTime;

	//
	// The following fields are referenced during APC queuing and process
	// attach/detach.
	//

	KAPC_STATE SavedApcState;//KAPC_STATE
	BOOLEAN Alertable;
	UCHAR ApcStateIndex;
	BOOLEAN ApcQueueable;
	BOOLEAN AutoAlignment;

	//
	// The following fields are referenced when the thread is initialized
	// and very infrequently thereafter.
	//

	PVOID StackBase;
	KAPC SuspendApc;
	KSEMAPHORE SuspendSemaphore;
	LIST_ENTRY ThreadListEntry;

	//
	// N.B. The below four UCHARs share the same DWORD and are modified
	//      by other threads. Therefore, they must ALWAYS be modified
	//      under the dispatcher lock to prevent granularity problems
	//      on Alpha machines.
	//

	CCHAR FreezeCount;
	CCHAR SuspendCount;
	UCHAR IdealProcessor;
	UCHAR DisableBoost;

} KTHREAD, *PKTHREAD, *RESTRICTED_POINTER PRKTHREAD;





typedef struct _MMSUPPORT {
	LARGE_INTEGER LastTrimTime;

	ULONGLONG Flags; //MMSUPPORT_FLAGS
	ULONG PageFaultCount;
	ULONG PeakWorkingSetSize; //WSLE_NUMBER
	ULONG WorkingSetSize;
	ULONG MinimumWorkingSetSize; //WSLE_NUMBER
	ULONG MaximumWorkingSetSize; //WSLE_NUMBER
	PVOID VmWorkingSetList; //struct _MMWSL *
	ULONG Claim;

	LIST_ENTRY WorkingSetExpansionLinks;
	ULONG NextEstimationSlot;
	ULONG NextAgingSlot;
	ULONG EstimatedAvailable;

	ULONG WorkingSetMutex; //EX_PUSH_LOCK

} MMSUPPORT, *PMMSUPPORT;


typedef struct _KPROCESS {

	//
	// The dispatch header and profile listhead are fairly infrequently
	// referenced.
	//

	DISPATCHER_HEADER Header;
	LIST_ENTRY ProfileListHead;

	//
	// The following fields are referenced during context switches.
	//

	ULONG_PTR DirectoryTableBase[2];

#if defined(_X86_)

	ULONGLONG LdtDescriptor; //KGDTENTRY
	ULONGLONG Int21Descriptor; //KGDTENTRY
	USHORT IopmOffset;
	UCHAR Iopl;
	BOOLEAN Unused;

#endif

#if defined(_AMD64_)

	USHORT IopmOffset;

#endif

	PVOID ActiveProcessors; //volatile KAFFINITY

	//
	// The following fields are referenced during clock interrupts.
	//

	ULONG KernelTime;
	ULONG UserTime;

	//
	// The following fields are referenced infrequently.
	//

	LIST_ENTRY ReadyListHead;
	SINGLE_LIST_ENTRY SwapListEntry;

#if defined(_X86_)

	PVOID VdmTrapcHandler;

#else

	PVOID Reserved1;

#endif

	LIST_ENTRY ThreadListHead;
	KSPIN_LOCK ProcessLock;
	KAFFINITY Affinity;

	//
	// N.B. The following bit number definitions must match the following
	//      bit field.
	//
	// N.B. These bits can only be written with interlocked operations.
	//

#define KPROCESS_AUTO_ALIGNMENT_BIT 0
#define KPROCESS_DISABLE_BOOST_BIT 1
#define KPROCESS_DISABLE_QUANTUM_BIT 2

	USHORT StackCount;

	CHAR BasePriority;
	CHAR ThreadQuantum;
	UCHAR AutoAlignment;
	UCHAR State;
	UCHAR ThreadSeed;
	UCHAR DisableBoost;
	UCHAR PowerState;
	UCHAR DisableQuantum;
	UCHAR IdealNode;
	union {
		UCHAR Flags; //KEXECUTE_OPTIONS
		UCHAR ExecuteOptions;
	};

} KPROCESS, *PKPROCESS, *PRKPROCESS;



#pragma pack(pop)

typedef struct _OBJECT_HEADER { 
	LONG PointerCount; 
	union { 
		LONG HandleCount; 
		PSINGLE_LIST_ENTRY SEntry; 
	}; 
	POBJECT_TYPE Type; 
	UCHAR NameInfoOffset; 
	UCHAR HandleInfoOffset; 
	UCHAR QuotaInfoOffset; 
	UCHAR Flags; 
	union 
	{ 
		PVOID ObjectCreateInfo; //POBJECT_CREATE_INFORMATION
		PVOID QuotaBlockCharged; 
	}; 

	PSECURITY_DESCRIPTOR SecurityDescriptor; 
	QUAD Body; 
} OBJECT_HEADER, *POBJECT_HEADER; 

//
//typedef struct _SYSTEM_MODULE_INFORMATION { 
//	ULONG Reserved[2]; 
//	PVOID Base; 
//	ULONG Size; 
//	ULONG Flags; 
//	USHORT Index; 
//	USHORT Unknown; 
//	USHORT LoadCount; 
//	USHORT ModuleNameOffset; 
//	CHAR ImageName[MAX_PATH]; 
//} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct _tagSysModuleList {
	ULONG ulCount;
	SYSTEM_MODULE_INFORMATION smi[1];
} MODULES, *PMODULES;
//
//typedef struct _SERVICE_DESCRIPTOR_TABLE {
//	/*
//	* Table containing cServices elements of pointers to service handler
//	* functions, indexed by service ID.
//	*/
//	PDWORD   ServiceTable;
//	/*
//	* Table that counts how many times each service is used. This table
//	* is only updated in checked builds.
//	*/
//	PULONG  CounterTable;
//	/*
//	* Number of services contained in this table.
//	*/
//	ULONG   TableSize;
//	/*
//	* Table containing the number of bytes of parameters the handler
//	* function takes.
//	*/
//	PUCHAR  ArgumentTable;
//} SERVICE_DESCRIPTOR_TABLE, *PSERVICE_DESCRIPTOR_TABLE;
//
//
//extern PSERVICE_DESCRIPTOR_TABLE    KeServiceDescriptorTable;


typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,  // 11
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,  // 0x10 -- 16
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemUnused1,
	SystemPerformanceTraceInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemUnused3,
	SystemUnused4,
	SystemUnused5,
	SystemUnused6,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation,
	SystemTimeSlipNotification,
	SystemSessionCreate,
	SystemSessionDetach,
	SystemSessionInformation
} SYSTEM_INFORMATION_CLASS;

typedef enum _MEMORY_INFORMATION_CLASS
{
	MemoryBasicInformation,
	MemoryWorkingSetList,
	MemorySectionName,
	MemoryBasicVlmInformation
}MEMORY_INFORMATION_CLASS;

typedef struct _MEMORY_BASIC_INFORMATION { 
	PVOID BaseAddress; 
	PVOID AllocationBase; 
	DWORD AllocationProtect; 
	DWORD RegionSize; 
	DWORD State; 
	DWORD Protect; 
	DWORD Type;
} MEMORY_BASIC_INFORMATION,*PMEMORY_BASIC_INFORMATION; 


typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY {
	HANDLE Section;
	PVOID MappedBase;
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT PathLength;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION_ENTRY, *PSYSTEM_MODULE_INFORMATION_ENTRY;

// undocumented API

typedef PVOID (*_PsGetProcessPeb)(IN PEPROCESS pEprocess);
typedef NTSTATUS NTKERNELAPI (*PSPTERMINATEPROCESS)(//PspTerminateProcess
	PEPROCESS Process,
	NTSTATUS ExitStatus
	);
typedef NTSTATUS (*PSPTERMINATETHREAD)( PETHREAD, NTSTATUS);//PspTerminateThread



NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation(
						 IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
						 IN OUT PVOID SystemInformation,
						 IN ULONG SystemInformationLength,
						 OUT PULONG ReturnLength OPTIONAL
						 );

typedef NTSTATUS (__stdcall * SysarkZwQueryVirtualMemory)(
	IN HANDLE               ProcessHandle,
	IN PVOID                BaseAddress,
	IN ULONG MemoryInformationClass,
	OUT PVOID               Buffer,
	IN ULONG                Length,
	OUT PULONG              ResultLength OPTIONAL
	);


NTSYSAPI
BOOLEAN
NTAPI
PsIsThreadTerminating(
					  __in PETHREAD  Thread
					  ); 


NTSYSAPI
BOOLEAN
NTAPI
KeAddSystemServiceTable (
						 IN PULONG_PTR Base,
						 IN PULONG Count OPTIONAL,
						 IN ULONG Limit,
						 IN PUCHAR Number,
						 IN ULONG Index
						 );


// 函数声明
EXTERN_C NTSTATUS DataInitialize();
EXTERN_C ULONG GetUndocumentFuncAddrByName(PUNICODE_STRING lpFuncName);
EXTERN_C BOOLEAN IsProcess(PVOID Eprocess);
EXTERN_C BOOLEAN GetFunctionNameByIndex(ULONG ulModuleBase, PULONG Index, PCHAR lpszFunctionName);
EXTERN_C HANDLE MapFileAsSection(PUNICODE_STRING FileName,PVOID *ModuleBase);


//
// Define types that are not exported.
//

typedef BOOLEAN BOOL;
typedef unsigned long DWORD;
typedef DWORD * LPDWORD;
typedef DWORD * PDWORD;
typedef unsigned long ULONG;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int UINT;


//Indicates that the memory pages within the region are mapped into the view of an image section.msdn
#define SEC_IMAGE         0x1000000
#define MEM_IMAGE SEC_IMAGE 

#endif