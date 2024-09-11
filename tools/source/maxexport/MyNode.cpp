#include "StdAfx.h"
#include "MyNode.h"
#include "INodeUtil.h"
#include <io/PropertyParser.h>
#include <hgr/TransformAnimation.h>
#include <config.h>


USING_NAMESPACE(io)


MyNode::MyNode( INode* node3ds, hgr::Node* node ) :
	m_node3ds( node3ds ),
	m_node( node ),
	castShadows( false ),
	receiveShadows( false )
{
	assert( node3ds != 0 );
	assert( node != 0 );

	m_node->setTransform( INodeUtil::getModelTransformLH(node3ds,0) );

	node->setName( node3ds->GetName() );
	userprop = INodeUtil::getUserPropertyBuffer( node3ds ).trim();

	castShadows = (node3ds->CastShadows() != 0);
	receiveShadows = (node3ds->RcvShadows() != 0);
}

MyNode::~MyNode()
{
}

hgr::Node* MyNode::node()
{
	assert( m_node != 0 );
	return m_node;
}

INode* MyNode::node3ds() const
{
	assert( m_node3ds != 0 );
	return m_node3ds;
}

const NS(lang,String)&	MyNode::name() const
{
	assert( m_node != 0 );
	return m_node->name();
}

const hgr::Node* MyNode::node() const		
{
	assert( m_node != 0 );
	return m_node;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
