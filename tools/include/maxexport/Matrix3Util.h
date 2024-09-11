#ifndef _MAXTRIX3UTIL_H
#define _MAXTRIX3UTIL_H


#include <math/float3x4.h>


/** 
 * Utilities for Matrix3 handling.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class Matrix3Util
{
public:
	/** Transform conversion matrix. */ 
	static const Matrix3	CONVTM;

	/** Returns transform in left-handed coordinate system. */
	static NS(math,float3x4)	toLH( const Matrix3& tm );
};


#endif // _MAXTRIX3UTIL_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
