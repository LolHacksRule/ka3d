#ifndef _MYPRIMITIVE_H
#define _MYPRIMITIVE_H


#include <gr/impl/DIPrimitive.h>
#include <lang/Array.h>
#include <lang/Object.h>
#include <math/float4.h>
#include <stdint.h>


class MyMaterial;
class MyGeometry;


class MyPrimitive :
	public gr::DIPrimitive
{
public:
	/**
	 * Note: This scalebias contains maximum component of vertex(min-max) in x and vertex(min) in yzw
	 */
	MyPrimitive( MyMaterial* mat, MyGeometry* geom, math::float4 scalebias );
	MyPrimitive( MyMaterial* mat, MyGeometry* geom, const NS(lang,Array)<int>& faces, math::float4 scalebias );
	~MyPrimitive();

	void		splitPrimitive( int maxbones, P(MyPrimitive)* out1, P(MyPrimitive)* out2 );

	MyMaterial*	material() const;

private:
	MyMaterial*		m_mat;
	MyGeometry*		m_geom;
	NS(math,float4)	m_scalebias;

	void		create( MyMaterial* mat, MyGeometry* geom, const NS(lang,Array)<int>& faces );
	void		getBonesUsedByFaces( const NS(lang,Array)<int>& faces, NS(lang,Array)<int>& bones );
	void		getVerticesUsedByFaces( const NS(lang,Array)<int>& faces, NS(lang,Array)<int>& bones );

	void		render()						{}
	void		setShader( gr::Shader* );
	gr::Shader*	shader() const;
	PrimType	type() const;

	void		setPackedVertexPositions( const NS(math,float4)* v, int count );
	void		setPackedVertexNormals( const NS(math,float4)* v, int count );

	static void				get_V3_16( const NS(math,float4)* v, int count, int16_t* out, NS(math,float4)* scalebiasout, NS(math,float4) scalebias );
	static void				get_V2_4_12( const NS(math,float4)* v, int count, int16_t* out, NS(math,float4)* scalebiasout );
	static void				get_V3_8( const NS(math,float4)* v, int count, int8_t* out, NS(math,float4)* scalebiasout, NS(math,float4) scalebias );
	static void				get_V3_8_Normal( const NS(math,float4)* v, int count, int8_t* out, NS(math,float4)* scalebiasout );

	MyPrimitive( const MyPrimitive& );
	MyPrimitive& operator=( const MyPrimitive& );
};


#endif // _MYPRIMITIVE_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
