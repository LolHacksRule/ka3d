#ifndef _MTLUTIL_H
#define _MTLUTIL_H


#include <lang/Array.h>


class MtlUtil
{
public:
	class MtlNameLess
	{
	public:
		bool operator()( Mtl* a, Mtl* b ) const				{return strcmp(a->GetName(),b->GetName()) < 0;}
	};

	static bool			hasFaceMtl( Face& face, Mtl* nodemtl );
	static Mtl*			getFaceMtl( Face& face, Mtl* nodemtl );
	static BitmapTex*	getBitmapTex( StdMat* stdmtl, int chn );
	static bool			isDynamicDxMaterial( Mtl* newmtl );
};


#endif // _MTLUTIL_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
