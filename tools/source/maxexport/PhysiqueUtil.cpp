#include "StdAfx.h"
#include "INodeUtil.h"
#include "PhysiqueUtil.h"
#include <lang/Float.h>
#include <Phyexp.h>
#include <Bipexp.h>
#include <modstack.h>
#include <algorithm>
#include <config.h>


USING_NAMESPACE(lang)


Modifier* PhysiqueUtil::findPhysiqueModifier( INode* node3ds )
{
	// Get object from node3ds. Abort if no object.
	::Object* obj = node3ds->GetObjectRef();
	if ( !obj ) 
		return 0;

	// Is derived object ?
	while ( obj->SuperClassID() == GEN_DERIVOB_CLASS_ID && obj )
	{
		// Yes -> Cast.
		IDerivedObject* derivedobj = static_cast<IDerivedObject*>( obj );
						
		// Iterate over all entries of the modifier stack.
		for ( int modstackix = 0 ; modstackix < derivedobj->NumModifiers() ; ++modstackix )
		{
			// Get current modifier.
			Modifier* modp = derivedobj->GetModifier( modstackix );

			// Is this Physique ?
			if ( modp->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B) )
			{
				// Yes -> Exit.
				return modp;
			}
		}
		obj = derivedobj->GetObjRef();
	}

	// Not found.
	return 0;
}

void PhysiqueUtil::listBones( IPhyContextExport* phycontextexp, Array<INode*>& bones )
{
	bones.clear();
	int vertices = phycontextexp->GetNumberVertices();
	for ( int i = 0 ; i < vertices ; ++i )
	{
		// rigid vertex / rigid blended vertex?
		IPhyVertexExport* vi = phycontextexp->GetVertexInterface( i );
		IPhyFloatingVertex* fv = static_cast<IPhyFloatingVertex*>( phycontextexp->GetFloatingVertexInterface(i) );
		
		if ( 0 != vi && 0 == fv )
		{
			switch ( vi->GetVertexType() )
			{
			case RIGID_TYPE:{
				IPhyRigidVertex* rv = static_cast<IPhyRigidVertex*>( vi );
				INode* bone = rv->GetNode();
				if ( bones.end() == std::find(bones.begin(),bones.end(),bone) ) 
					bones.add(bone);
				break;}

			case (RIGID_TYPE|BLENDED_TYPE):{
				IPhyBlendedRigidVertex* rbv = static_cast<IPhyBlendedRigidVertex*>( vi );
				for ( int x = 0 ; x < rbv->GetNumberNodes() ; ++x ) 
				{
					INode* bone = rbv->GetNode( x );
					if ( bones.end() == std::find(bones.begin(),bones.end(),bone) ) 
						bones.add(bone);
				}
				break;}

			default:{
				assert( false );
				break;}
			}
		}
		else if ( fv ) // floating vertex
		{
			for ( int x = 0; x < fv->GetNumberNodes() ; ++x )
			{
				INode* bone = fv->GetNode( x );
				if ( bones.end() == std::find(bones.begin(),bones.end(),bone) ) 
					bones.add(bone);
			}
		}
		else
		{
			assert( false );
		}

		if ( fv )
			phycontextexp->ReleaseVertexInterface( fv );
		if ( vi )
			phycontextexp->ReleaseVertexInterface( vi );
	}

	std::sort( bones.begin(), bones.end(), INodeUtil::SortByName() );
}

void PhysiqueUtil::addWeights( ExportWeightMap* vmap, Array<P(ExportVertex)>& vertexlist, INode* node3ds, IPhyContextExport* phycontextexp )
{
	assert( phycontextexp->GetNumberVertices() == vertexlist.size() );

	int vertices = phycontextexp->GetNumberVertices();
	for ( int i = 0 ; i < vertices ; ++i )
	{
		// rigid vertex / rigid blended vertex?
		IPhyVertexExport* vi = phycontextexp->GetVertexInterface( i /*,HIERARCHIAL_VERTEX*/ );
		IPhyFloatingVertex* fv = static_cast<IPhyFloatingVertex*>( phycontextexp->GetFloatingVertexInterface(i) );

		if ( vi && !fv )
		{
			switch ( vi->GetVertexType() )
			{
			case RIGID_TYPE:{
				IPhyRigidVertex* rv = static_cast<IPhyRigidVertex*>( vi );
				INode* bone = rv->GetNode();
				if ( bone == node3ds )
					vmap->addWeight( vertexlist[i], 1.f );
				break;}

			case (RIGID_TYPE|BLENDED_TYPE):{
				IPhyBlendedRigidVertex* rbv = static_cast<IPhyBlendedRigidVertex*>( vi );
				for ( int x = 0 ; x < rbv->GetNumberNodes() ; ++x ) 
				{
					INode* bone = rbv->GetNode( x );
					if ( bone == node3ds )
						vmap->addWeight( vertexlist[i], rbv->GetWeight(x) );
				}
				break;}

			default:{
				assert( false );
				break;}
			}
		}
		else if ( fv )
		{
			for ( int x = 0; x < fv->GetNumberNodes() ; ++x )
			{
				INode* bone = fv->GetNode( x );
				if ( bone == node3ds )
				{
					float total;
					float weight = fv->GetWeight( x, total );
					vmap->addWeight( vertexlist[i], weight );
				}
			}
		}
		else
		{
			assert( false );
		}

		if ( fv )
			phycontextexp->ReleaseVertexInterface( fv );
		if ( vi )
			phycontextexp->ReleaseVertexInterface( vi );
	}
}

Matrix3 PhysiqueUtil::getInitBoneTM( INode* node3ds, IPhysiqueExport* phyExport )
{
	return node3ds->GetNodeTM(0);
}

Matrix3 PhysiqueUtil::getInitSkinTM( INode* node3ds, IPhysiqueExport* phyExport )
{
	return node3ds->GetNodeTM(0);
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
