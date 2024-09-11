#ifndef _MYSCENEEXPORTCLASSDESC_H
#define _MYSCENEEXPORTCLASSDESC_H


class MySceneExportClassDesc : 
	public ClassDesc
{
public:
	HINSTANCE instance;

	MySceneExportClassDesc();

	int				IsPublic();
	void*			Create( BOOL loading=FALSE );
	const TCHAR*	ClassName();
	SClass_ID		SuperClassID();
	Class_ID		ClassID();
	const TCHAR*	Category();
};


#endif // _MYSCENEEXPORTCLASSDESC_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
