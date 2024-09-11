#ifndef _MYGEOMETRY_H
#define _MYGEOMETRY_H


#include "ExportVertex.h"
#include "ExportTriangle.h"
#include "ExportWeightMap.h"
#include <lang/Array.h>
#include <lang/String.h>
#include <math/float4.h>


class MyGeometry :
	public lang::Object
{
public:
	NS(lang,Array)<P(ExportVertex)>		vertexlist;
	NS(lang,Array)<P(ExportTriangle)>		trianglelist;
	NS(lang,Array)<P(ExportWeightMap)>		weightmaplist;
	int									uvwlayers;

	explicit MyGeometry( const NS(lang,String)& name );
	~MyGeometry();

	// updates ExportVertex::connectedFaces list
	void	connectVerticesToFaces();

	int		vertices() const;

	void	getVertexWeights( ExportVertex* v, NS(math,float4)* outweights, NS(math,float4)* outbones ) const;

	const NS(lang,String)&		name() const;

private:
	struct VertexWeight
	{
		float	weight;
		int		boneindex;

		VertexWeight() : weight(0), boneindex(0) {}
	};

	NS(lang,String)	m_name;

	MyGeometry( const MyGeometry& );
	MyGeometry& operator=( const MyGeometry& );
};


#endif // _MYGEOMETRY_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
