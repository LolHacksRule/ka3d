#include "StdAfx.h"
#include "SilentQuit.h"
#include "ProgressDialog.h"
#include "resource.h"
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <config.h>


USING_NAMESPACE(lang)


ProgressDialog::ProgressDialog( HINSTANCE instance, HWND parent, 
	const String& progresstitle )
{
	m_hwnd = CreateDialogParam( instance, MAKEINTRESOURCE(IDD_PROGRESS), parent, dlgproc, (LPARAM)this );
	m_pbar = GetDlgItem( m_hwnd, IDC_PROGRESS1 );
	m_text = GetDlgItem( m_hwnd, IDC_PROGRESSDESCRIPTION );
	
	assert( m_hwnd );
	assert( m_pbar );
	assert( m_text );

	SetWindowText( m_hwnd, ("Please wait - " + progresstitle).c_str() );
	SendMessage( m_pbar, PBM_SETRANGE, 0, MAKELPARAM(0,1000) );
	ShowWindow( m_hwnd, SW_NORMAL );

	m_abort = false;
}

ProgressDialog::~ProgressDialog()
{
	destroy();
}

void ProgressDialog::destroy()
{
	if ( m_hwnd != 0 )
	{
		DestroyWindow( m_hwnd );
		m_hwnd = 0;
	}
}

void ProgressDialog::setText( const String& text )
{
	assert( m_hwnd );

	Debug::printf( "%s\n", text.c_str() );

	check();
	SetWindowText( m_text, text.c_str() );
	update();
}

void ProgressDialog::setProgress( float pos )
{
	assert( m_hwnd );

	if ( pos < 0.f )
	{
		//Debug::printf( "WARNING: Progress indicator belo 0%%\n" );
		pos = 0.f;
	}
	else if ( pos > 1.f )
	{
		//Debug::printf( "WARNING: Progress indicator over 100%%\n" );
		pos = 1.f;
	}

	check();
	SendMessage( m_pbar, PBM_SETPOS, (int)(pos*1000.f), 0 );
	update();
}

void ProgressDialog::abort()
{
	m_abort = true;
}

void ProgressDialog::check()
{
	if ( m_abort || !m_hwnd )
	{
		Debug::printf( "Aborting export...\n" );
		throwError( SilentQuit() );
	}

	if ( !flushWin32MsgQueue() )
	{
		Debug::printf( "Quitting 3dsmax...\n" );
		throwError( SilentQuit() );
	}
}

void ProgressDialog::update()
{
	UpdateWindow( m_hwnd );
}

BOOL CALLBACK ProgressDialog::dlgproc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp )
{
	ProgressDialog* dlg = reinterpret_cast<ProgressDialog*>( GetWindowLongPtr(hwnd,GWLP_USERDATA) );

	switch ( msg ) 
	{
	case WM_INITDIALOG:{
		dlg = reinterpret_cast<ProgressDialog*>(lp);
		SetWindowLongPtr( hwnd, GWLP_USERDATA, lp );
		CenterWindow( hwnd, GetParent(hwnd) );
		break;}

	case WM_COMMAND:{
		switch ( LOWORD(wp) ) 
		{
		case IDABORT:{
			dlg->abort();
			break;}
		}
		break;}

	default:
		return FALSE;
	}
	return TRUE;
}       

bool ProgressDialog::flushWin32MsgQueue()
{
	MSG msg;
	if ( PeekMessage(&msg,0,0,0,PM_NOREMOVE) )
	{
		while ( WM_QUIT != msg.message &&
			PeekMessage(&msg,0,0,0,PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		return WM_QUIT != msg.message;
	}
	return true;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
