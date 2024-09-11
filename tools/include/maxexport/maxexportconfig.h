#ifndef _MAXEXPORTCONFIG_H
#define _MAXEXPORTCONFIG_H


#include "maxversion.h"
#include <gr/Context.h>


#if (MAX_PRODUCT_VERSION_MAJOR>=6)
#define MAXEXPORT_HASDXMATERIAL
#endif

// WORKAROUND: some EGL implementations bug with >9bit pos coords (Hybrid Gerbera/Rasteroid)
#define WORKAROUND_EGL_9BIT_COORDS
// Just for testing
//#define WORKAROUND_EGL_FORCE_INT_VPOS_COORDS

enum ExportPlatform
{
	EXPORTPLATFORM_DX = NS(gr,Context)::PLATFORM_DX,
	EXPORTPLATFORM_EGL = NS(gr,Context)::PLATFORM_EGL,
	EXPORTPLATFORM_PSP = NS(gr,Context)::PLATFORM_PSP,
	EXPORTPLATFORM_SW = NS(gr,Context)::PLATFORM_SW,
	EXPORTPLATFORM_N3D = NS(gr,Context)::PLATFORM_N3D,
	EXPORTPLATFORM_COUNT
};

ExportPlatform getExportPlatform();
const char* getExportPlatformName( ExportPlatform platformid );


#endif // _MAXEXPORTCONFIG_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
