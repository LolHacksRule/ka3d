#ifndef _MYMATERIAL_H
#define _MYMATERIAL_H


#include <gr/Shader.h>
#include <lang/Array.h>
#include <lang/Object.h>
#include <lang/String.h>
#include <math/float4.h>

#include "maxexportconfig.h"
#ifdef MAXEXPORT_HASDXMATERIAL
#include "IDxMaterial.h"
#else
class IDxMaterial;
#endif


class MyMesh;
class OptionInterface;


class MyMaterial :
	public gr::Shader
{
public:
	class MtlEqualsMyMaterial
	{
	public:
		MtlEqualsMyMaterial( Mtl* mtl )				: m_mtl(mtl) {}
		bool operator()( MyMaterial* a ) const		{return a->mtl3ds() == m_mtl;}

	private:
		Mtl*	m_mtl;
	};

	template <class T> class Param
	{
	public:
		NS(lang,String)	type;
		T				value;
	};

	/** See MaterialFlags. */
	int		flags;

	MyMaterial( Mtl* mtl, int index, Interface* ip, OptionInterface* opt, 
		const NS(lang,Array)<MyMesh*>& meshusers );

	~MyMaterial();

	void			setFloat( const char* param, float value );
	void			setColor( const char* param, const Color& color );
	void			setColor( const char* param, const AColor& color );
	void			setVector( const char* param, const NS(math,float4)& value );
	void			setTexture( const char* param, const NS(lang,String)& name );
	void			setTexture( const char* param, BitmapTex* texmap );
	void			setTexture( const char* param, PBBitmap* bmp );

	void					setName( const NS(lang,String)& name )					{m_name=name;}
	void					setSort( SortType sort )							{m_sort=sort;}

	Mtl*					mtl3ds() const										{return m_mtl3ds;}
	int						index() const										{return m_index;}
	bool					hasTexture( const char* typestr ) const;
	bool					hasVector( const char* typestr ) const;
	bool					hasFloat( const char* typestr ) const;
	NS(lang,String)			getTextureName( const char* typestr ) const;
	const NS(lang,String)&		name() const										{return m_name;}
	const NS(lang,String)&		shader() const										{return m_shader;}
	int						priority() const									{return 0;}
	bool					isDxMaterial() const								{return m_dxmtl != 0;}
	int						textures() const									{return m_textures.size();}
	int						vectors() const										{return m_vectors.size();}
	int						floats() const										{return m_floats.size();}
	bool					unlit() const										{return m_unlit;}
	NS(math,float4)			applyTiling( int texlayer, const NS(math,float4)& uv ) const;
	const NS(math,float4)&		getUVScale( int texlayer ) const;

	/** Returns array of meshes using this material. */
	const NS(lang,Array)<MyMesh*>&	meshUsers() const								{return m_meshusers;}

	const Param<NS(lang,String)>&	getTexture( int i ) const							{return m_textures[i];}
	const Param<NS(math,float4)>&	getVector( int i ) const							{return m_vectors[i];}
	const Param<float>&			getFloat( int i ) const								{return m_floats[i];}

private:
	Mtl*								m_mtl3ds;
	IDxMaterial*						m_dxmtl;
	int									m_index;
	NS(lang,String)						m_name;
	NS(lang,String)						m_shader;
	NS(lang,Array)< Param<NS(lang,String)> >	m_textures;
	NS(lang,Array)< Param<NS(math,float4)> >	m_vectors;
	NS(lang,Array)< Param<float> >			m_floats;
	bool								m_unlit;
	NS(lang,Array)<MyMesh*>				m_meshusers;
	NS(math,float4)						m_uvScale;
	SortType							m_sort;

	void		removeExistingTexture( const char* param );

	void		extractDxMaterial( OptionInterface* opt );
	void		extractStdMtl( Interface* ip, OptionInterface* opt );

	gr::Shader*	clone() const;
	void		setTechnique( const char* );

	void		setTexture( ParamType, gr::BaseTexture* );
	void		setTexture( const char*, gr::BaseTexture* );
	void		setMatrix( ParamType, const NS(math,float4x4)& );
	void		setMatrix( const char*, const NS(math,float4x4)& );
	void		setMatrixArray( ParamType, NS(math,float4x4)**, int );
	void		setVector( ParamType, const NS(math,float4)& );
	void		setFloat( ParamType, float );

	gr::BaseTexture*		getTexture( const char* name );
	NS(math,float4x4)			getMatrix( const char* name );
	NS(math,float4)			getVector( const char* name );
	float					getFloat( const char* name );

	int			begin();
	void		beginPass( int );
	void		endPass();
	void		end();
	SortType	sort() const;
	bool		enabled() const;
};


#endif // _MYMATERIAL_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
