#include "StdAfx.h"
#include "StdMatUtil.h"
#include <config.h>


BitmapTex* StdMatUtil::getStdMatBitmapTex( StdMat* stdmat, int id )
{
	StdMat2* stdmat2 = 0;
	int channel = id;
	if ( stdmat->SupportsShaders() )
	{
		stdmat2 = static_cast<StdMat2*>( stdmat );
		channel = stdmat2->StdIDToChannel( id );
	}

	if ( stdmat->MapEnabled(channel) )
	{
		Texmap*	tex	= stdmat->GetSubTexmap(channel);
		if ( tex && tex->ClassID() == Class_ID(BMTEX_CLASS_ID,0) &&
			(!stdmat2 || 2 == stdmat2->GetMapState(channel)) )
		{
			BitmapTex* bmptex = static_cast<BitmapTex*>(tex);
			if ( bmptex->GetMapName() )
			{
				return bmptex;
			}
		}
	}
	return 0;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
