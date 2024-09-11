#ifndef _MYCAMERA_H
#define _MYCAMERA_H


#include "MyNode.h"
#include <hgr/Camera.h>


class MyCamera :
	public MyNode
{
public:
	explicit MyCamera( INode* node3ds );
	~MyCamera();

	NS(hgr,Camera)*	node()	{return static_cast<NS(hgr,Camera)*>(MyNode::node());}

private:
	MyCamera( const MyCamera& );
	MyCamera& operator=( const MyCamera& );
};


#endif // _MYCAMERA_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
