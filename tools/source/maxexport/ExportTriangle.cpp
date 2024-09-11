#include "StdAfx.h"
#include "ExportTriangle.h"
#include <lang/Math.h>
#include <lang/Float.h>
#include <math/float.h>
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


ExportTriangle::ExportTriangle() :
	material( 0 ),
	smoothing( -1 )
{
	for ( int i = 0 ; i < 3 ; ++i )
	{
		colors[i] = float4(1,1,1,1);
		for ( int k = 0 ; k < MAX_UVW ; ++k )
			uvw[k][i] = float3(0,0,0);
	}
}

float3 ExportTriangle::faceNormal() const
{
	assert( verts[0] != 0 );
	assert( verts[1] != 0 );
	assert( verts[2] != 0 );
	return cross( verts[1]->pos-verts[0]->pos, verts[2]->pos-verts[0]->pos );
}

bool ExportTriangle::isDegenerate() const
{
	int k = 2;
	for ( int i = 0 ; i < 3 ; k = i++ )
	{
		float3 edge = verts[i]->pos - verts[k]->pos;
		if ( edge.length() < 1e-6f )
			return true;
	}

	float3 facenormal = cross( verts[1]->pos-verts[0]->pos, verts[2]->pos-verts[0]->pos );
	if ( facenormal.lengthSquared() < Float::MIN_VALUE )
		return true;

	return false;
}

float3 ExportTriangle::getVertexNormal( int i ) const
{
	assert( i >= 0 && i < 3 );

	float3 normal(0,0,0);
	ExportVertex* v = verts[i];
	assert( v != 0 );
	assert( v->connectedFaces.size() > 0 ); // sounds like connectVerticesToFaces was not called
	for ( int k = 0 ; k < v->connectedFaces.size() ; ++k )
	{
		ExportTriangle* face = v->connectedFaces[k];
		if ( face == this || 0 != (face->smoothing & smoothing) )
			normal += face->faceNormal();
	}

	return normalize0( normal );
}

float3 ExportTriangle::getVertexTangent( int i ) const
{
	assert( i >= 0 && i < 3 );

	ExportVertex* v = verts[i];
	assert( v != 0 );
	assert( v->connectedFaces.size() > 0 ); // sounds like connectVerticesToFaces was not called

	float3 s(0,0,0);
	float3 t(0,0,0);
	float3 facenormal = faceNormal();

	for ( int k = 0 ; k < v->connectedFaces.size() ; ++k )
	{
		ExportTriangle* face = v->connectedFaces[k];

		//float cosa = saturate( dot( facenormal, face->faceNormal() ) );
		//float angle = Math::acos( cosa );

		if ( face == this || 0 != (face->smoothing & smoothing) )
		//if ( face == this || angle < Math::PI*.25f )
		{
			const float3 v0 = face->verts[0]->pos;
			const float3 v1 = face->verts[1]->pos;
			const float3 v2 = face->verts[2]->pos;
			float3 s0( 0,0,0 );
			float3 t0( 0,0,0 );

			// rotated UVs
			float U0 = face->uvw[0][0].x;
			float V0 = face->uvw[0][0].y;
			rotate2DPoint( U0, V0, -Math::PI*.5f );
			float U1 = face->uvw[0][1].x;
			float V1 = face->uvw[0][1].y;
			rotate2DPoint( U1, V1, -Math::PI*.5f );
			float U2 = face->uvw[0][2].x;
			float V2 = face->uvw[0][2].y;
			rotate2DPoint( U2, V2, -Math::PI*.5f );

			float3 P = v1 - v0;
			float3 Q = v2 - v0;
			float s1 = U1 - U0;
			float t1 = V1 - V0;
			float s2 = U2 - U0;
			float t2 = V2 - V0;
	
			float tmp = 1.0f;
			if ( Math::abs(s1*t2 - s2*t1) >= 0.0001f )
				tmp /= s1*t2 - s2*t1;
			
			t0.x = (t2*P.x - t1*Q.x);
			t0.y = (t2*P.y - t1*Q.y);
			t0.z = (t2*P.z - t1*Q.z);
			t0 *= tmp;

			s0.x = (s1*Q.x - s2*P.x);
			s0.y = (s1*Q.y - s2*P.y);
			s0.z = (s1*Q.z - s2*P.z);
			s0 *= tmp;

			s += normalize0(s0);
			t += normalize0(t0);
		}
	}

	t = cross( s, getVertexNormal(i) );
	return normalize0( t );
}

float3 ExportTriangle::getSmoothVertexNormal( int i ) const
{
	assert( i >= 0 && i < 3 );

	float3 normal(0,0,0);
	ExportVertex* v = verts[i];
	assert( v != 0 );
	assert( v->connectedFaces.size() > 0 ); // sounds like connectVerticesToFaces was not called
	for ( int k = 0 ; k < v->connectedFaces.size() ; ++k )
	{
		ExportTriangle* face = v->connectedFaces[k];
		normal += face->faceNormal();
	}

	return normalize0( normal );
}

void ExportTriangle::rotate2DPoint( float &X, float &Y, float A )
{	
	float NewX,NewY,TempX,TempY;

	TempX = X;
	TempY = Y;

	NewX = TempX * (float)cos(-A) - TempY * (float)sin(-A);
	NewY = TempX * (float)sin(-A) + TempY * (float)cos(-A);

	X = NewX;
	Y = NewY;

}
