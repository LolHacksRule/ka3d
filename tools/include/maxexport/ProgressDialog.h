#ifndef _PROGRESSDIALOG_H
#define _PROGRESSDIALOG_H


#include <lang/Object.h>
#include <lang/String.h>
#include <lightmappacker/ProgressInterface.h>


class ProgressDialog :
	public lang::Object,
	public lightmappacker::ProgressInterface
{
public:
	explicit ProgressDialog( HINSTANCE instance, HWND parent, 
		const NS(lang,String)& progresstitle );

	~ProgressDialog();

	void	destroy();

	/** Causes SilentQuit to be thrown. */
	void	abort();

	void	setText( const NS(lang,String)& text );

	void	setProgress( float pos );

private:
	HWND	m_hwnd;
	HWND	m_pbar;
	HWND	m_text;
	bool	m_abort;

	void	check();
	void	update();

	static BOOL CALLBACK	dlgproc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );
	static bool				flushWin32MsgQueue();

	ProgressDialog( const ProgressDialog& );
	ProgressDialog& operator=( const ProgressDialog& );
};


#endif // _PROGRESSDIALOG_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
