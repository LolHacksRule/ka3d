// sceneviewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "sceneviewer.h"
#include "MainFrm.h"
#include "resource.h"
#include "version.h"
#include "sceneviewerDoc.h"
#include "sceneviewerView.h"
#include <hgr/Globals.h>
#include <lang/Debug.h>
#include <lang/System.h>
#include <lang/Globals.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


USING_NAMESPACE(lang)


// CSceneViewerApp

BEGIN_MESSAGE_MAP(CSceneViewerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CSceneViewerApp construction

CSceneViewerApp::CSceneViewerApp() :
	m_mainWindow( 0 ),
	m_suspended( 0 ),
	m_time( 0 )
{
	sprintf( m_appname, "hgr viewer version %s", SCENEVIEWER_VERSION_NUMBER );
}

CSceneViewerApp::~CSceneViewerApp()
{
	Debug::printf( "CSceneViewerApp::~CSceneViewerApp: Releasing globals\n" );
	NS(hgr,Globals)::cleanup();
	lang_Globals::cleanup();
}

const char* CSceneViewerApp::name() const
{
	return m_appname;
}


// The one and only CSceneViewerApp object

CSceneViewerApp theApp;

// CSceneViewerApp initialization

void CSceneViewerApp::resume()
{
	if ( 0 == --m_suspended )
	{
		if ( m_mainWindow != 0 )
		{
			m_mainWindow->Invalidate( FALSE );
			tick();
		}
	}
}

void CSceneViewerApp::suspend()
{
	++m_suspended;
}

float CSceneViewerApp::tick()
{
	int time = System::currentTimeMillis();
	if ( 0 == m_time )
		m_time = time;
	float dt = float(time - m_time) * 1e-3f;
	m_time = time;
	return dt;
}

bool CSceneViewerApp::suspended() const
{
	return m_suspended > 0;
}

void CSceneViewerApp::setMainWindow( CMainFrame* mainwindow )
{
	ASSERT( !m_mainWindow || !mainwindow ); // only one main window at a time
	m_mainWindow = mainwindow;
}

CMainFrame* CSceneViewerApp::mainWindow() const
{
	ASSERT_VALID( m_mainWindow );
	return m_mainWindow;
}

CSceneViewerApp* CSceneViewerApp::get()
{
	return &theApp;
}

BOOL CSceneViewerApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	AddDocTemplate( new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CSceneViewerDoc),
		RUNTIME_CLASS(CMainFrame),       // hgr
		RUNTIME_CLASS(CSceneViewerView)) );
	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// App command to run the dialog
void CSceneViewerApp::OnAppAbout()
{
	suspend();
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
	resume();
}


// CSceneViewerApp message handlers

void CAboutDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText( IDC_APPNAME, CSceneViewerApp::get()->name() );

	return TRUE;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
