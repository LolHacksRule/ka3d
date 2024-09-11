#include "StdAfx.h"
#include "MtlUtil.h"
#include "MyMesh.h"
#include "ParamBlockUtil.h"
#include "MyMaterial.h"
#include "OptionInterface.h"
#include <io/PathName.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <math/float4x4.h>
#include <math/toString.h>
#include <stdmat.h>
#include <config.h>


USING_NAMESPACE(io)
USING_NAMESPACE(lang)
USING_NAMESPACE(math)


const float4 HALF3( 0.5f, 0.5f, 0.5f, 0.5f );


MyMaterial::MyMaterial( Mtl* mtl, int index, Interface* ip,
	OptionInterface* opt, 
	const Array<MyMesh*>& meshusers ) :
	flags( SHADER_DEFAULT ),
	m_mtl3ds( mtl ),
	m_dxmtl( 0 ),
	m_index( index ),
	m_unlit( false ),
	m_meshusers( meshusers ),
	m_uvScale( 1, 1, 1, 1 ),
	m_sort( Shader::SORT_NONE )
{
	assert( mtl != 0 );

	try
	{

#ifdef MAXEXPORT_HASDXMATERIAL

		m_dxmtl = (IDxMaterial*)mtl->GetInterface( IDXMATERIAL_INTERFACE );
		if ( m_dxmtl != 0 )
			extractDxMaterial( opt );
		else
			extractStdMtl( ip, opt );

#else

		extractStdMtl( ip, opt );
#endif
	}
	catch ( Exception& e )
	{
		String origname = mtl->GetName();
		String msg = Format("{0}\n\nMaterial \"{1}\" is used by meshes:\n", e.getMessage().format(), origname).format();
		
		for ( int i = 0 ; i < m_meshusers.size() && i < 20 ; ++i )
			msg = msg + m_meshusers[i]->name() + "\n";
		if ( m_meshusers.size() > 20 )
			msg = msg + "...";

		throwError( Exception( Format("{0}", msg) ) );
	}
}

MyMaterial::~MyMaterial()
{
}

gr::Shader* MyMaterial::clone() const
{
	assert( false ); // shouldn't be used in exporter
	return new MyMaterial( *this );
}

void MyMaterial::setColor( const char* param, const Color& color )
{
	setVector( param, NS(math,float4)( color.r, color.g, color.b, 1.f ) );
}

void MyMaterial::setColor( const char* param, const AColor& color )
{
	setVector( param, NS(math,float4)( color.r, color.g, color.b, color.a ) );
}

void MyMaterial::setVector( const char* param, const NS(math,float4)& value )
{
	assert( !hasVector(param) );

	Param<NS(math,float4)> v;
	v.type = param;
	v.value = value;
	m_vectors.add( v );
}

void MyMaterial::setFloat( const char* param, float value )
{
	assert( !hasFloat(param) );

	Param<float> v;
	v.type = param;
	v.value = value;
	m_floats.add( v );
}

void MyMaterial::removeExistingTexture( const char* param )
{
	for ( int i = 0 ; i < m_textures.size() ; ++i )
	{
		if ( m_textures[i].type == param )
			m_textures.remove( i-- );
	}
}

void MyMaterial::setTexture( const char* param, BitmapTex* texmap )
{
	assert( texmap->GetName() != 0 );

	removeExistingTexture( param );

	Param<String> v;
	v.type = param;
	v.value = String(texmap->GetMapName()).toLowerCase();
	m_textures.add( v );
}

void MyMaterial::setTexture( const char* param, PBBitmap* bmp )
{
	assert( bmp );
	assert( 0 != bmp->bi.Name() );

	removeExistingTexture( param );

	Param<String> v;
	v.type = param;
	v.value = bmp->bi.Name();
	m_textures.add( v );
}

void MyMaterial::setTexture( const char* param, const String& name )
{
	removeExistingTexture( param );

	Param<String> v;
	v.type = param;
	v.value = name;
	m_textures.add( v );
}

bool MyMaterial::hasTexture( const char* typestr ) const
{
	for ( int i = 0 ; i < m_textures.size() ; ++i )
		if ( m_textures[i].type == typestr )
			return true;
	return false;
}

bool MyMaterial::hasVector( const char* typestr ) const
{
	for ( int i = 0 ; i < m_vectors.size() ; ++i )
		if ( m_vectors[i].type == typestr )
			return true;
	return false;
}

bool MyMaterial::hasFloat( const char* typestr ) const
{
	for ( int i = 0 ; i < m_floats.size() ; ++i )
		if ( m_floats[i].type == typestr )
			return true;
	return false;
}

String MyMaterial::getTextureName( const char* typestr ) const
{
	for ( int i = 0 ; i < m_textures.size() ; ++i )
		if ( m_textures[i].type == typestr )
			return m_textures[i].value;
	return "";
}

gr::BaseTexture* MyMaterial::getTexture( const char* /*name*/ )
{
	assert( false );
	return 0;
}

NS(math,float4x4) MyMaterial::getMatrix( const char* /*name*/ )
{
	assert( false );
	return NS(math,float4x4)(0);
}

NS(math,float4) MyMaterial::getVector( const char* /*name*/ )
{
	assert( false );
	return NS(math,float4)(0,0,0,0);
}

float MyMaterial::getFloat( const char* /*name*/ )
{
	assert( false );
	return 0;
}

void MyMaterial::setTechnique( const char* )
{
	assert( false );
}

void MyMaterial::setTexture( const char*, gr::BaseTexture* ) 
{
	assert( false );
}

void MyMaterial::setTexture( ParamType, gr::BaseTexture* ) 
{
	assert( false );
}

void MyMaterial::setMatrix( const char*, const NS(math,float4x4)& ) 
{
	assert( false );
}

void MyMaterial::setMatrix( ParamType, const NS(math,float4x4)& )
{
	assert( false );
}

void MyMaterial::setMatrixArray( ParamType, NS(math,float4x4)**, int ) 
{
	assert( false );
}

void MyMaterial::setVector( ParamType, const NS(math,float4)& ) 
{
	assert( false );
}

void MyMaterial::setFloat( ParamType, float )
{
	assert( false );
}

int MyMaterial::begin()
{
	assert( false );
	return 0;
}

void MyMaterial::beginPass( int ) 
{
	assert( false );
}

void MyMaterial::endPass() 
{
	assert( false );
}

void MyMaterial::end()
{
	assert( false );
}

MyMaterial::SortType MyMaterial::sort() const
{
	return m_sort;
}

bool MyMaterial::enabled() const
{
	return false;
}

void MyMaterial::extractStdMtl( Interface* ip, OptionInterface* opt )
{
	assert( m_mtl3ds );
	if ( !m_mtl3ds->GetName() )
		throwError( Exception( Format("All used materials need to have defined names") ) );
	String origname = m_mtl3ds->GetName();

	Debug::printf( "Extracting Standard material \"%s\"", origname.c_str() );

	// check for shader tag
	String shadertag = "FX=";
	int shaderix = origname.indexOf(shadertag);
	if ( shaderix == -1 )
	{
		m_name = origname;
		m_shader = "";
	}
	else
	{
		if ( -1 == shaderix )
			throwError( Exception( Format("'{1}<shader name>' tag not found from material \"{0}\" name. Every used material needs to have a defined shader.", origname, shadertag) ) );
		if ( -1 != origname.indexOf(".fx",shaderix) )
			throwError( Exception( Format("'{1}<shader name>' tag cannot contain .fx file extension as in material \"{0}\".", origname, shadertag) ) );
		m_name = origname.substring( 0, shaderix ).trim();
		m_shader = origname.substring( shaderix+shadertag.length(), origname.length() );
		if ( -1 != m_shader.indexOf(' ') )
			throwError( Exception( Format("Shader name cannot contain spaces as material \"{0}\".", m_shader, origname) ) );
	}

	// extract Standard Material
	if ( m_mtl3ds->ClassID() != Class_ID(DMTL_CLASS_ID,0) )
		throwError( Exception( Format("Material \"{0}\" is not 3dsmax Standard Material. Only Standard and DirectX Shader materials can be exported.", origname) ) );
	StdMat* stdmtl = static_cast<StdMat*>( m_mtl3ds );
	
	// basic shader parameters
	if ( stdmtl->GetTwoSided() )
	{
		Debug::printf( "Material %s is 2-sided\n", origname.c_str() );
		flags |= SHADER_TWOSIDED;
	}

	// extract base color map
	BitmapTex* texmap = MtlUtil::getBitmapTex( stdmtl, ID_DI );
	bool difftexalpha = false;
	if ( 0 != texmap )
	{
		setTexture( "BASEMAP", texmap );

		// check if we have alpha channel in use
		difftexalpha = (texmap->GetAlphaSource() != ALPHA_NONE);

		// check that UV offset/tiling has not been used
		StdUVGen* uvgen = texmap->GetUVGen();
		if ( uvgen )
		{
			float uscale = uvgen->GetUScl(0);
			float vscale = uvgen->GetVScl(0);
			m_uvScale.x = uscale;
			m_uvScale.y = vscale;
		}
	}

	// extract specular exponent
	float spec = stdmtl->GetShinStr( 0 ) * 100.f;
	Debug::printf( "Specular exponent of %s: %g\n", origname.c_str(), spec );
	setFloat( "SHININESS", spec );

	// extract bump map (to be used as normal map)
	texmap = MtlUtil::getBitmapTex( stdmtl, ID_BU );
	if ( 0 != texmap )
		setTexture( "NORMALMAP", texmap );

	// extract reflection map
	texmap = MtlUtil::getBitmapTex( stdmtl, ID_RL );
	if ( 0 != texmap )
		setTexture( "REFLMAP", texmap );

	// extract ambient color
	Color amb = stdmtl->GetAmbient(0);
	Interval interval;
	amb *= ip->GetAmbient( 0, interval );
	setColor( "AMBIENTC", amb );

	// extract diffuse color
	setColor( "DIFFUSEC", stdmtl->GetDiffuse(0) );

	// extract specular color
	setColor( "SPECULARC", stdmtl->GetSpecular(0) );

	// set default shader if not any
	if ( m_shader == "" )
	{
		// find out default shader name by map combination
		if ( hasTexture("NORMALMAP") && hasTexture("BASEMAP") )
		{
			if ( hasTexture("REFLMAP") )
				m_shader = opt->getOptionString( "DefaultBumpReflShader" );
			else
				m_shader = opt->getOptionString( "DefaultBumpTextureShader" );
		}
		else if ( hasTexture("BASEMAP") )
		{
			if ( difftexalpha )
				m_shader = opt->getOptionString( "DefaultDiffuseTextureAlphaShader" );
			else
				m_shader = opt->getOptionString( "DefaultDiffuseTextureShader" );
		}
		else
		{
			m_shader = opt->getOptionString( "DefaultPlainShader" );
		}

		// check if users are skinned (CRASHES????)
		bool skinned = false;
		for ( int i = 0 ; i < m_meshusers.size() ; ++i )
		{
			assert( m_meshusers[i] != 0 );
			if ( m_meshusers[i]->isSkinned() )
			{
				skinned = true;
				break;
			}
		}
		if ( skinned && m_shader != "" && !m_shader.startsWith("s-") )
		{
			m_shader = "s-" + m_shader;
		}

		// abort export if no shader defined
		if ( m_shader == "" )
			throwError( Exception( Format("No default shader set but material {0} has no shader!", origname) ) );
		Debug::printf( "  Material \"%s\" uses default shader \"%s\"\n", m_name.c_str(), m_shader.c_str() );
	}
}

float4 MyMaterial::applyTiling( int texlayer, const float4& uv ) const
{
	if ( texlayer == 0 )
		return (uv - HALF3) * m_uvScale + HALF3;
	else
		return uv;
}

const float4& MyMaterial::getUVScale( int texlayer ) const
{
	static const float4 ID4(1,1,1,1);
	if ( texlayer == 0 )
		return m_uvScale;
	else
		return ID4;
}

#ifdef MAXEXPORT_HASDXMATERIAL
void MyMaterial::extractDxMaterial( OptionInterface* /*opt*/ )
{
	Debug::printf( "Extracting DX material %s\n", (char*)m_mtl3ds->GetName() );

	String				fxfilename	= m_dxmtl->GetEffectFilename();
	int					fxbitmaps	= m_dxmtl->GetNumberOfEffectBitmaps();
	IParamBlock2*		paramblock	= m_mtl3ds->GetParamBlock(0);
	int					numparams	= paramblock->NumParams();
	ParamBlockDesc2*	desc		= paramblock->GetDesc();

	// extract basics
	m_name = String( m_mtl3ds->GetName() );
	m_shader = fxfilename;

	// extract parameters
	for ( int i = 0 ; i < desc->Count() ; ++i )
	{
		ParamID		paramid		= desc->IndextoID( i );
		ParamDef&	paramdef	= desc->GetParamDef( paramid );

		if ( paramdef.type == TYPE_BITMAP )
		{
			PBBitmap* bmp = paramblock->GetBitmap( paramid );
			assert( bmp );
			assert( bmp->bi.Name() );
			setTexture( paramdef.int_name, bmp );
		}
		else if ( paramdef.type == TYPE_FRGBA )
		{
			setColor( paramdef.int_name, paramblock->GetAColor(paramid) );
		}
		else if ( paramdef.type == TYPE_FLOAT )
		{
			setFloat( paramdef.int_name, paramblock->GetFloat(paramid) );
		}
		else
		{
			Debug::printf( "  Skipped parameter %s, type %s (%d)\n", paramdef.int_name, ParamBlockUtil::toString(paramdef.type), (int)paramdef.type );
		}
	}
}
#endif

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
