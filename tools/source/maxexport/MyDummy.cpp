#include "StdAfx.h"
#include "MyDummy.h"
#include "INodeUtil.h"
#include <lang/Math.h>
#include <dummy.h>
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


MyDummy::MyDummy( INode* node3ds ) :
	MyNode( node3ds, new hgr::Dummy )
{
	ObjectState objstate = node3ds->EvalWorldState( 0 );
	assert( 0 != objstate.obj );
	assert( Class_ID(DUMMY_CLASS_ID,0) == objstate.obj->ClassID() );

	DummyObject* dummyobj = static_cast<DummyObject*>( objstate.obj );

	Point3 pmin = dummyobj->GetBox().pmin;
	Point3 pmax = dummyobj->GetBox().pmax;
	Matrix3 vertextm = INodeUtil::getVertexTransform( node3ds );
	pmin = vertextm * pmin;
	pmax = vertextm * pmax;

	for ( int i = 0 ; i < 3 ; ++i )
	{
		float minv = Math::min( pmin[i], pmax[i] );
		float maxv = Math::max( pmin[i], pmax[i] );
		pmin[i] = minv;
		pmax[i] = maxv;
	}

	node()->setBox( float3(pmin.x,pmin.y,pmin.z), float3(pmax.x,pmax.y,pmax.z) );
}

MyDummy::~MyDummy()
{
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
