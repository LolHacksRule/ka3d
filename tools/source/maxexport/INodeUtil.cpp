#include "StdAfx.h"
#include "INodeUtil.h"
#include "Matrix3Util.h"
#include <lang/Math.h>
#include <config.h>


// class for all biped controllers except the root and the footsteps
#define BIPSLAVE_CONTROL_CLASS_ID Class_ID(0x9154,0)
// class for the center of mass,  biped root controller
#define BIPBODY_CONTROL_CLASS_ID Class_ID(0x9156,0) 
// class for the biped footstep controller
#define FOOTPRINT_CLASS_ID Class_ID(0x3011,0)        


USING_NAMESPACE(lang)


bool INodeUtil::isSplitNeeded( INode* node3ds )
{
#ifdef MAXEXPORT_SPLIT_BIPED
	INode* parent3ds = node3ds->GetParentNode();
	return 0 != parent3ds &&
		String(node3ds->GetName()).toLowerCase().indexOf("spine") >= 0 && 
		String(parent3ds->GetName()).toLowerCase().indexOf("pelvis") >= 0;
#else
	return false;
#endif
}

NS(math,float3x4) INodeUtil::getModelTransformLH( INode* node3ds, TimeValue time )
{
	Matrix3 tm = getNodeTM( node3ds, time );
	INode* parent3ds = node3ds->GetParentNode();
	if ( parent3ds != 0 )
	{
		// re-parent biped spine to root instead of pelvis
		// to break dependency between lower body and upper body
		if ( isSplitNeeded(node3ds) )
		{
			parent3ds = parent3ds->GetParentNode();
			if ( 0 != parent3ds )
			{
				if ( parent3ds->IsRootNode() )
				{
					tm *= RotateXMatrix( -Math::PI*.5f );
					return Matrix3Util::toLH( tm );
				}
				else
				{
					parent3ds = parent3ds->GetParentNode();
				}
			}
		}

		tm = tm * Inverse( getNodeTM(parent3ds,time) );
	}
	return Matrix3Util::toLH( tm );
}

Matrix3 INodeUtil::getVertexTransform( INode* node3ds )
{
	Matrix3 vertextm(1);
	vertextm.PreTranslate( node3ds->GetObjOffsetPos() );
	PreRotateMatrix( vertextm, node3ds->GetObjOffsetRot() );
	ApplyScaling( vertextm, node3ds->GetObjOffsetScale() );
	return vertextm * Matrix3Util::CONVTM;
}

bool INodeUtil::hasNegativeScaling( INode* node3ds )
{
	Matrix3 vertextm(1);
	vertextm.PreTranslate( node3ds->GetObjOffsetPos() );
	PreRotateMatrix( vertextm, node3ds->GetObjOffsetRot() );
	ApplyScaling( vertextm, node3ds->GetObjOffsetScale() );
	return DotProd( CrossProd(vertextm.GetRow(0), vertextm.GetRow(1)), vertextm.GetRow(2) ) < 0.f;
}

bool INodeUtil::isBiped( INode* node3ds )
{
	assert( node3ds );
	Control* c = node3ds->GetTMController();
	return 0 != c &&
		c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID ||
		c->ClassID() == BIPBODY_CONTROL_CLASS_ID ||
		c->ClassID() == FOOTPRINT_CLASS_ID;
}

bool INodeUtil::isExportableGeometry( INode* node3ds, const ObjectState& objstate )
{
	assert( node3ds );
	assert( objstate.obj );

	Class_ID cid = objstate.obj->ClassID();
	bool isbiped = isBiped( node3ds );
	bool isgeom = objstate.obj->SuperClassID() == GEOMOBJECT_CLASS_ID;
	bool istarget = (cid == Class_ID(TARGET_CLASS_ID,0)) != 0;
	bool isbone = ((cid == Class_ID(BONE_CLASS_ID,0)) != 0 || cid == BONE_OBJ_CLASSID);
	bool isdummy = (cid == Class_ID(DUMMY_CLASS_ID,0)) != 0;
	return isgeom && !istarget && !isbiped && !isbone && !isdummy;
}

void INodeUtil::getNodesToExport( Interface* i, DWORD options, Array<INode*>& nodes3ds )
{
	nodes3ds.clear();
	
	if ( SCENE_EXPORT_SELECTED & options )
	{
		for ( int k = 0 ; k < i->GetSelNodeCount() ; ++k )
		{
			INode* node3ds = i->GetSelNode(k);
			if ( node3ds && !isHidden(node3ds) )
				nodes3ds.add( node3ds );
		}
	}
	else
	{
		INode* root = i->GetRootNode();
		if ( root && !isHidden(root) )
			listNodes( root, nodes3ds );
	}
}

void INodeUtil::getTriObjectFromNode( INode* node3ds, TimeValue time, bool* deleteit, TriObject** triobj )
{
	TriObject* tri = 0;
	bool needdel = false;

	if ( node3ds != 0 )
	{
		::Object* obj = node3ds->EvalWorldState(time).obj;

		if ( obj )
		{
			if ( obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)) ) 
			{ 
				tri = static_cast<TriObject*>( obj->ConvertToType( time, Class_ID(TRIOBJ_CLASS_ID,0) ) );

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				if ( obj != tri ) 
					needdel = true;
			}
		}
	}

	*deleteit = needdel;
	*triobj = tri;
}

void INodeUtil::listNodes( INode* parent3ds, Array<INode*>& nodes3ds )
{
	nodes3ds.add( parent3ds );

	for ( int k = 0 ; k < parent3ds->NumberOfChildren() ; ++k )
	{
		INode* child3ds = parent3ds->GetChildNode( k );
		if ( child3ds && !isHidden(child3ds) )
			listNodes( child3ds, nodes3ds );
	}
}

Matrix3 INodeUtil::getNodeTM( INode* node3ds, TimeValue t )
{
	Matrix3 tm = node3ds->GetNodeTM( t );
	if ( node3ds->IsRootNode() )
		tm *= RotateXMatrix( Math::PI*.5f );
	return tm;
}

String INodeUtil::getUserPropertyBuffer( INode* node3ds )
{
	TSTR str;
	node3ds->GetUserPropBuffer( str );
	Array<char> buf;
	for ( int i = 0 ; i < str.Length() ; ++i )
		buf.add( str[i] );
	buf.add( 0 );
	return String( buf.begin() );
}

bool INodeUtil::isHidden( INode* node3ds )
{
	assert( node3ds );

	if ( !node3ds->IsHidden() )
		return false;

	for ( int k = 0 ; k < node3ds->NumberOfChildren() ; ++k )
	{
		INode* child3ds = node3ds->GetChildNode( k );
		if ( child3ds && !isHidden(child3ds) )
			return false;
	}

	return true;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
