// SysarkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sysark.h"
#include "SysarkDlg.h"
#include "FirstLittleDlg.h"

extern ULONG g_nSelectItem;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static UINT indicators[] =
{
	IDS_INDICATOR_MESSAGE,
	IDS_INDICATOR_TIME
};

extern HANDLE g_hDevice;


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysarkDlg dialog

CSysarkDlg::CSysarkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSysarkDlg::IDD, pParent)
	, m_pStyle1Dlg(NULL)
{
	//{{AFX_DATA_INIT(CSysarkDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//卸载驱动
	UnloadNTDriver(DRIVER_NAME);
	//加载驱动
	if (LoadNTDriver(DRIVER_NAME, DRIVER_PATH))
	{
		m_hDevSrv = OpenDriver(DRIVER_NAME);
		g_hDevice = m_hDevSrv;
	}
}

void CSysarkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSysarkDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSysarkDlg, CDialog)
	//{{AFX_MSG_MAP(CSysarkDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_STATUS_MSG, OnStatusMsg)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysarkDlg message handlers

BOOL CSysarkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	m_wndStatusBar.Create(this); //We create the status bar
	m_wndStatusBar.SetIndicators(indicators,2); //Set the number of panes
	CRect rect;
	GetClientRect(&rect);
	//Size the two panes
	m_wndStatusBar.SetPaneInfo(0,IDS_INDICATOR_MESSAGE,
		 SBPS_NORMAL,rect.Width()-100);      
	m_wndStatusBar.SetPaneInfo(1,IDS_INDICATOR_TIME,SBPS_STRETCH ,0);
	//This is where we actually draw it on the screen
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,
				   IDS_INDICATOR_TIME);

//	::SendMessage(m_hWnd, WM_STATUS_MSG, (LPARAM)"hell", NULL);
	InitChildDialog();
	

	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSysarkDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


void CSysarkDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if (m_pStyle1Dlg != NULL && \
		::IsWindow(m_pStyle1Dlg->m_hWnd))
	{
		CRect rect;
		GetClientRect(&rect);
		rect.bottom -= 15;
		m_pStyle1Dlg->MoveWindow(&rect);
		m_pStyle1Dlg->ShowWindow(SW_SHOW);
		m_pStyle1Dlg->UpdateWindow();
		UpdateWindow();

	}

	if (::IsWindow(m_wndStatusBar.m_hWnd))
	{
		//This is where we actually draw it on the screen
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,
				   IDS_INDICATOR_TIME);
	}
	
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSysarkDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSysarkDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


VOID CSysarkDlg::Clear()
{
	//卸载驱动
	UnloadNTDriver(DRIVER_NAME);

	if (NULL != m_pStyle1Dlg)
	{
		delete m_pStyle1Dlg;
		m_pStyle1Dlg = NULL;
	}

}


//创建各个子控件
BOOL CSysarkDlg::InitChildDialog()
{
	BOOL bResult = TRUE;

	m_pStyle1Dlg = new CStyle1Dlg(this, m_hDevSrv);
	m_pStyle1Dlg->Create(IDD_STYLE_1_DLG, this);
	CRect rect;
	GetClientRect(&rect);
	rect.bottom -= 15;
	m_pStyle1Dlg->MoveWindow(&rect);
	
	LPCTSTR ItemTitle[] = {"进程", "驱动模块", "内核钩子", "注册表", "文件", "启动项", "服务"};
	m_pStyle1Dlg->InitChildDialog(7, ItemTitle, FirstLevel);
	

	m_pStyle1Dlg->ShowWindow(SW_SHOW);
	m_pStyle1Dlg->UpdateWindow();

	return bResult;
}


//更新状态栏信息
LRESULT CSysarkDlg::OnStatusMsg(WPARAM wParam, LPARAM lParam)
{
	//TODO: Add your message handle code
	PTCHAR lpMsg = (PTCHAR)wParam;
	m_wndStatusBar.SetPaneText(0, lpMsg);
	return 0;
}



