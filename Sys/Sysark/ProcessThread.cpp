#include "ProcessThread.h"



EXTERN_C VOID EnumProcessThread( IN PEPROCESS pEProcess, IN PALL_PROCESS_THREAD_INFO pAllThreadInf)
{
	PETHREAD pEThread = NULL;
	PLIST_ENTRY pEntry = NULL, pEntryHeader = NULL;;
	int uCount = 0;

	KeAttachProcess(pEProcess);
	pEntryHeader = *(PLIST_ENTRY*)((PUCHAR)pEProcess + ThreadListHead_KPROCESS);

	pEntry = pEntryHeader->Flink;

	while (pEntry != pEntryHeader)
	{
		pEThread = (PETHREAD)((PUCHAR)pEntry - ThreadListEntry_KTHREAD);
		pAllThreadInf->vThreadInf[uCount].EThread = *(PULONG)&pEThread;
		pAllThreadInf->vThreadInf[uCount].Teb = *(ULONG *)((PUCHAR)pEThread + Teb_KTHREAD);
		pAllThreadInf->vThreadInf[uCount].ulPriority = *(ULONG *)((PUCHAR)pEThread + Priority_KTHREAD);
		pAllThreadInf->vThreadInf[uCount].ulContextSwitches = *(ULONG *)((PUCHAR)pEThread + ContextSwitches_KTHREAD);
		pAllThreadInf->vThreadInf[uCount].ThreadStart = *(ULONG *)((PUCHAR)pEThread + StartAddress_ETHREAD);
		pAllThreadInf->vThreadInf[uCount].ThreadID = *(ULONG *)((PUCHAR)pEThread + UniqueThread_ETHREAD);

		uCount++;
		pAllThreadInf->uCount = uCount;
		pEntry = pEntry->Flink;
	}

	KeDetachProcess();
}