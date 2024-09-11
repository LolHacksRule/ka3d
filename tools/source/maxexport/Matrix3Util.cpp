#include "StdAfx.h"
#include "Matrix3Util.h"
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


const Matrix3 Matrix3Util::CONVTM( Point3(1,0,0), Point3(0,1,0), Point3(0,0,-1), Point3(0,0,0) );


float3x4 Matrix3Util::toLH( const Matrix3& tm )
{
	Matrix3 m = CONVTM * tm * CONVTM;
	float3x4 out;
	out.setColumn( 0, float3(m.GetRow(0).x,m.GetRow(0).y,m.GetRow(0).z) );
	out.setColumn( 1, float3(m.GetRow(1).x,m.GetRow(1).y,m.GetRow(1).z) );
	out.setColumn( 2, float3(m.GetRow(2).x,m.GetRow(2).y,m.GetRow(2).z) );
	out.setColumn( 3, float3(m.GetRow(3).x,m.GetRow(3).y,m.GetRow(3).z) );
	return out;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
