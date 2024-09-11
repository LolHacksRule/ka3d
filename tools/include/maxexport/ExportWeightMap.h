#ifndef _EXPORTWEIGHTMAP_H
#define _EXPORTWEIGHTMAP_H


#include "ExportVertex.h"
#include <lang/Array.h>
#include <lang/String.h>
#include <lang/Object.h>
#include <math/float3.h>


class ExportWeightMap :
	public lang::Object
{
public:
	struct VertexWeight
	{
		P(ExportVertex)	vertex;
		float			weight;
	};

	explicit ExportWeightMap( const NS(lang,String)& name );

	void	addWeight( ExportVertex* v, float w );

	float	getWeight( ExportVertex* v ) const;

private:
	NS(lang,String)				m_name;
	NS(lang,Array)<VertexWeight>	m_vertexWeights;
};


#endif // _EXPORTWEIGHTMAP_H
