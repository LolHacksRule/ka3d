#ifndef _MYSHAPES_H
#define _MYSHAPES_H


#include "MyNode.h"
#include <hgr/Lines.h>
#include <lang/Array.h>


class MyShape :
	public MyNode
{
public:
	explicit MyShape( INode* node3ds );

	hgr::Lines*			node()						{return static_cast<hgr::Lines*>(MyNode::node());}
	const hgr::Lines*	node() const				{return static_cast<const hgr::Lines*>(MyNode::node());}
};



#endif // _MYSHAPES_H
