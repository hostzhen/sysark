

BOOL LoadNTDriver(LPCTSTR lpszDriverName,LPCTSTR lpszDriverPath);
BOOL UnloadNTDriver( char * szSvrName );
HANDLE OpenDriver(LPCTSTR lpszDriverName);
BOOL CloseDriver(HANDLE hDriver);
