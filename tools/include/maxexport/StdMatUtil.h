#ifndef _STDMATUTIL_H
#define _STDMATUTIL_H


#include <stdmat.h>


/** 
 * Utilities for 3dsmax StdMat handling.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class StdMatUtil
{
public:
	/** 
	 * Returns Standard material Blinn shader bitmap texture (if any) from specified channel. 
	 * @param id Channel id: ID_DI, ID_RL, ...
	 * @return Bitmap texture of the channel or 0 if not any.
	 */
	static BitmapTex*	getStdMatBitmapTex( StdMat* stdmat, int id );
};


#endif // _STDMATUTIL_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
