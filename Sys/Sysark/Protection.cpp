#include "Protection.h"

//恢复内存写保护
VOID EnableWriteProtection()
{
	__asm
	{
		push eax
		mov  eax, cr0
		or   eax, 0x10000
		mov  cr0, eax
		sti
		pop  eax
	}
}

//去除内存写保护
VOID DisableWriteProtection()
{
	__asm
	{
		push eax
		cli 
		mov eax, cr0
		and eax, ~0x10000
		mov cr0, eax
		pop eax
	}
}