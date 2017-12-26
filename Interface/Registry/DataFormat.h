#pragma once

class CDataFormat
{
private:

public:
	CDataFormat(void);
	~CDataFormat(void);
	static CString FormatDataToB(unsigned char *Buffer,unsigned long Length);
	static CString FormatDataToHex(unsigned char *Buffer,unsigned long Length);
};
