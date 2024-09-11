#ifndef _LIGHTMAPPACKER_MAP_H
#define _LIGHTMAPPACKER_MAP_H


#include <lightmappacker/Rect.h>
#include <lightmappacker/Combined.h>
#include <lang/Array.h>
#include <lang/Object.h>
#include <lang/String.h>
#include <math/float3x3.h>


namespace lightmappacker
{


/**
 * List of triangles packed to a lightmap.
 */
class LightMap :
	public lang::Object
{
public:
	/** Lists of grouped triangles. */
	NS(lang,Array)<P(Combined)>	combined;
	/** Free rectangles in in this lightmap. */
	NS(lang,Array)<Rect>			freerects;
	/** Actual light data. Not used by the lightmap packer. */
	NS(lang,Array)<NS(math,float3x3)>	data;
	/** Names of the lightmap. Not used by the lightmap packer. */
	NS(lang,String)				name[3];

	LightMap( int w, int h, int index );

	/** 
	 * Changes width/height based on used triangle UV area. 
	 * Does not affect user data array! 
	 */
	void	crop();

	bool	isSpace( const Rect& rc ) const;

	Rect	maxFreeRect() const;

	bool	hasMaterial( int materialindex ) const;

	/** Returns width of this lightmap in pixels. */
	int		width() const;

	/** Returns height of this lightmap in pixels. */
	int		height() const;

	/** Returns index of this lightmap. */
	int		index() const;

	/** Returns area in pixels. */
	int		area() const;

private:
	int		m_width;
	int		m_height;
	int		m_index;
};


} // lightmappacker


#endif // _LIGHTMAPPACKER_MAP_H
