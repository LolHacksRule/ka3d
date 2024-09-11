#include "StdAfx.h"
#include "MyPrimitive.h"
#include "MyMaterial.h"
#include "MyGeometry.h"
#include "NVMeshMenderEx.h"
#include "maxexportconfig.h"
#include <gr/VertexFormat.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/System.h>
#include <lang/Integer.h>
#include <lang/Exception.h>
#include <math/float.h>
#include <math/toString.h>
#include <algorithm>
#include <config.h>


USING_NAMESPACE(gr)
USING_NAMESPACE(lang)
USING_NAMESPACE(math)


static void print( const Array<float4>& v, const char* name )
{
	/*Debug::printf( "Vertex data '%s':\n", name );
	for ( int i = 0 ; i < v.size() ; ++i )
		Debug::printf( "[%d] = %g %g %g %g\n", i, v[i].x, v[i].y, v[i].z, v[i].w );*/
}


MyPrimitive::MyPrimitive( MyMaterial* mat, MyGeometry* geom, float4 scalebias ) :
	m_mat( mat ),
	m_geom( geom ),
	m_scalebias( scalebias )
{
	int polycount = geom->trianglelist.size();

	Array<int> faces;
	faces.resize( polycount );
	for ( int i = 0 ; i < polycount ; ++i )
		faces[i] = i;
	create( mat, geom, faces );
}

MyPrimitive::MyPrimitive( MyMaterial* mat, MyGeometry* geom, const Array<int>& faces, float4 scalebias ) :
	m_mat( mat ),
	m_geom( geom ),
	m_scalebias( scalebias )
{
	create( mat, geom, faces );
}

MyPrimitive::~MyPrimitive()
{
}

void MyPrimitive::create( MyMaterial* mat, MyGeometry* geom, const Array<int>& faces )
{
	assert( 0 != mat );
	assert( 0 != geom );

	//Debug::printf( "DEBUG: Extracting primitive \"%s\" from \"%s\"\n", mat->name().c_str(), geom->name().c_str() );

	// get triangles
	Array<ExportTriangle*> trianglelist( faces.size() );
	trianglelist.clear();
	for ( int i = 0 ; i < faces.size() ; ++i )
	{
		int fi = faces[i];
		assert( fi >= 0 && fi < geom->trianglelist.size() );

		ExportTriangle* tri = geom->trianglelist[fi];
		assert( tri != 0 );

		if ( tri->material == mat )
		{
			trianglelist.add( tri );

			// set vertex defaults
			for ( int k = 0 ; k < 3 ; ++k )
			{
				for ( int n = 0 ; n < ExportVertex::MAX_UVW ; ++n )
					tri->verts[k]->uvw[n] = tri->uvw[n][k];
				assert( tri->verts[k] != 0 );
				tri->verts[k]->normal = tri->getVertexNormal(k);
				tri->verts[k]->tangent = tri->getVertexTangent(k);
				tri->verts[k]->color = tri->colors[k];
				tri->verts[k]->next = 0;
				tri->splitVerts[k] = 0;
			}
		}
	}

	// tangent space needed?
	const bool tangentspace = mat->hasTexture("NORMALMAP");

	// split vertex UV and vertex normal discontinuities
	for ( int i = 0 ; i < trianglelist.size() ; ++i )
	{
		ExportTriangle* tri = trianglelist[i];
		for ( int k = 0 ; k < 3 ; ++k )
		{
			float3 vertnormal = tri->getVertexNormal( k );
			float3 verttangent = tri->getVertexTangent( k );
			float3 uvw[ExportVertex::MAX_UVW];
			for ( int n = 0 ; n < ExportVertex::MAX_UVW ; ++n )
				uvw[n] = tri->uvw[n][k];

			P(ExportVertex) vert = tri->verts[k];
			for ( ; vert != 0 ; vert = vert->next )
			{
				// uvw mismatch?
				bool split = false;
				for ( int n = 0 ; n < ExportVertex::MAX_UVW ; ++n )
				{
					if ( (uvw[n]-vert->uvw[n]).length() > 0.001f )
					{
						split = true;
						break;
					}
				}
				if ( split )
					continue;

				// normal mismatch?
				float cosa = clamp( dot(vertnormal, vert->normal), 0.f, 1.f );
				float angle = Math::acos( cosa );
				if ( angle > Math::toRadians(10.f) )
					continue;

				// tangent mismatch?
				if ( tangentspace )
				{
					cosa = clamp( dot(verttangent, vert->tangent), 0.f, 1.f );
					angle = Math::acos( cosa );
					if ( angle > Math::toRadians(10.f) )
						continue;
				}

				// vert accepted
				break;
			}

			// split needed?
			if ( !vert )
			{
				vert = new ExportVertex( *tri->verts[k] );
				
				vert->normal = vertnormal;
				vert->tangent = verttangent;
				for ( int n = 0 ; n < ExportVertex::MAX_UVW ; ++n )
					vert->uvw[n] = uvw[n];

				vert->next = tri->verts[k]->next;
				tri->verts[k]->next = vert;
			}

			tri->splitVerts[k] = vert;
		}
	}

	// build vertex list
	Array<ExportVertex*> vertexlist;
	for ( int i = 0 ; i < trianglelist.size() ; ++i )
	{
		ExportTriangle* tri = trianglelist[i];
		for ( int k = 0 ; k < 3 ; ++k )
			vertexlist.add( tri->splitVerts[k] );
	}
	std::sort( vertexlist.begin(), vertexlist.end() );
	vertexlist.resize( std::unique( vertexlist.begin(), vertexlist.end() ) - vertexlist.begin() );

	// extract face data
	Array<int> indices( trianglelist.size()*3 );
	indices.clear();
	for ( int i = 0 ; i < trianglelist.size() ; ++i )
	{
		ExportTriangle* tri = trianglelist[i];
		for ( int k = 0 ; k < 3 ; ++k )
		{
			int ix = vertexlist.indexOf( tri->splitVerts[k] );
			assert( ix != -1 );
			indices.add( ix );
		}
	}

	// setup vertex format
	VertexFormat vf;
	switch ( getExportPlatform() )
	{
	case EXPORTPLATFORM_EGL:
		vf.addPosition( VertexFormat::DF_V3_16 );
		vf.addNormal( VertexFormat::DF_V3_8 );
		vf.addTextureCoordinate( VertexFormat::DF_V2_16 );
		break;

	case EXPORTPLATFORM_SW:
		vf.addPosition( VertexFormat::DF_V3_16 );
		vf.addTextureCoordinate( VertexFormat::DF_V2_16 );
		break;

	case EXPORTPLATFORM_N3D:
		vf.addPosition( VertexFormat::DF_V3_16 );
		vf.addTextureCoordinate( VertexFormat::DF_V2_16 );
		break;

	default: // PC
		vf.addPosition();
		vf.addNormal();
		vf.addTextureCoordinate( VertexFormat::DF_V2_32 );
		if ( mat->flags & MyMaterial::SHADER_LIGHTMAPPING )
			vf.addTextureCoordinate( VertexFormat::DF_V2_32 );
		if ( geom->weightmaplist.size() > 0 )
			vf.addWeights( VertexFormat::DF_V2_32, VertexFormat::DF_V3_32 );
		break;
	}

	// extract vertex data
	const int				vertices = vertexlist.size();
	Array<float4>			vpos( vertices );
	Array<float4>			vnormals( vertices );
	Array<float4>			vcolors( vertices );
	Array< Array<float4> >	vtexcoords( vf.textureCoordinates() );
	Array<float4>			vweights;
	Array<float4>			vbones;

	for ( int k = 0 ; k < vf.textureCoordinates() ; ++k )
		vtexcoords[k].resize( vertices, float4(0,0,0,0) );

	if ( vf.hasData(VertexFormat::DT_BONEWEIGHTS) )
	{
		vweights.resize( vertices, float4(0,0,0,0) );
		vbones.resize( vertices, float4(0,0,0,0) );
	}
	
	for ( int i = 0 ; i < vertexlist.size() ; ++i )
	{
		ExportVertex* v = vertexlist[i];
		
		vpos[i] = float4( v->pos, 1.f );
		vnormals[i] = float4( v->normal, 0.f );
		vcolors[i] = float4( v->color );

		for ( int k = 0 ; k < vf.textureCoordinates() ; ++k )
			vtexcoords[k][i] = float4( v->uvw[k], 0.f );

		if ( vf.hasData(VertexFormat::DT_BONEWEIGHTS) )
		{
			geom->getVertexWeights( v, &vweights[i], &vbones[i] );
		}
	}

	// compute vertex tangents if bump map channel used in material
	Array<float4> vtans;
	if ( tangentspace )
	{
		vf.addTangent( VertexFormat::DF_V3_32 );

		vtans.clear();
		vnormals.clear();
		for ( int i = 0 ; i < vertexlist.size() ; ++i )
		{
			vtans.add( float4(vertexlist[i]->tangent,0) );
			vnormals.add( float4(vertexlist[i]->normal,0) );
		}
	}

	print( vpos, "vpos" );
	print( vnormals, "vnormals" );
	print( vcolors, "vcolors" );
	for ( int i = 0 ; i < vtexcoords.size() ; ++i )
		print( vtexcoords[i], "vtexcoords" );
	print( vweights, "vweights" );
	print( vbones, "vbones" );
	print( vtans, "vtans" );

	if ( vertices >= 0x10000 )
		throwError( Exception( Format("Mesh \"{0}\" has too many polygons using the same material. Limit is 65535 polygons/material. Please assign two different materials for the same mesh (using Multi-Sub Material). Sorry for inconvenience.", geom->name()) ) );
	setFormat( vf, vertices, indices.size() );
	setIndices( 0, indices.begin(), indices.size() );
	
	setPackedVertexPositions( vpos.begin(), vertices );
	
	if ( vf.hasData(VertexFormat::DT_NORMAL) )
		setPackedVertexNormals( vnormals.begin(), vertices );

	if ( vf.hasData(VertexFormat::DT_DIFFUSE) )
	{
		for ( int i = 0 ; i < vertices ; ++i )
		{
			float4 v = clamp( vcolors[i] * 255.f, float4(0,0,0,0), float4(255,255,255,255) );
			setVertexDiffuseColors( i, &v, 1 );
		}
	}

	if ( vtexcoords.size() > 0 )
	{
		switch ( getExportPlatform() )
		{
		case EXPORTPLATFORM_EGL:{
			Array<float4>& texcoords = vtexcoords[0];
			for ( int i = 0 ; i < texcoords.size() ; ++i )
				texcoords[i] = m_mat->applyTiling( 0, texcoords[i] );

			float4 scalebias;
			Array<int16_t> data( vertices*3 );
			assert( vtexcoords[0].size() == vertices );
			get_V3_16( &vtexcoords[0][0], vertices, &data[0], &scalebias, m_scalebias );
			setVertexData( VertexFormat::DT_TEX0, 0, &data[0], VertexFormat::DF_V3_16, vertices );
			setVertexTextureCoordinateScaleBias( scalebias );
			break;}

		case EXPORTPLATFORM_SW:{
			Array<float4>& texcoords = vtexcoords[0];
			float4 uvscale = m_mat->getUVScale(0);
			for ( int i = 0 ; i < texcoords.size() ; ++i )
			{
				texcoords[i] = m_mat->applyTiling( 0, texcoords[i] );
				if ( texcoords[i].x < 0.f || texcoords[i].y < 0.f )
					throwError( Exception( Format("Mesh \"{0}\" has negative texture coordinates (=tiling up or left), not supported on software rendering", geom->name()) ) );
			}

			float4 scalebias;
			Array<int16_t> data( vertices*2 );
			assert( vtexcoords[0].size() == vertices );
			get_V2_4_12( &vtexcoords[0][0], vertices, &data[0], &scalebias );
			setVertexData( VertexFormat::DT_TEX0, 0, &data[0], VertexFormat::DF_V2_16, vertices );
			setVertexTextureCoordinateScaleBias( scalebias );
			break;}

		case EXPORTPLATFORM_N3D:{
			Array<float4>& texcoords = vtexcoords[0];
			for ( int i = 0 ; i < texcoords.size() ; ++i )
			{
				texcoords[i] = m_mat->applyTiling( 0, texcoords[i] );
				if ( texcoords[i].x < 0.f || texcoords[i].y < 0.f )
					throwError( Exception( Format("Mesh \"{0}\" has negative texture coordinates (=tiling up or left), not supported on Nokia N3D", geom->name()) ) );
			}

			float4 scalebias;
			Array<int16_t> data( vertices*2 );
			assert( texcoords.size() == vertices );
			get_V2_4_12( &texcoords[0], vertices, &data[0], &scalebias );
			setVertexData( VertexFormat::DT_TEX0, 0, &data[0], VertexFormat::DF_V2_16, vertices );
			setVertexTextureCoordinateScaleBias( scalebias );

			/*Debug::printf( "Dumping texture coordinates:\n" );
			for ( int i = 0 ; i < texcoords.size() ; ++i )
				Debug::printf( "uv=%g %g, data=%d %d, data*scalebias=%g %g\n", texcoords[i].x, texcoords[i].y, data[i*2], data[i*2+1], float(data[i*2])*scalebias.x+scalebias.y, float(data[i*2+1])*scalebias.x+scalebias.z );
			*/
			break;}

		default:{
			for ( int i = 0 ; i < vtexcoords.size() ; ++i )
			{
				for ( int k = 0 ; k < vtexcoords[i].size() ; ++k )
				{
					float4 uv = m_mat->applyTiling( i, vtexcoords[i][k] );
					setVertexTextureCoordinates( k, i, &uv, 1 );
				}
			}
			break;}
		}
	}

	if ( vf.hasData(VertexFormat::DT_BONEWEIGHTS) )
		setVertexBoneWeights( 0, vweights.begin(), vertices );
	if ( vf.hasData(VertexFormat::DT_BONEINDICES) )
		setVertexBoneIndices( 0, vbones.begin(), vertices );
	if ( vf.hasData(VertexFormat::DT_TANGENT) )
		setVertexTangents( 0, vtans.begin(), vertices );

	packBones();

	Debug::printf( "Mesh \"%s\" primitive \"%s\" uses vertex format %s\n", geom->name().c_str(), mat->name().c_str(), vf.toString().c_str() );
}

void MyPrimitive::setPackedVertexPositions( const float4* v, int count )
{
	float4 scalebias( 1.f, 0,0,0 );

	switch ( getExportPlatform() )
	{
	case EXPORTPLATFORM_EGL:{
		Array<int16_t> data( count*3 );
		get_V3_16( v, count, &data[0], &scalebias, m_scalebias );
		setVertexData( VertexFormat::DT_POSITION, 0, &data[0], VertexFormat::DF_V3_16, count );
		break;}

	case EXPORTPLATFORM_N3D:
	case EXPORTPLATFORM_SW:{
		Array<int16_t> data( count*3 );
		get_V3_16( v, count, &data[0], &scalebias, m_scalebias );
		setVertexData( VertexFormat::DT_POSITION, 0, &data[0], VertexFormat::DF_V3_16, count );
		break;}

	default:
		setVertexPositions( 0, v, count );
		break;
	}

	setVertexPositionScaleBias( scalebias );
}

void MyPrimitive::setPackedVertexNormals( const float4* v, int count )
{
	switch ( getExportPlatform() )
	{
	case EXPORTPLATFORM_EGL:{
		float4 scalebias( 1.f, 0,0,0 );
		Array<int8_t> data( count*3 );
		get_V3_8_Normal( v, count, &data[0], &scalebias );
		setVertexData( VertexFormat::DT_NORMAL, 0, &data[0], VertexFormat::DF_V3_8, count );
		break;}

	default:
		setVertexNormals( 0, v, count );
		break;
	}
}

MyMaterial* MyPrimitive::material() const
{
	return static_cast<MyMaterial*>(shader());
}

gr::Shader* MyPrimitive::shader() const
{
	return m_mat;
}

void MyPrimitive::setShader( gr::Shader* )
{
	assert( false ); // shouldnt be set
}

void MyPrimitive::getBonesUsedByFaces( const Array<int>& faces, Array<int>& bones )
{
	assert( usedBones() > 0 );

	for ( int i = 0 ; i < faces.size() ; ++i )
	{
		int face = faces[i];
		int ind[3];
		assert( face*3 >= 0 && face*3 < indices() );
		getIndices( face*3, ind, 3 );

		for ( int k = 0 ; k < 3 ; ++k )
		{
			float4 boneind;
			getVertexBoneIndices( ind[k], &boneind, 1 );

			for ( int n = 0 ; n < 4 ; ++n )
			{
				int boneix = (int)boneind[n];
				bones.add( boneix );
			}
		}
	}

	std::sort( bones.begin(), bones.end() );
	bones.resize( std::unique(bones.begin(),bones.end())-bones.begin() );
}

void MyPrimitive::getVerticesUsedByFaces( const Array<int>& faces, Array<int>& vertices )
{
	for ( int i = 0 ; i < faces.size() ; ++i )
	{
		int face = faces[i];
		int ind[3];
		assert( face*3 >= 0 && face*3 < indices() );
		getIndices( face*3, ind, 3 );

		for ( int k = 0 ; k < 3 ; ++k )
			vertices.add( ind[k] );
	}

	std::sort( vertices.begin(), vertices.end() );
	vertices.resize( std::unique(vertices.begin(),vertices.end())-vertices.begin() );
}

void MyPrimitive::splitPrimitive( int maxbones,
	P(MyPrimitive)* out1, P(MyPrimitive)* out2 )
{
	Array<int> faceset1;
	Array<int> faceset2;
	findSplit( maxbones, faceset1, faceset2 );

	*out1 = new MyPrimitive( m_mat, m_geom, faceset1, m_scalebias );
	*out2 = new MyPrimitive( m_mat, m_geom, faceset2, m_scalebias );
}

MyPrimitive::PrimType MyPrimitive::type() const
{
	return PRIM_TRI;
}

void MyPrimitive::get_V3_16( const float4* v, int count, int16_t* out, float4* scalebiasout, float4 scalebias )
{
	assert( v != 0 );

	const int f = 16383;
	int16_t* const out0 = out;
	for ( int i = 0 ; i < count ; ++i )
	{
		for ( int k = 0 ; k < 3 ; ++k )
		{
			float x = (v[i][k] - scalebias[1+k]);
			if ( scalebias.x != 1.f )
				x /= scalebias.x;
			assert( x >= 0.f && x <= 1.f );
			
			int xi = (int)(x * float(f));
			if ( xi < 0 )
				xi = 0;
			else if ( xi > f )
				xi = f;
			
			*out++ = (int16_t)xi;
		}
	}

	// WORKAROUND: some EGL implementations support only 9bit coords
	if ( EXPORTPLATFORM_EGL == getExportPlatform() )
	{
		const int reducebits = 5;
		const int count3 = count*3;
		for ( int i = 0 ; i < count3 ; ++i )
			out0[i] >>= reducebits;
		scalebias.x *= (float)(1<<reducebits);
	}
	
	scalebias.x *= 1.f/float(f);
	*scalebiasout = scalebias;
}

void MyPrimitive::get_V2_4_12( const float4* v, int count, int16_t* out, float4* scalebiasout )
{
	assert( v != 0 );

	const int fixedbits = 12;
	const float fixedscalar = float(1<<fixedbits);
	float4 scalebias = float4( 1.f/fixedscalar, 0,0,0 );

	int16_t* const out0 = out;
	for ( int i = 0 ; i < count ; ++i )
	{
		for ( int k = 0 ; k < 2 ; ++k )
		{
			float x = v[i][k] * fixedscalar;
			
			int xi = (int)(x);
			if ( xi > 32767 )
				xi = 32767;
			else if ( xi < -32767 )
				xi = -32767;
			
			*out++ = (int16_t)xi;
		}
	}
	
	*scalebiasout = scalebias;
}

void MyPrimitive::get_V3_8( const float4* v, int count, int8_t* out, float4* scalebiasout, float4 scalebias )
{
	assert( v != 0 );

	const int f = 127;
	int8_t* const out0 = out;
	for ( int i = 0 ; i < count ; ++i )
	{
		for ( int k = 0 ; k < 3 ; ++k )
		{
			float x = (v[i][k] - scalebias[1+k]);
			if ( scalebias.x != 1.f )
				x /= scalebias.x;
			assert( x >= 0.f && x <= 1.f );
			
			int xi = (int)(x * float(f));
			if ( xi < 0 )
				xi = 0;
			else if ( xi > f )
				xi = f;
			
			*out++ = (int8_t)xi;
		}
	}
	
	scalebias.x *= 1.f/float(f);
	*scalebiasout = scalebias;
}

void MyPrimitive::get_V3_8_Normal( const float4* v, int count, int8_t* out, float4* scalebiasout )
{
	float scale = 2.f/255.f;
	float invscale = 255.f/2.f;
	float4 scalebias = float4(scale, 1.f/255.f,1.f/255.f,1.f/255.f );

	for ( int i = 0 ; i < count ; ++i )
	{
		for ( int k = 0 ; k < 3 ; ++k )
		{
			float x0 = v[i][k];
			assert( x0 >= -1.f && x0 <= 1.f );

			float x = (x0 - scalebias[1+k]) * invscale;
			int xi = (int)x;
			if ( xi < -128 )
				xi = -128;
			else if ( xi > 127 )
				xi = 127;

			*out++ = (int8_t)xi;
		}
	}

	*scalebiasout = scalebias;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
