#ifndef _MYDUMMY_H
#define _MYDUMMY_H


#include "MyNode.h"
#include <hgr/Dummy.h>


class MyDummy :
	public MyNode
{
public:
	explicit MyDummy( INode* node3ds );
	~MyDummy();

	hgr::Dummy*		node()	{return static_cast<hgr::Dummy*>(MyNode::node());}

private:
	MyDummy( const MyDummy& );
	MyDummy& operator=( const MyDummy& );
};


#endif // _MYDUMMY_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
