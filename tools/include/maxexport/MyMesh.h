#ifndef _MYMESH_H
#define _MYMESH_H


#include "MyNode.h"
#include "MyMaterial.h"
#include "MyGeometry.h"
#include "MyPrimitive.h"
#include <hgr/Mesh.h>
#include <lang/Array.h>


class MyMesh :
	public MyNode
{
public:
	enum ExportVertexColors { DONT_EXPORT_VERTEX_COLORS, EXPORT_VERTEX_COLORS };
	
	explicit MyMesh( INode* node3ds );
	~MyMesh();

	void		extractMtls( NS(lang,Array)<Mtl*>& mtls );
	void		extractBones( const NS(lang,Array)<MyNode*>& nodes );
	void		extractGeometry( const NS(lang,Array)<P(MyMaterial)>& materials, ExportVertexColors exportvertexcolors );
	void		extractPrimitives( int maxbonesperprimitive );

	void				removePrimitive( int i )	{node()->removePrimitive(i);}
	hgr::Mesh*			node()						{return static_cast<hgr::Mesh*>(MyNode::node());}
	const hgr::Mesh*	node() const				{return static_cast<const hgr::Mesh*>(MyNode::node());}
	int					primitives() const			{return node()->primitives();}
	MyPrimitive*		getPrimitive( int i ) const	{return static_cast<MyPrimitive*>( node()->getPrimitive(i) );}
	MyGeometry*			geometry() const			{return m_geom;}
	bool				isSkinned() const;
	NS(math,float4)		calculateScaleBias() const;

	static NS(math,float4)	getScaleBias( const NS(math,float4)* v, int count );
	static void				getMinMax( const NS(math,float4)* v, int count, NS(math,float4)* minv, NS(math,float4)* maxv );

private:
	class NodeDepthLess
	{
	public:
		bool operator()( hgr::Node* a, hgr::Node* b ) const {return a->depth() < b->depth();}
	};

	P(MyGeometry)	m_geom;

	Mesh*		getMesh( std::auto_ptr<TriObject>* tridel );

	MyMesh( const MyMesh& );
	MyMesh& operator=( const MyMesh& );
};


#endif // _MYMESH_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
