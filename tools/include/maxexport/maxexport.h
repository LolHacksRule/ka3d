#ifndef _MAXEXPORT_H
#define _MAXEXPORT_H


#define MAXEXPORT_EXPORTS

#ifdef MAXEXPORT_EXPORTS
#define MAXEXPORT_API extern "C" __declspec(dllexport) 
#else
#define MAXEXPORT_API extern "C" __declspec(dllimport)
#endif


MAXEXPORT_API int LibNumberClasses();
MAXEXPORT_API ClassDesc* LibClassDesc( int i );
MAXEXPORT_API const TCHAR* LibDescription();
MAXEXPORT_API ULONG LibVersion();


#endif // _MAXEXPORT_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
