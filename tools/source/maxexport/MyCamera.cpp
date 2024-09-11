#include "StdAfx.h"
#include "MyCamera.h"
#include <config.h>


MyCamera::MyCamera( INode* node3ds ) :
	MyNode( node3ds, new NS(hgr,Camera) )
{
	ObjectState objstate = node3ds->EvalWorldState( 0 );
	assert( objstate.obj );
	assert( CAMERA_CLASS_ID == objstate.obj->SuperClassID() );

	GenCamera* cameraobj = static_cast<GenCamera*>( objstate.obj );
	Interval validrange(0,0);
	CameraState camerastate;
	int cameraevalresult = cameraobj->EvalCameraState( 0, validrange, &camerastate );
	assert( REF_SUCCEED == cameraevalresult );
	
	node()->setHorizontalFov( camerastate.fov );
}

MyCamera::~MyCamera()
{
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
