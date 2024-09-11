#include "StdAfx.h"
#include "maxexport.h"
#include "MySceneExportClassDesc.h"
#include <config.h>


static bool s_controlsInitialized = false;
static MySceneExportClassDesc s_classDesc;


BOOL APIENTRY DllMain( HINSTANCE instance, DWORD reason, void* )
{
	s_classDesc.instance = instance;
	
	if ( !s_controlsInitialized )
	{
		s_controlsInitialized = true;
		InitCustomControls( instance );
		InitCommonControls();
	}

    switch ( reason )
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

MAXEXPORT_API int LibNumberClasses()
{
	return 1;
}

MAXEXPORT_API ClassDesc* LibClassDesc( int i )
{
	switch ( i )
	{
	case 0:		return &s_classDesc;
	default:	return 0;
	}
}

MAXEXPORT_API const TCHAR* LibDescription()
{
	return _T("hgr export");
}

MAXEXPORT_API ULONG	LibVersion()
{
	return VERSION_3DSMAX;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
