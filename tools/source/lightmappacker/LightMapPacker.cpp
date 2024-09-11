#include <lightmappacker/LightMapPacker.h>
#include <lightmappacker/Rect.h>
#include <lightmappacker/LightMap.h>
#include <lightmappacker/ProgressInterface.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <lang/Debug.h>
#include <lang/Integer.h>
#include <lang/Exception.h>
#include <math/float.h>
#include <math/toString.h>
#include <algorithm>
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


namespace lightmappacker
{


LightMapPacker::LightMapPacker( float texeldensity, int mapwidth, int mapheight ) :
	m_texelDensity( texeldensity ),
	m_mapWidth( mapwidth ),
	m_mapHeight( mapheight ),
	m_progress( 0 )
{
}

LightMapPacker::~LightMapPacker()
{
}

void LightMapPacker::addTriangle( 
	int mesh, int triangle, int material,
	const float3& v1, const float3& v2, const float3& v3 )
{	
	assert( mesh < 1000000 );

	float3 e1 = v1 - v3;
	float3 e2 = v2 - v1;
	float3 e3 = v3 - v2;
	if ( ( e1.length() < 1e-9f ) ||
		( e2.length() < 1e-9f ) ||
		( e3.length() < 1e-9f ) )
	{
		throwError( Exception( Format("Added degenerate triangle (mesh={0}, tri={1}) to lightmap packer: {2} {3} {4}", mesh, triangle, toString(v1), toString(v2), toString(v3)) ) );
	}

	float3 facenormal = cross( e1, e2 );
	float facenormallen = facenormal.length();
	if ( facenormallen < Float::MIN_VALUE )
		throwError( Exception( Format("Added degenerate triangle, face normal invalid (mesh={0}, tri={1}) to lightmap packer: {2} {3} {4}", mesh, triangle, toString(v1), toString(v2), toString(v3)) ) );
	facenormal *= 1.f / facenormallen;

	Triangle tri;
	tri.triangle = triangle;
	tri.material = material;
	tri.mesh = mesh;
	tri.pos[0] = v1;
	tri.pos[1] = v2;
	tri.pos[2] = v3;
	tri.uv[2] = tri.uv[1] = tri.uv[0] = float2(0,0);
	tri.plane = float4( facenormal, -dot(v1,facenormal) );

	tri.mapUV( m_texelDensity );

	m_triangles.add( tri );
}

void LightMapPacker::build( ProgressInterface* progress )
{
	m_progress = progress;

	// setup
	assert( m_triangles.size() > 0 );
	m_combined.clear();
	m_lightmaps.clear();
	m_clippedTriangles.clear();
	if ( m_triangles.size() == 0 )
		return;

	setProgress( "Clipping lightmap triangles", .2f );
	clipTriangles();

	setProgress( "Building lightmap triangle groups", .4f );
	buildCombined();

	setProgress( "Clipping lightmap triangle groups", .6f );
	clipCombined();

	setProgress( "Packing lightmaps", .8f );
	packMaps();

	setProgress( "Cropping lightmaps", .95f );
	cropMaps();

	// compute efficiency measure
	float minarea = 0.f;
	float area = 0.f;
	for ( int i = 0 ; i < m_lightmaps.size() ; ++i )
		area += (float)m_lightmaps[i]->area();
	for ( int i = 0 ; i < m_triangles.size() ; ++i )
		minarea += m_triangles[i].areaUV();
	Debug::printf( "LightMapPacker: Total area in pixels is %g lightmaps, optimal is %g (%g%%)\n", area/float(m_mapWidth*m_mapHeight), minarea/float(m_mapWidth*m_mapHeight), minarea/area*100.f );

	m_progress = 0;
}

void LightMapPacker::buildCombined()
{
	// find highest mesh index
	int meshes = 0;
	for ( int i = 0 ; i < m_triangles.size() ; ++i )
		if ( m_triangles[i].mesh+1 > meshes )
			meshes = m_triangles[i].mesh+1;

	// combine triangles by signed primary axis, mesh index and material index
	Debug::printf( "LightMapPacker: Preparing combined polygons\n" );
	m_combined.resize( meshes*6 );
	for ( int i = 0 ; i < m_combined.size() ; ++i )
		m_combined[i] = new Combined();
	for ( int i = 0 ; i < m_triangles.size() ; ++i )
	{
		Triangle* tri = &m_triangles[i];
		int axis = tri->primaryAxis();
		if ( tri->primaryAxisSign() < 0 )
			axis += 3;
		assert( axis >= 0 && axis < 6 );

		int index = axis + tri->mesh*6;
		m_combined[index]->triangles.add( tri );
	}
	for ( int i = 0 ; i < m_combined.size() ; ++i )
		if ( m_combined[i]->triangles.size() == 0 )
			m_combined.remove( i-- );
	Debug::printf( "LightMapPacker: Number of combined polygons: %d\n", m_combined.size() );

	// sort combined triangles by area
	Debug::printf( "LightMapPacker: Sorting combined polygon triangles by area\n" );
	for ( int i = 0 ; i < m_combined.size() ; ++i )
		std::sort( m_combined[i]->triangles.begin(), m_combined[i]->triangles.end(), Triangle::SortByAreaUV() );

	/*
	// sort by plane distance
	Debug::printf( "LightMapPacker: Sorting %d triangles\n", m_triangles.size() );
	std::sort( m_triangles.begin(), m_triangles.end(), Triangle::SortByPlaneD() );

	// combine triangles that share same plane
	Debug::printf( "LightMapPacker: Preparing combined polygons\n" );
	m_combined.add( new Combined() );
	m_combined.last()->triangles.add( &m_triangles[0] );
	for ( int i = 1 ; i < m_triangles.size() ; ++i )
	{
		float diff = m_triangles[i].plane.w - m_combined.last()->triangles[0]->plane.w;
		if ( Math::abs(diff) > 0.01f )
		{
			m_combined.add( new Combined() );
		}
		m_combined.last()->triangles.add( &m_triangles[i] );
	}
	Debug::printf( "LightMapPacker: Number of combined polygons: %d\n", m_combined.size() );
	*/

	// split combined polygons, which have multiple materials / overlapping triangles
	Combined split;
	Debug::printf( "LightMapPacker: Splitting combined polygons with different materials etc.\n" );
	int overlaps = 0;
	for ( int i = 0 ; i < m_combined.size() ; ++i )
	{
		if ( ((i+1)&0x3FF) == 0 )
			setProgress( "Splitting combined polygons", float(i)/float(m_combined.size()) );

		split.triangles.clear();
		Combined& comb = *m_combined[i];

		for ( int k = 1 ; k < comb.triangles.size() ; ++k )
		{
			Triangle* tri = comb.triangles[k];
			if ( !comb.canBeCombined(tri) )
			{
				comb.triangles.remove( k-- );
				split.triangles.add( tri );
			}
		}
		if ( split.triangles.size() > 0 )
			m_combined.add( new Combined(split) );
	}

	Debug::printf( "LightMapPacker: Overlapped triangles %d\n", overlaps );
	Debug::printf( "LightMapPacker: Number of combined polygons after splitting is %d\n", m_combined.size() );
}

void LightMapPacker::clipTriangles()
{
	for ( int k = 0 ; k < m_triangles.size() ; ++k )
	{
		Triangle clip;
		int e1, e2;
		if ( m_triangles[k].clipByUV( (float)(m_mapWidth-1), (float)(m_mapHeight-1), &clip, &e1, &e2 ) )
		{
			m_triangles.add( clip );

			// store clip info
			ClippedTriangle cliptri;
			cliptri.triangle = m_triangles[k];
			cliptri.edge1 = e1;
			cliptri.edge2 = e2;
			m_clippedTriangles.add( cliptri );
			cliptri.triangle = clip;
			cliptri.edge1 = e2;
			cliptri.edge2 = e1;
			m_clippedTriangles.add( cliptri );

			--k;
		}
	}
	Debug::printf( "LightMapPacker: Clipped to %d triangles\n", m_triangles.size() );
}

void LightMapPacker::clipCombined()
{
	Combined clip;
	for ( int i = 0 ; i < m_combined.size() ; ++i )
	{
		clip.triangles.clear();
		if ( m_combined[i]->clipByUV( (float)(m_mapWidth-1), (float)(m_mapHeight-1), &clip ) )
		{
			m_combined.add( new Combined(clip) );
			--i;
		}
	}
	Debug::printf( "LightMapPacker: Clipped to %d combined polygons\n", m_combined.size() );
}

void LightMapPacker::packMaps()
{
	// sort combined polygons by 1) by material 2) by size
	std::sort( m_combined.begin(), m_combined.end(), Combined::SortByMaterial() );
	for ( int i = 0 ; i < m_combined.size() ; ++i )
	{
		int k = i+1;
		while ( k < m_combined.size() && 
			m_combined[k]->triangles[0]->material == m_combined[i]->triangles[0]->material )
		{
			++k;
		}

		std::sort( m_combined.begin()+i, m_combined.begin()+k, Combined::SortByArea() );
	}

	for ( int i = 0 ; i < m_combined.size() ; ++i )
	{
		if ( (0xFF&i) == 0 )
			Debug::printf( "LightMapPacker.packMaps: %d/%d done\n", i, m_combined.size() );

		Combined& comb = *m_combined[i];
		Rect polyrc = comb.rectUV();
		
		// best fit
		Rect foundrc( Integer::MAX_VALUE, Integer::MAX_VALUE );
		int foundmapindex = -1;
		int closestrectix = -1;
		for ( int k = 0 ; k < m_lightmaps.size() ; ++k )
		{
			LightMap* lmap = m_lightmaps[k];
			if ( k == m_lightmaps.size()-1 || 
				lmap->hasMaterial(comb.triangles[0]->material) )
			{
				for ( int n = 0 ; n < lmap->freerects.size() ; ++n )
				{
					Rect freerc = lmap->freerects[n];
					if ( polyrc.width() < freerc.width() &&
						polyrc.height() < freerc.height() &&
						(closestrectix == -1 || 
						freerc.area() < m_lightmaps[foundmapindex]->freerects[closestrectix].area()) )
					{
						foundmapindex = k;
						closestrectix = n;
					}
				}
			}
		}

		// no space in existing maps?
		if ( -1 == foundmapindex )
		{
			m_lightmaps.add( new LightMap(m_mapWidth, m_mapHeight, m_lightmaps.size()) );
			assert( m_lightmaps.last()->isSpace(polyrc) );
			--i;
			continue;
		}

		// clip top-left corner of free rect
		LightMap* foundmap = m_lightmaps[foundmapindex];
		Rect closestrect = foundmap->freerects[closestrectix];
		foundmap->freerects.remove( closestrectix );
		closestrect.clipTopLeft( polyrc.width(), polyrc.height(), foundmap->freerects );

		// translate combined uv to found rc
		float2 uvoffset( float(-polyrc.x0), float(-polyrc.y0) );
		uvoffset += float2( float(closestrect.x0), float(closestrect.y0) );
		for ( int k = 0 ; k < comb.triangles.size() ; ++k )
		{
			Triangle* tri = comb.triangles[k];
			for ( int n = 0 ; n < 3 ; ++n )
			{
				tri->uv[n] += uvoffset;

				// clamp to exact pixel
				int u = int( tri->uv[n].x + .5f );
				int v = int( tri->uv[n].y + .5f );
				tri->uv[n] = float2( float(u), float(v) );
				assert( tri->uv[n].x >= 0.f && tri->uv[n].x <= float(m_mapWidth) );
				assert( tri->uv[n].y >= 0.f && tri->uv[n].y <= float(m_mapWidth) );
			}
		}

		for ( int k = 0 ; k < comb.triangles.size() ; ++k )
			comb.triangles[k]->lightmap = foundmap;
		foundmap->combined.add( m_combined[i] );
	}

	Debug::printf( "LightMapPacker: Packed %d triangles to %d maps (%dx%d)\n", m_triangles.size(), m_lightmaps.size(), m_mapWidth, m_mapHeight );
}

void LightMapPacker::cropMaps()
{
	for ( int i = 0 ; i < m_lightmaps.size() ; ++i )
		m_lightmaps[i]->crop();
}

void LightMapPacker::setProgress( const char* sz, float percent )
{
	if ( m_progress != 0 )
	{
		m_progress->setText( sz );
		m_progress->setProgress( percent );
	}
}


} // lightmappacker
