#ifndef _LIGHTMAPPACKER_RECT_H
#define _LIGHTMAPPACKER_RECT_H


#include <lang/Array.h>


namespace lightmappacker
{


class Rect
{
public:
	int		x0;
	int		y0;
	int		x1;
	int		y1;

	Rect();

	Rect( int w, int h );

	Rect( int u0, int v0, int u1, int v1 );

	/**
	 * Clips area of (dx,dy) from the rectangle and adds remaining
	 * sub-rectangles to the user array.
	 */
	void	clipTopLeft( int dx, int dy, NS(lang,Array)<Rect>& freerects ) const;

	/** 
	 * Returns intersection of rectangles. 
	 */
	Rect	operator&( const Rect& other ) const;

	/** 
	 * Returns union of rectangles. 
	 */
	Rect	operator|( const Rect& other ) const;

	int		width() const	{return x1-x0;}

	int		height() const	{return y1-y0;}

	int		area() const	{return width()*height();}

	bool	isEmpty() const	{return area() <= 0;}
};


} // lightmappacker


#endif // _LIGHTMAPPACKER_RECT_H
