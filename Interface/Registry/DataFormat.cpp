#include "StdAfx.h"
#include "DataFormat.h"

CDataFormat::CDataFormat(void)
{
}

CDataFormat::~CDataFormat(void)
{
}

CString CDataFormat::FormatDataToB(unsigned char *Buffer,unsigned long Length)
{
	CString Str,Ret=L"";
	wchar_t Temp[50]={0};
	int Pos=99;
	int a,b=0;
	for(;Length>0;Length-=1)
	{
		Pos=99;
		a=Buffer[Length-1];
		if(a==0)
			continue;
		while(1)
		{
			a=a/2;	
			b=a%2;
			if(b)
			{
				Temp[Pos--]=L'1';
			}
			else
			{
				Temp[Pos--]=L'0';
			}
			if(a<2)
				break;	
		}
		Str.Format("%s",&Temp[Pos+1]);
		Ret.Insert(0,Str);
	}
	return Ret;
}

CString CDataFormat::FormatDataToHex(unsigned char *Buffer, unsigned long Length)
{
	if(Length>0x1000)
		return L"";
	CString Ret=L"";
	CString Temp;
	for(; Length > 0; Length--)
	{
		Temp.Format("%02X ",Buffer[Length-1]);
		Ret.Insert(0,Temp);
	}
	return Ret;
}