#include "stdafx.h"
#include "DataDef.h"
#include <windows.h>


void wcharTochar(const wchar_t *wchar, char *chr, int length)  
{  
	WideCharToMultiByte( CP_ACP, 0, wchar, -1, chr, length, NULL, NULL );  
} 
