// sceneviewer.h : main header file for the sceneviewer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


class CMainFrame;

// CSceneViewerApp:
// See sceneviewer.cpp for the implementation of this class
//

class CSceneViewerApp : public CWinApp
{
public:
	CSceneViewerApp();
	~CSceneViewerApp();

	void		resume();
	void		suspend();
	float		tick();
	void		setMainWindow( CMainFrame* mainwindow );
	bool		suspended() const;
	CMainFrame*	mainWindow() const;
	const char*	name() const;

	static CSceneViewerApp* get();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

private:
	char		m_appname[256];
	CMainFrame*	m_mainWindow;
	int			m_suspended;
	int			m_time;
};

extern CSceneViewerApp theApp;

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
