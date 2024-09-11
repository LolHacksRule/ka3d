#include "StdAfx.h"
#include "MySceneExportClassDesc.h"
#include "MySceneExport.h"
#include <config.h>


MySceneExportClassDesc::MySceneExportClassDesc()
{
	instance = 0;
}

int MySceneExportClassDesc::IsPublic()
{
	return TRUE;
}

void* MySceneExportClassDesc::Create( BOOL loading )
{
	return new MySceneExport( instance );
}

const TCHAR* MySceneExportClassDesc::ClassName()
{
	return _T("SceExport");
}

SClass_ID MySceneExportClassDesc::SuperClassID()
{
	return SCENE_EXPORT_CLASS_ID;
}

Class_ID MySceneExportClassDesc::ClassID()
{
	return Class_ID(0x6ba369a6, 0x503755a6);
}

const TCHAR* MySceneExportClassDesc::Category()
{
	return "SgExportCategory";
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
