#include "StdAfx.h"
#include "MyShape.h"
#include "INodeUtil.h"
#include "MySceneExport.h"
#include <lang/Math.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


static void localPrintf( FILE* fh, const char* fmt, ... )
{
	// format variable arguments
	const unsigned MAX_MSG = 2000;
	char msg[MAX_MSG+4];
	va_list marker;
	va_start( marker, fmt );
	vsprintf( msg, fmt, marker );
	va_end( marker );
	assert( strlen(msg) < MAX_MSG ); // too long debug message

	Debug::printf( "%s", msg );
	fprintf( fh, "%s", msg );
}


MyShape::MyShape( INode* node3ds ) :
	MyNode( node3ds, new hgr::Lines(0) )
{
	PolyShape polyshape;
	ObjectState objstate = node3ds->EvalWorldState( 0 );
	ShapeObject* shape = static_cast<ShapeObject*>( objstate.obj );
	shape->MakePolyShape( 0, polyshape, PSHAPE_BUILTIN_STEPS, FALSE );
	Matrix3 vertextm = INodeUtil::getVertexTransform( node3ds );
	Matrix3 nodetm = node3ds->GetNodeTM(0);

	FILE* fh = fopen( MySceneExport::getPostfixedOutputFilename("_lines.txt").c_str(), "at" );

	// DEBUG: print line points
	localPrintf( fh, "// Exporting PolyShape \"%s\", %d lines\n", name().c_str(), polyshape.numLines );
	Array<Point3> points;
	for ( int i = 0 ; i < polyshape.numLines ; ++i )
	{
		PolyLine& polyline = polyshape.lines[i];

		// get points
		localPrintf( fh, "// Line [%d][xyz], %d points (non-interpolated)\n{", i, polyline.numPts );
		points.clear();
		for ( int k = 0 ; k < polyline.numPts ; ++k )
		{
			Point3 p;
			nodetm.TransformPoints( &polyline.pts[k].p, &p, 1 );
			points.add(p);
			localPrintf( fh, " {%.0f,%.0f,%.0f}", p.x, p.y, p.z );
			if ( k+1 < polyline.numPts )
				localPrintf( fh, "," );
			if ( ((k+1) % 10) == 0 )
				localPrintf( fh, "\n" );
		}
		localPrintf( fh, "};\n" );

		// split if distance larger than MAX_POINT_DIST
		const float MAX_POINT_DIST = 50.f;
		bool splitvert = true;
		while ( splitvert )
		{
			splitvert = false;
			for ( int k = 0 ; k < points.size() ; ++k )
			{
				//localPrintf( "processing point %d (of %d)\n", k, points.size() );
				int n = (k+1)%points.size();
				float dx = points[n].x-points[k].x;
				float dy = points[n].y-points[k].y;
				float dz = points[n].z-points[k].z;
				float d = Math::sqrt( dx*dx + dy*dy + dz*dz );

				if ( d > MAX_POINT_DIST )
				{
					//localPrintf( "  distance from k%d=(%g %g %g) to n%d=(%g %g %g) is %g, splitting\n", k, points[k].x, points[k].y, points[k].z, n, points[n].x, points[n].y, points[n].z, d );
					float x1 = (points[n].x+points[k].x)*.5f;
					float y1 = (points[n].y+points[k].y)*.5f;
					float z1 = (points[n].z+points[k].z)*.5f;
					points.add( n, Point3(x1,y1,z1) );
					splitvert = true;
					--k;
				}
			}
		}

		// output to debug stream
		localPrintf( fh, "// Line [%d][xyz], %d points (interpolated)\n{", i, points.size() );
		for ( int k = 0 ; k < points.size() ; ++k )
		{
			if ( (k+1) % 10 == 0 )
				localPrintf( fh, "\n" );
			
			Point3 p = points[k];
			localPrintf( fh, "{%.0f,%.0f,%.0f}", p.x, p.y, p.z );

			if ( k+1 < points.size() )
				localPrintf( fh, ", " );
			else
				localPrintf( fh, "};\n" );
		}
	}

	for ( int i = 0 ; i < polyshape.numLines ; ++i )
	{
		const int prevlines = node()->lines();
		PolyLine& polyline = polyshape.lines[i];

		Point3 p0 = vertextm * polyline.pts[0].p;
		float3 prev = float3(p0.x,p0.y,p0.z);
		for ( int k = 1 ; k < polyline.numPts ; ++k )
		{
			Point3 p1 = vertextm * polyline.pts[k].p;
			float3 point = float3(p1.x,p1.y,p1.z);
			node()->addLine( prev, point, float4(1,1,1,1) );
			prev = point;
		}

		node()->addPath( prevlines, node()->lines() );
	}

	fclose( fh );
}
