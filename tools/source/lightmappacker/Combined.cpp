#include <lightmappacker/Combined.h>
#include <lang/Math.h>
#include <lang/Float.h>
#include <algorithm>
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


namespace lightmappacker
{


bool Combined::SortByMaterial::operator()( Combined* a, Combined* b ) const
{
	return a->triangles[0]->material < b->triangles[0]->material;
}

bool Combined::SortByArea::operator()( Combined* a, Combined* b ) const
{
	return a->rectUV().area() > b->rectUV().area();
}

bool Combined::clipByUV( float du, float dv, Combined* out )
{
	std::sort( triangles.begin(), triangles.end(), Triangle::SortByMaxU() );
	while ( sizeU() >= du )
	{
		out->triangles.add( triangles.last() );
		triangles.resize( triangles.size()-1 );
	}

	std::sort( triangles.begin(), triangles.end(), Triangle::SortByMaxV() );
	while ( sizeV() >= dv )
	{
		out->triangles.add( triangles.last() );
		triangles.resize( triangles.size()-1 );
	}

	assert( (float)rectUV().width() <= du && (float)rectUV().height() <= dv );
	return out->triangles.size() > 0;;
}

void Combined::getDimensionsUV( float* u0, float* v0, float* u1, float* v1 ) const
{
	*u0 = Float::MAX_VALUE;
	*v0 = Float::MAX_VALUE;
	*u1 = -Float::MAX_VALUE;
	*v1 = -Float::MAX_VALUE;

	for ( int i = 0 ; i < triangles.size() ; ++i )
	{
		for ( int k = 0 ; k < 3 ; ++k )
		{
			float2 uv = triangles[i]->uv[k];
			*u0 = Math::min( uv.x, *u0 );
			*v0 = Math::min( uv.y, *v0 );
			*u1 = Math::max( uv.x, *u1 );
			*v1 = Math::max( uv.y, *v1 );
		}
	}
}

float Combined::sizeU() const
{
	float u0, v0, u1, v1;
	getDimensionsUV( &u0, &v0, &u1, &v1 );
	return u1-u0+1.f;
}

float Combined::sizeV() const
{
	float u0, v0, u1, v1;
	getDimensionsUV( &u0, &v0, &u1, &v1 );
	return v1-v0+1.f;
}

Rect Combined::rectUV() const
{
	float u0, v0, u1, v1;
	getDimensionsUV( &u0, &v0, &u1, &v1 );
	return Rect( int(u0)-1, int(v0)-1, int(u1)+2, int(v1)+2 );
}

bool Combined::canBeCombined( const Triangle* tri ) const
{
	if ( triangles.size() > 0 )
	{
		if ( tri->material != triangles[0]->material )
			return false;

		if ( tri->mesh != triangles[0]->mesh )
			return false;

		for ( int n = 0 ; n < triangles.size() ; ++n )
		{
			Triangle* t = triangles[n];
			if ( t != tri && tri->isOverlapUV(*t) )
				return false;
		}
	}
	return true;
}


} // lightmappacker
