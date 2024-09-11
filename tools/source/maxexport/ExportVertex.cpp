#include "StdAfx.h"
#include "ExportVertex.h"
#include <config.h>


USING_NAMESPACE(math)


ExportVertex::ExportVertex() :
	pos( 0, 0, 0 ),
	normal( 0, 0, 0 ),
	color( 1, 1, 1, 1 )
{
	for ( int i = 0 ; i < MAX_UVW ; ++i )
		uvw[i] = float3(0,0,0);
}
