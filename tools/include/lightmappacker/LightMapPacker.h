#ifndef _LIGHTMAPPACKER_LIGHTMAPPACKER_H
#define _LIGHTMAPPACKER_LIGHTMAPPACKER_H


#include <lightmappacker/LightMap.h>
#include <lang/Object.h>


namespace lightmappacker
{


class ProgressInterface;


/**
 * Packs polygon lightmaps to textures.
 * Use this class to prepare lightmaps for calculation.
 * Specify lightmap parameters in constructor.
 * Add all triangles before computing lightmaps by using addTriangle.
 * Call build() after all triangles have been added.
 */
class LightMapPacker :
	public lang::Object
{
public:
	/**
	 * Triangle clipped by lightmap packer.
	 * Triangle is always clipped half-way between vertices.
	 */
	class ClippedTriangle
	{
	public:
		/** Result triangle. */
		Triangle	triangle;
		/** Source edge start. */
		int			edge1;
		/** Source edge end. */
		int			edge2;
	};

	/**
	 * Creates lightmap packer with specified options.
	 * Add all triangles before computing lightmaps by using addTriangle.
	 * Call compute() after all triangles have been added.
	 * @param texeldensity Texel density, texels / unit (like texels/m).
	 * @param mapwidth Default width of a single lightmap.
	 * @param mapheight Default height of a single lightmap.
	 */
	LightMapPacker( float texeldensity, int mapwidth, int mapheight );

	~LightMapPacker();

	/**
	 * Adds a triangle to the computation.
	 * Add all triangles before computing lightmaps.
	 * Call build() after all triangles have been added.
	 * @param mesh User mesh index. Unused by the packer.
	 * @param triangle User triangle index. Unused by the packer.
	 * @param material User material index. Used to group same material triangles to same lightmap.
	 * @param v1 Triangle vertex 1.
	 * @param v1 Triangle vertex 2.
	 * @param v1 Triangle vertex 3.
	 */
	void	addTriangle( int mesh, int triangle, int material,
				const NS(math,float3)& v1, const NS(math,float3)& v2, const NS(math,float3)& v3 );

	/**
	 * Builds lightmaps from added triangles.
	 * @param progress Optional progress bar interface.
	 */
	void	build( ProgressInterface* progress=0 );

	/**
	 * Returns computed lightmaps.
	 */
	const NS(lang,Array)<P(LightMap)>&		lightmaps() const	{return m_lightmaps;}

	/**
	 * Returns list of source triangles.
	 */
	const NS(lang,Array)<Triangle>&		triangles() const	{return m_triangles;}

	/**
	 * Returns list of clipped triangles.
	 */
	const NS(lang,Array)<ClippedTriangle>&	clippedTriangles() const	{return m_clippedTriangles;}

private:
	float							m_texelDensity;
	int								m_mapWidth;
	int								m_mapHeight;
	NS(lang,Array)<Triangle>			m_triangles;
	NS(lang,Array)<P(Combined)>		m_combined;
	NS(lang,Array)<P(LightMap)>		m_lightmaps;
	NS(lang,Array)<ClippedTriangle>	m_clippedTriangles;
	ProgressInterface*				m_progress;

	void	mapTriangles();
	void	clipTriangles();
	void	buildCombined();
	void	clipCombined();
	void	packMaps();
	void	cropMaps();

	void	setProgress( const char* sz, float percent );

	LightMapPacker( const LightMapPacker& );
	LightMapPacker& operator=( const LightMapPacker& );
};


} // lightmappacker


#endif // _LIGHTMAPPACKER_LIGHTMAPPACKER_H
