#ifndef _LIGHTMAPPACKER_TRIANGLE_H
#define _LIGHTMAPPACKER_TRIANGLE_H


#include <math/float2.h>
#include <math/float3.h>
#include <math/float4.h>
#include <lightmappacker/Rect.h>


namespace lightmappacker
{


class LightMap;


/**
 * Lightmap triangle.
 */
class Triangle
{
public:
	struct SortByPlaneD
	{
		bool operator()( const Triangle& a, const Triangle& b ) const;
	};

	struct SortByMaxU
	{
		bool operator()( const Triangle* a, const Triangle* b ) const;
	};

	struct SortByMaxV
	{
		bool operator()( const Triangle* a, const Triangle* b ) const;
	};

	struct SortByAreaUV
	{
		bool operator()( const Triangle* a, const Triangle* b ) const;
	};

	/** User specified index. */
	int				triangle;
	/** User specified index. */
	int				material;
	/** User specified index. */
	int				mesh;

	/** Vertex positions. */
	NS(math,float3)	pos[3];
	/** Polygon mapping plane. */
	NS(math,float4)	plane;
	/** Lightmap UV-coordinates (in pixels!). */
	NS(math,float2)	uv[3];
	/** In which lightmap this triangle is stored. */
	LightMap*		lightmap;

	Triangle();

	void			mapUV( float texeldensity );

	/** Returns true if clipped. */
	bool			clipByUV( float du, float dv, Triangle* out, int* clipedge1, int* clipedge2 );

	void			translateUV( const NS(math,float2)& duv );

	NS(math,float3)	normal() const		{return NS(math,float3)(plane.x,plane.y,plane.z);}

	int				primaryAxis() const;

	int				primaryAxisSign() const;

	float			areaUV() const;

	float			minU() const		{return uv[0].x < uv[1].x ? (uv[0].x < uv[2].x ? uv[0].x : uv[2].x) : (uv[1].x < uv[2].x ? uv[1].x : uv[2].x);}

	float			minV() const		{return uv[0].y < uv[1].y ? (uv[0].y < uv[2].y ? uv[0].y : uv[2].y) : (uv[1].y < uv[2].y ? uv[1].y : uv[2].y);}

	float			maxU() const		{return uv[0].x > uv[1].x ? (uv[0].x > uv[2].x ? uv[0].x : uv[2].x) : (uv[1].x > uv[2].x ? uv[1].x : uv[2].x);}

	float			maxV() const		{return uv[0].y > uv[1].y ? (uv[0].y > uv[2].y ? uv[0].y : uv[2].y) : (uv[1].y > uv[2].y ? uv[1].y : uv[2].y);}

	Rect			rectUV() const;

	bool			isOverlapUV( const Triangle& tri ) const;

	void			getScaledUV( NS(math,float2)* out, float scale ) const;
};


} // lightmappacker


#endif // _LIGHTMAPPACKER_TRIANGLE_H
