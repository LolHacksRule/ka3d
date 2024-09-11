#include <lightmappacker/LightMap.h>
#include <config.h>


namespace lightmappacker
{


LightMap::LightMap( int w, int h, int index ) :
	m_width( w ),
	m_height( h ),
	m_index( index )
{
	freerects.add( Rect(w,h) );
}

void LightMap::crop()
{
	assert( data.size() == 0 ); // cannot be initialized before

	Rect rc(0,0,0,0);
	for ( int k = 0 ; k < combined.size() ; ++k )
		rc = rc | combined[k]->rectUV();

	while ( rc.x1+2 < m_width/2 )
		m_width /= 2;
	while ( rc.y1+2 < m_height/2 )
		m_height /= 2;
}

bool LightMap::isSpace( const Rect& rc ) const
{
	for ( int i = 0 ; i < freerects.size() ; ++i )
	{
		if ( rc.width() <= freerects[i].width() && 
			rc.height() <= freerects[i].height() )
			return true;
	}
	return false;
}

bool LightMap::hasMaterial( int materialindex ) const
{
	if ( combined.size() == 0 )
		return false;
	return combined[0]->triangles[0]->material == materialindex;
}

int LightMap::width() const
{
	return m_width;
}

int LightMap::height() const
{
	return m_height;
}

int LightMap::index() const
{
	return m_index;
}

int LightMap::area() const
{
	return m_width*m_height;
}


} // lightmappacker
