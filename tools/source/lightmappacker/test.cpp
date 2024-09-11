#include <lightmappacker/LightMapPacker.h>
#include <img/ShapeUtil.h>
#include <img/ImageWriter.h>
#include <io/all.h>
#include <lang/all.h>
#include <math/all.h>
#include <assert.h>
#include <stdio.h>
#include <config.h>


USING_NAMESPACE(io)
USING_NAMESPACE(gr)
USING_NAMESPACE(math)
USING_NAMESPACE(lang)


namespace lightmappacker
{



static void dumpLightMaps( const char* filenamefmt, LightMapPacker& lmp )
{
	const Array<P(LightMap)>& lightmaps = lmp.lightmaps();
	for ( int i = 0 ; i < lightmaps.size() ; ++i )
	{
		LightMap* lmap = lightmaps[i];
		int w = lmap->width();
		int h = lmap->height();

		Array<int> pic;
		pic.resize( w*h, 0 );
		int pitch = w * 4;
		
		//int colors[] = {0xFF,0xFF00,0xFFFF,0xFF0000,0xFF00FF,0xFFFF00,0xFFFFFF};
		int colors[] = {0x60,0x6000,0x600000};
		const int COLORS = sizeof(colors)/sizeof(colors[0]);
		int colorindex = 0;

		for ( int k = 0 ; k < lmap->combined.size() ; ++k )
		{
			for ( int n = 0 ; n < lmap->combined[k]->triangles.size() ; ++n )
			{
				Triangle& tri = *lmap->combined[k]->triangles[n];
				colorindex = k;
				int color = colors[colorindex % COLORS];
				colorindex = (colorindex+1) % COLORS;

				float2 uv[3];
				for ( int m = 0 ; m < 3 ; ++m )
				{
					uv[m] = tri.uv[m];
					//uv[m].x *= (float)lmap->width();
					//uv[m].y *= (float)lmap->height();
				}

				int u[] = { int(uv[0].x), int(uv[1].x), int(uv[2].x) };
				int v[] = { int(uv[0].y), int(uv[1].y), int(uv[2].y) };

				img::ShapeUtil::drawPolygon( &pic[0], w, h, u, v, 3, color );
			}
		}

		for ( int k = 0 ; k < lmap->combined.size() ; ++k )
		{
			for ( int n = 0 ; n < lmap->combined[k]->triangles.size() ; ++n )
			{
				Triangle& tri = *lmap->combined[k]->triangles[n];

				float2 uv[3];
				for ( int m = 0 ; m < 3 ; ++m )
				{
					uv[m] = tri.uv[m];
					//uv[m].x *= (float)lmap->width();
					//uv[m].y *= (float)lmap->height();
				}

				int u[] = { int(uv[0].x), int(uv[1].x), int(uv[2].x) };
				int v[] = { int(uv[0].y), int(uv[1].y), int(uv[2].y) };

				img::ShapeUtil::drawLine( &pic[0], w, h, u[0], v[0], u[1], v[1], 0xFFFFFFFF );
				img::ShapeUtil::drawLine( &pic[0], w, h, u[1], v[1], u[2], v[2], 0xFFFFFFFF );
				img::ShapeUtil::drawLine( &pic[0], w, h, u[2], v[2], u[0], v[0], 0xFFFFFFFF );
			}
		}

		char fname[1000];
		sprintf( fname, filenamefmt, i );
		img::ImageWriter::writePNG( fname, pic.begin(), w, h, pitch, SurfaceFormat::SURFACE_X8R8G8B8, 0, SurfaceFormat() );
	}
}

static void run()
{
	int mapsize = 1024;
	LightMapPacker lmp( 1.f/.05f, mapsize, mapsize );

	lmp.addTriangle( 0, 0, 0, float3(2.034807f,0.097318f,1.675517f), float3(2.034807f,0.097318f,-2.256130f), float3(-2.247185f,0.097318f,-2.256130f) );
	lmp.addTriangle( 0, 1, 0, float3(-2.247185f,0.097318f,-2.256130f), float3(-2.247185f,0.097318f,1.675517f), float3(2.034807f,0.097318f,1.675517f) );
/*
	lmp.addTriangle( 0, 0, 0, float3(0.984221f,0.000000f,-0.803071f), float3(-3.270982f,0.000000f,-0.803071f), float3(-3.270982f,0.000000f,-1.402924f) );
	lmp.addTriangle( 0, 1, 0, float3(-3.270982f,0.000000f,-1.402924f), float3(0.984221f,0.000000f,-1.402924f), float3(0.984221f,0.000000f,-0.803071f) );
	lmp.addTriangle( 0, 2, 0, float3(0.984221f,0.768561f,-0.803071f), float3(0.984221f,0.768561f,-1.402924f), float3(-3.270982f,0.768561f,-1.402924f) );
	lmp.addTriangle( 0, 3, 0, float3(-3.270982f,0.768561f,-1.402924f), float3(-3.270982f,0.768561f,-0.803071f), float3(0.984221f,0.768561f,-0.803071f) );
	lmp.addTriangle( 0, 4, 0, float3(0.984221f,0.768561f,-1.402924f), float3(0.984221f,0.000000f,-1.402924f), float3(-3.270982f,0.000000f,-1.402924f) );
	lmp.addTriangle( 0, 5, 0, float3(-3.270982f,0.000000f,-1.402924f), float3(-3.270982f,0.768561f,-1.402924f), float3(0.984221f,0.768561f,-1.402924f) );
	lmp.addTriangle( 0, 6, 0, float3(0.984221f,0.768561f,-0.803071f), float3(0.984221f,0.000000f,-0.803071f), float3(0.984221f,0.000000f,-1.402924f) );
	lmp.addTriangle( 0, 7, 0, float3(0.984221f,0.000000f,-1.402924f), float3(0.984221f,0.768561f,-1.402924f), float3(0.984221f,0.768561f,-0.803071f) );
	lmp.addTriangle( 0, 8, 0, float3(-3.270982f,0.768561f,-0.803071f), float3(-3.270982f,0.000000f,-0.803071f), float3(0.984221f,0.000000f,-0.803071f) );
	lmp.addTriangle( 0, 9, 0, float3(0.984221f,0.000000f,-0.803071f), float3(0.984221f,0.768561f,-0.803071f), float3(-3.270982f,0.768561f,-0.803071f) );
	lmp.addTriangle( 0, 10, 0, float3(-3.270982f,0.768561f,-1.402924f), float3(-3.270982f,0.000000f,-1.402924f), float3(-3.270982f,0.000000f,-0.803071f) );
	lmp.addTriangle( 0, 11, 0, float3(-3.270982f,0.000000f,-0.803071f), float3(-3.270982f,0.768561f,-0.803071f), float3(-3.270982f,0.768561f,-1.402924f) );
	lmp.addTriangle( 1, 0, 0, float3(-0.000000f,0.056236f,-1.340295f), float3(0.000000f,0.056236f,1.302804f), float3(0.000000f,-0.318672f,1.302804f) );
	lmp.addTriangle( 1, 1, 0, float3(0.000000f,-0.318672f,1.302804f), float3(-0.000000f,-0.318672f,-1.340295f), float3(-0.000000f,0.056236f,-1.340295f) );
	lmp.addTriangle( 1, 2, 0, float3(-2.436901f,0.056236f,-1.340295f), float3(-2.436901f,-0.318672f,-1.340295f), float3(-2.436900f,-0.318672f,1.302805f) );
	lmp.addTriangle( 1, 3, 0, float3(-2.436900f,-0.318672f,1.302805f), float3(-2.436900f,0.056236f,1.302805f), float3(-2.436901f,0.056236f,-1.340295f) );
	lmp.addTriangle( 1, 4, 0, float3(-2.436901f,-0.318672f,-1.340295f), float3(-0.000000f,-0.318672f,-1.340295f), float3(0.000000f,-0.318672f,1.302804f) );
	lmp.addTriangle( 1, 5, 0, float3(0.000000f,-0.318672f,1.302804f), float3(-2.436900f,-0.318672f,1.302805f), float3(-2.436901f,-0.318672f,-1.340295f) );
	lmp.addTriangle( 1, 6, 0, float3(-2.436901f,0.056236f,-1.340295f), float3(-0.000000f,0.056236f,-1.340295f), float3(-0.000000f,-0.318672f,-1.340295f) );
	lmp.addTriangle( 1, 7, 0, float3(-0.000000f,-0.318672f,-1.340295f), float3(-2.436901f,-0.318672f,-1.340295f), float3(-2.436901f,0.056236f,-1.340295f) );
	lmp.addTriangle( 1, 8, 0, float3(-2.436900f,0.056236f,1.302805f), float3(0.000000f,0.056236f,1.302804f), float3(-0.000000f,0.056236f,-1.340295f) );
	lmp.addTriangle( 1, 9, 0, float3(-0.000000f,0.056236f,-1.340295f), float3(-2.436901f,0.056236f,-1.340295f), float3(-2.436900f,0.056236f,1.302805f) );
	lmp.addTriangle( 1, 10, 0, float3(-2.436900f,-0.318672f,1.302805f), float3(0.000000f,-0.318672f,1.302804f), float3(0.000000f,0.056236f,1.302804f) );
	lmp.addTriangle( 1, 11, 0, float3(0.000000f,0.056236f,1.302804f), float3(-2.436900f,0.056236f,1.302805f), float3(-2.436900f,-0.318672f,1.302805f) );
*/
	lmp.build();

	dumpLightMaps( "C:/TEMP/out_lightmap%03d.png", lmp );
}

void test()
{
	String libname = "lightmappacker";

	Debug::printf( "\n-------------------------------------------------------------------------\n" );
	Debug::printf( "%s library test begin\n", libname.c_str() );
	Debug::printf( "-------------------------------------------------------------------------\n" );
	run();
	Debug::printf( "%s library test ok\n", libname.c_str() );
}


} // lightmappacker

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
