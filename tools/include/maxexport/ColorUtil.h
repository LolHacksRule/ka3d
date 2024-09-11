#ifndef _COLORUTIL_H
#define _COLORUTIL_H


#include <stdint.h>


class ColorUtil
{
public:
	static uint32_t rgb32( int r, int g, int b, int a )
	{
		if ( r < 0 )
			r = 0;
		else if ( r > 255 )
			r = 255;
		if ( g < 0 )
			g = 0;
		else if ( g > 255 )
			g = 255;
		if ( b < 0 )
			b = 0;
		else if ( b > 255 )
			b = 255;
		if ( a < 0 )
			a = 0;
		else if ( a > 255 )
			a = 255;
		return (a<<24) + (r<<16) + (g<<8) + b;
	}

	static uint32_t rgb32f( float r, float g, float b, float a )
	{
		if ( r < 0.f )
			r = 0.f;
		else if ( r > 1.f )
			r = 1.f;
		if ( g < 0.f )
			g = 0.f;
		else if ( g > 1.f )
			g = 1.f;
		if ( b < 0.f )
			b = 0.f;
		else if ( b > 1.f )
			b = 1.f;
		if ( a < 0.f )
			a = 0.f;
		else if ( a > 1.f )
			a = 1.f;
		return rgb32( int(r*255.f), int(g*255.f), int(b*255.f), int(a*255.f) );
	}
};


#endif // _COLORUTIL_H
