#include <lightmappacker/Rect.h>
#include <lang/Math.h>
#include <assert.h>
#include <config.h>


USING_NAMESPACE(lang)


namespace lightmappacker
{


Rect::Rect() :
	x0( 0 ),
	y0( 0 ),
	x1( 0 ),
	y1( 0 )
{
}

Rect::Rect( int w, int h ) :
	x0( 0 ),
	y0( 0 ),
	x1( w ),
	y1( h )
{
}

Rect::Rect( int u0, int v0, int u1, int v1 ) :
	x0( u0 ),
	y0( v0 ),
	x1( u1 ),
	y1( v1 )
{
}

void Rect::clipTopLeft( int dx, int dy, Array<Rect>& freerects ) const
{
	assert( dx <= width() );
	assert( dy <= height() );

	int freex = width() - dx;
	int freey = height() - dy;

	if ( freey > freex )
	{
		Rect rc;
		rc.x0 = x0 + dx;
		rc.y0 = y0;
		rc.x1 = x1;
		rc.y1 = y0 + dy;
		freerects.add( rc );

		rc.x0 = x0;
		rc.y0 = y0 + dy;
		rc.x1 = x1;
		rc.y1 = y1;
		freerects.add( rc );
	}
	else
	{
		Rect rc;
		rc.x0 = x0 + dx;
		rc.y0 = y0;
		rc.x1 = x1;
		rc.y1 = y1;
		freerects.add( rc );

		rc.x0 = x0;
		rc.y0 = y0 + dy;
		rc.x1 = x0 + dx;
		rc.y1 = y1;
		freerects.add( rc );
	}
}

Rect Rect::operator&( const Rect& other ) const
{
	return Rect( 
		Math::max(x0,other.x0), Math::max(y0,other.y0),
		Math::min(x1,other.x1), Math::min(y1,other.y1) );
}

Rect Rect::operator|( const Rect& other ) const
{
	return Rect( 
		Math::min(x0,other.x0), Math::min(y0,other.y0),
		Math::max(x1,other.x1), Math::max(y1,other.y1) );
}


} // lightmappacker
