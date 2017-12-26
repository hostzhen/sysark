#ifndef REG_SUPORT_HEADER
#define REG_SUPORT_HEADER
#define BCD TEXT("\\REGISTRY\\MACHINE\\BCD")
#define ASM TEXT(L"\\REGISTRY\\MACHINE\\SAM")
#define SECURITY TEXT(L"\\REGISTRY\\MACHINE\\SECURITY")
#define SYSTEM   TEXT(L"\\REGISTRY\\MACHINE\\SYSTEM")
#define SOFT       TEXT(L"\\REGISTRY\\MACHINE\\SOFTWARE")
#define COMPON TEXT(L"\\REGISTRY\\components")
#define USER         TEXT(L"\\REGISTRY\\USER");
#define MACHINE  TEXT(L"\\REGISTRY\\MACHINE")
 extern "C"
 {
 #include"ntreg.h"
 }
#include "../DataDef.h"
#include <vector>
using namespace std;

typedef  struct hive  HIVE;
typedef  struct hive* PHIVE;

BOOL GetRightToAccessFile(HANDLE hSor,HANDLE hDes);

typedef struct  tagHiveFileList
{
	WCHAR Name[MAX_PATH];
	WCHAR Path[MAX_PATH];
}HIVE_FILE_LSIT,*PHIVE_FILE_LIST;


void CloseHive(PHIVE hive);
PHIVE  NewOpenHive(char* filename, int mode);
BOOL GetHiveFileList(vector<PHIVE_FILE_LIST> &vHiveFileList);

#endif 