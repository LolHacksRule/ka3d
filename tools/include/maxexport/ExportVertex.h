#ifndef _EXPORTVERTEX_H
#define _EXPORTVERTEX_H


#include <lang/Array.h>
#include <lang/Object.h>
#include <math/float3.h>
#include <math/float4.h>


class ExportTriangle;


class ExportVertex :
	public lang::Object
{
public:
	enum UVWLayers
	{ 
		UVW_BASEMAP,
		UVW_LIGHTMAP,
		MAX_UVW
	};

	NS(math,float3)					pos;
	NS(lang,Array)<ExportTriangle*>	connectedFaces;

	// temps
	P(ExportVertex)		next;
	NS(math,float3)		uvw[MAX_UVW];
	NS(math,float3)		normal;
	NS(math,float3)		tangent;
	NS(math,float4)		color;

	ExportVertex();
};


#endif // _EXPORTVERTEX_H
