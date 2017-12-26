// Sysark.h : main header file for the SYSARK application
//

#if !defined(AFX_SYSARK_H__33BA5751_DA33_41A4_AA17_BA4FEFC16737__INCLUDED_)
#define AFX_SYSARK_H__33BA5751_DA33_41A4_AA17_BA4FEFC16737__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


/////////////////////////////////////////////////////////////////////////////
// CSysarkApp:
// See Sysark.cpp for the implementation of this class
//

class CSysarkApp : public CWinApp
{
public:
	CSysarkApp();
	~CSysarkApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSysarkApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSysarkApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSARK_H__33BA5751_DA33_41A4_AA17_BA4FEFC16737__INCLUDED_)
