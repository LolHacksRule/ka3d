#ifndef _EXPORTTRIANGLE_H
#define _EXPORTTRIANGLE_H


#include "ExportVertex.h"
#include <lang/Object.h>
#include <math/float3.h>
#include <math/float4.h>


class MyMaterial;


class ExportTriangle :
	public lang::Object
{
public:
	enum { MAX_UVW = ExportVertex::MAX_UVW };

	P(ExportVertex)	verts[3];
	NS(math,float4)	colors[3];
	NS(math,float3)	uvw[MAX_UVW][3];
	MyMaterial*		material;
	int				smoothing;
	P(ExportVertex)	splitVerts[3];

	ExportTriangle();

	NS(math,float3)	faceNormal() const;

	NS(math,float3)	getVertexNormal( int i ) const;

	NS(math,float3)	getVertexTangent( int i ) const;

	NS(math,float3)	getSmoothVertexNormal( int i ) const;

	bool			isDegenerate() const;

private:
	static void		rotate2DPoint( float &X, float &Y, float A );
};


#endif // _EXPORTTRIANGLE_H
