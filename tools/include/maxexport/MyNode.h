#ifndef _MYNODE_H
#define _MYNODE_H


#include <hgr/Node.h>
#include <lang/Object.h>
#include <lang/String.h>


namespace hgr {
	class TransformAnimation;}


class MyNode :
	public lang::Object
{
public:
	P(hgr::TransformAnimation)	tmanim;
	NS(lang,String)				userprop;
	bool						castShadows;
	bool						receiveShadows;

	MyNode( INode* node3ds, hgr::Node* node );
	~MyNode();

	hgr::Node*			node();
	INode*				node3ds() const;
	const NS(lang,String)&	name() const;
	const hgr::Node*	node() const;

private:
	INode*			m_node3ds;
	P(hgr::Node)	m_node;

	MyNode();
	MyNode( const MyNode& );
	MyNode& operator=( const MyNode& );
};


#endif // _MYNODE_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
