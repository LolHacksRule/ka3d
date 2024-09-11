#include "StdAfx.h"
#include "MyGeometry.h"
#include <lang/Float.h>
#include <lang/String.h>
#include <lang/Exception.h>
#include <math/float3.h>
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


MyGeometry::MyGeometry( const String& name ) :
	uvwlayers( 0 ),
	m_name( name )
{
}

MyGeometry::~MyGeometry()
{
}

void MyGeometry::getVertexWeights( ExportVertex* v, float4* outweights, float4* outbones ) const
{
	const int maxweights = 3;
	VertexWeight weights[maxweights];

	// note: assumes that weightmaplist order matches MyMesh addBone order
	int count = 0;
	for ( int i = 0 ; i < weightmaplist.size() ; ++i )
	{
		float w = weightmaplist[i]->getWeight(v);
		if ( w > 0.f )
		{
			int slot = 0;
			while ( slot < maxweights && weights[slot].weight >= w )
				++slot;

			if ( slot < maxweights )
			{
				++count;
				if ( count > maxweights )
					count = maxweights;
				for ( int k = slot+1 ; k < count ; ++k )
					weights[k] = weights[k-1];
				
				weights[slot].weight = w;
				weights[slot].boneindex = i;
			}
		}
	}

	for ( int i = count ; i < maxweights ; ++i )
	{
		weights[i].weight = 0.f;
		weights[i].boneindex = 0;
	}

	assert( weights[0].weight >= weights[1].weight );
	assert( weights[1].weight >= weights[2].weight );
	assert( maxweights <= 4 );

	float sumw = 0.f;
	for ( int i = 0 ; i < maxweights ; ++i )
		sumw += weights[i].weight;
	float invsumw = (sumw >= Float::MIN_VALUE ? 1.f / sumw : 0.f);

	*outweights = float4(0,0,0,0);
	*outbones = float4(0,0,0,0);
	for ( int i = 0 ; i < maxweights ; ++i )
	{
		(*outweights)[i] = weights[i].weight * invsumw;
		(*outbones)[i] = (float)weights[i].boneindex;
		assert( (*outbones)[i] >= 0 && (*outbones)[i] < weightmaplist.size() );
	}
}

int MyGeometry::vertices() const
{
	return vertexlist.size();
}

const String& MyGeometry::name() const
{
	return m_name;
}

void MyGeometry::connectVerticesToFaces()
{
	for ( int i = 0 ; i < vertexlist.size() ; ++i )
	{
		assert( vertexlist[i] != 0 );
		vertexlist[i]->connectedFaces.clear();
	}
	for ( int i = 0 ; i < trianglelist.size() ; ++i )
	{
		assert( trianglelist[i] != 0 );
		for ( int k = 0 ; k < 3 ; ++k )
		{
			assert( trianglelist[i]->verts[k] != 0 );
			trianglelist[i]->verts[k]->connectedFaces.add( trianglelist[i] );
		}
	}
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
