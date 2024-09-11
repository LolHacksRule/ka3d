#include "StdAfx.h"
#include "ExportWeightMap.h"
#include <config.h>


USING_NAMESPACE(math)


ExportWeightMap::ExportWeightMap( const NS(lang,String)& name ) : 
	m_name( name )
{
}

void ExportWeightMap::addWeight( ExportVertex* v, float w )
{
	VertexWeight vw;
	vw.vertex = v;
	vw.weight = w;
	m_vertexWeights.add( vw );
}

float ExportWeightMap::getWeight( ExportVertex* v ) const
{
	for ( int i = 0 ; i < m_vertexWeights.size() ; ++i )
	{
		for ( ExportVertex* vert = m_vertexWeights[i].vertex ; vert != 0 ; vert = vert->next )
		{
			if ( vert == v )
				return m_vertexWeights[i].weight;
		}
	}
	return 0.f;
}
