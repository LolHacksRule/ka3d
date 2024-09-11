#include <lightmappacker/Triangle.h>
#include <lang/Math.h>
#include <lang/Debug.h>
#include <math/IntersectionUtil.h>
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


namespace lightmappacker
{


bool Triangle::SortByPlaneD::operator()( const Triangle& a, const Triangle& b ) const
{
	const float eps = 1e-9f;
	float d = a.plane.w - b.plane.w; 
	bool signedd = d < 0.f;
	float absd = signedd ? -d : d;
	return absd < eps ? false : signedd;
}

bool Triangle::SortByMaxU::operator()( const Triangle* a, const Triangle* b ) const
{
	return a->maxU() < b->maxU();
}

bool Triangle::SortByMaxV::operator()( const Triangle* a, const Triangle* b ) const
{
	return a->maxV() < b->maxV();
}

bool Triangle::SortByAreaUV::operator()( const Triangle* a, const Triangle* b ) const
{
	return a->rectUV().area() < b->rectUV().area();
}

Triangle::Triangle() :
	triangle( -1 ),
	material( -1 ),
	mesh( -1 ),
	lightmap( 0 )
{
}

void Triangle::mapUV( float texeldensity )
{
	switch ( primaryAxis() )
	{
	case 0:
		for ( int i = 0; i < 3 ; ++i )
		{
        	uv[i].x = pos[i].z * texeldensity;
			uv[i].y = -pos[i].y * texeldensity;
		}
		break;

	case 1:
		for ( int i = 0; i < 3 ; ++i )
		{
	        uv[i].x = pos[i].x * texeldensity;
			uv[i].y = -pos[i].z * texeldensity;
		}
		break;

	case 2:
		for ( int i = 0; i < 3 ; ++i )
		{
	        uv[i].x = pos[i].x * texeldensity;
			uv[i].y = -pos[i].y * texeldensity;
		}
		break;
	}

	// round to exact pixel
	for ( int i = 0 ; i < 3 ; ++i )
	{
		if ( uv[i].x > 0.f )
			uv[i].x = floor(uv[i].x+.5f);
		else
			uv[i].x = ceil(uv[i].x-.5f);
		
		if ( uv[i].y > 0.f )
			uv[i].y = floor(uv[i].y+.5f);
		else
			uv[i].y = ceil(uv[i].y-.5f);
	}

	assert( areaUV() > 1e-9f );
}

int Triangle::primaryAxisSign() const
{
	const int axis = primaryAxis();
	const float3 axisv[] = {float3(1,0,0), float3(0,1,0), float3(0,0,1)};
	return dot( axisv[axis], normal() ) >= 0.f ? 1 : -1;
}

int	Triangle::primaryAxis() const
{
	float3 absnormal = abs( normal() );
	if ( absnormal.x >= absnormal.y && absnormal.x >= absnormal.z )
		return 0;
	else if (absnormal.y >= absnormal.x && absnormal.y >= absnormal.z )
		return 1;
	else
		return 2;
}

bool Triangle::clipByUV( float du, float dv, Triangle* out, int* clipedge1, int* clipedge2 )
{
	int e1 = -1;
	int e2 = -1;
	float2 maxduv(0,0);

	int k = 2;
	for ( int i = 0 ; i < 3 ; k = i++ )
	{
		float2 duv( Math::abs(uv[i].x-uv[k].x),  Math::abs(uv[i].y-uv[k].y) );
		if ( duv.x > du || duv.y > dv )
		{
			if ( duv.lengthSquared() > maxduv.lengthSquared() )
			{
				e1 = k;
				e2 = i;
				maxduv = duv;
			}
		}
	}

	if ( e1 != -1 )
	{
		//Debug::printf( "Splitting triangle[%d]: (%g %g) (%g %g) (%g %g)\n", triangle, uv[0].x, uv[0].y, uv[1].x, uv[1].y, uv[2].x, uv[2].y );
		*out = *this;
		float3 deltapos = (pos[e2] - pos[e1])*.5f;
		float2 deltauv = (uv[e2] - uv[e1])*.5f;
		pos[e1] += deltapos;
		uv[e1] += deltauv;
		out->pos[e2] -= deltapos;
		out->uv[e2] -= deltauv;

		// store clip info
		*clipedge1 = e1;
		*clipedge2 = e2;
		return true;
	}

	return false;
}

float Triangle::areaUV() const
{
	float3 e1( uv[1] - uv[0], 0 );
	float3 e2( uv[2] - uv[0], 0 );
	float a = cross(e1,e2).length() * .5f;
	return a;
}

Rect Triangle::rectUV() const
{
	return Rect( int(minU()), int(minV()), int(maxU()), int(maxV()) );
}

void Triangle::translateUV( const float2& duv )
{
	for ( int i = 0 ; i < 3 ; ++i )
		uv[i] += duv;
}

void Triangle::getScaledUV( NS(math,float2)* out, float scale ) const
{
	float2 center = (uv[0] + uv[1] + uv[2]) * .33333333333333f;
	for ( int i = 0 ; i < 3 ; ++i )
		out[i] = (uv[i]-center)*scale + center;
}

bool Triangle::isOverlapUV( const Triangle& tri ) const
{
	float2 a[3];
	float2 b[3];
	getScaledUV( a, .99f );
	tri.getScaledUV( b, .99f );
	return IntersectionUtil::testTriangleTriangleOverlap( a, b );
}


} // lightmappacker
