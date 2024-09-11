#ifndef _LIGHTMAPPACKER_COMBINED_H
#define _LIGHTMAPPACKER_COMBINED_H


#include <lightmappacker/Rect.h>
#include <lightmappacker/Triangle.h>
#include <lang/Array.h>
#include <lang/Object.h>


namespace lightmappacker
{


/**
 * Group of triangles.
 */
class Combined :
	public lang::Object
{
public:
	struct SortByMaterial
	{
		bool operator()( Combined* a, Combined* b ) const;
	};

	struct SortByArea
	{
		bool operator()( Combined* a, Combined* b ) const;
	};

	/** List of triangles in this group. */
	NS(lang,Array)<Triangle*>	triangles;

	/** Clips this group to the limits (du,dv). Returns true if clipped. */
	bool			clipByUV( float du, float dv, Combined* out );

	/** Returns rectangle which has top left coordinate (0,0). */
	Rect			rectUV() const;

	/** Returns true if the triangle can be combined. */
	bool			canBeCombined( const Triangle* tri ) const;

private:
	/** Return U size. */
	float			sizeU() const;

	/** Return V size. */
	float			sizeV() const;

	/** Returns bounding rect. */
	void			getDimensionsUV( float* u0, float* v0, float* u1, float* v1 ) const;
};


} // lightmappacker


#endif // _LIGHTMAPPACKER_COMBINED_H
