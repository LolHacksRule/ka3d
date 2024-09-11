#include "StdAfx.h"
#include "SkinUtil.h"
#include "INodeUtil.h"
#include <modstack.h>
#include <algorithm>
#include <config.h>


USING_NAMESPACE(lang)


Modifier* SkinUtil::findSkinModifier( INode* node )
{
	// Get object from node. Abort if no object.
	::Object* obj = node->GetObjectRef();
	if ( !obj )
		return 0;

	// print modifier stack
	while (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		IDerivedObject* derivedobj = static_cast<IDerivedObject*>(obj);
		for ( int modstackix = 0 ; modstackix < derivedobj->NumModifiers() ; ++modstackix )
			Modifier* mod = derivedobj->GetModifier(modstackix);
		obj = derivedobj->GetObjRef();
	}

	// Is derived object ?
	obj = node->GetObjectRef();
	while (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)
	{
		// Yes -> Cast.
		IDerivedObject* derivedobj = static_cast<IDerivedObject*>(obj);

		// Iterate over all entries of the modifier stack.
		for ( int modstackix = 0 ; modstackix < derivedobj->NumModifiers() ; ++modstackix )
		{
			// Get current modifier.
			Modifier* mod = derivedobj->GetModifier(modstackix);

			// Is this Skin ?
			if ( mod->ClassID() == SKIN_CLASSID )
			{
				// Yes -> Exit.
				return mod;
			}
		}
		obj = derivedobj->GetObjRef();
	}
	return 0;
}

void SkinUtil::listBones( ISkin* skin, Array<INode*>& bones )
{
	bones.clear();
	for ( int i = 0 ; i < skin->GetNumBones() ; ++i )
		bones.add( skin->GetBone(i) );

	std::sort( bones.begin(), bones.end(), INodeUtil::SortByName() );
}

void SkinUtil::addWeights( ExportWeightMap* vmap, Array<P(ExportVertex)>& vertexlist,
	INode* node, ISkin* skin, ISkinContextData* skincx )
{
	assert( skincx->GetNumPoints() == vertexlist.size() );

	for ( int i = 0 ; i < skincx->GetNumPoints() ; ++i )
	{
		int bonec = skincx->GetNumAssignedBones(i);
		for ( int k = 0 ; k < bonec ; ++k )
		{
			int boneidx = skincx->GetAssignedBone( i, k );
			INode* bone = skin->GetBone( boneidx );
			assert( bone );
			if ( bone == node )
			{
				float w = skincx->GetBoneWeight( i, k );
				if ( w > 0.f )
					vmap->addWeight( vertexlist[i], w );
			}
		}
	}
}

Matrix3 SkinUtil::getInitBoneTM( INode* node, ISkin* skin )
{
	Matrix3 tm;
	int ok = skin->GetBoneInitTM( node, tm );
	assert( ok == SKIN_OK );
	return tm;
}

Matrix3 SkinUtil::getInitSkinTM( INode* node, ISkin* skin )
{
	Matrix3 tm;
	int ok = skin->GetSkinInitTM( node, tm );
	assert( ok == SKIN_OK );
	return tm;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
