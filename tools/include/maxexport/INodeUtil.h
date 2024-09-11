#ifndef _INODEUTIL_H
#define _INODEUTIL_H


#include <lang/Array.h>
#include <lang/String.h>
#include <math/float3x4.h>
#include <string.h>


/** 
 * Utilities for 3dsmax INode handling.
 */
class INodeUtil
{
public:
	/** Used with std::sort to sort nodes by name. */
	class SortByName
	{
	public:
		bool operator()( INode* a, INode* b ) const {return strcmp(a->GetName(),b->GetName()) < 0;}
	};

	/** 
	 * Returns true if the node should be parented to root.
	 * Used to detect need for splitting lower body from upper body.
	 */
	static bool				isSplitNeeded( INode* node3ds );

	/** 
	 * Returns true if the node matrix has negative scaling. 
	 */
	static bool				hasNegativeScaling( INode* node3ds );

	/** 
	 * Returns engine model-to-parent transformation. 
	 * @param node3ds Source 3dsmax node for transformation.
	 * @param time Time of transformation.
	 */
	static NS(math,float3x4)	getModelTransformLH( INode* node3ds, TimeValue time );

	/** 
	 * Returns object transformation which should be applied to 3dsmax 
	 * vertices to convert them to engine conventions.
	 * Transforming transformed vertices with getModelToParentLH()
	 * will give coordinates in parent space.
	 */
	static Matrix3	getVertexTransform( INode* node3ds );

	/** 
	 * Is node Character Studio biped or not? 
	 */
	static bool		isBiped( INode* node3ds );

	/** 
	 * Tests if specified node/object state contains exportable geometry. 
	 */
	static bool		isExportableGeometry( INode* node3ds, const ObjectState& objstate );

	/** 
	 * Collects nodes from the scene. 
	 * Clears node3ds container before adding the nodes.
	 * @param nodes3ds [out] Receives nodes to export.
	 */
	static void		getNodesToExport( Interface* ip, DWORD options, NS(lang,Array)<INode*>& nodes3ds );

	/** 
	 * Appends hierarchy of INodes to array, parents first.
	 */
	static void		listNodes( INode* parent3ds, NS(lang,Array)<INode*>& nodes );

	/**
	 * Return a pointer to a TriObject given an INode or return 0
	 * if the node3ds cannot be converted to a TriObject.
	 * @param deleteit [out] Receives true if the triobj should be deleted after use.
	 * @param triobj [out] Receives the triangle object.
	 */
	static void		getTriObjectFromNode( INode* node3ds, TimeValue time, bool* deleteit, TriObject** triobj );

	/**
	 * Returns true if node, and all of it's child nodes, are hidden.
	 */
	static bool		isHidden( INode* node3ds );

	/**
	 * Returns node User Property text buffer or "" if empty.
	 */
	static NS(lang,String)		getUserPropertyBuffer( INode* node3ds );

private:
	static Matrix3	getNodeTM( INode* node3ds, TimeValue t );
};


#endif // _INODEUTIL_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
