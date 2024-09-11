#ifndef _SKINUTIL_H
#define _SKINUTIL_H


#include "ExportWeightMap.h"
#include <lang/Array.h>
#include <iparamb2.h>
#include <iskin.h>


/** 
 * Utilities for 3dsmax Skin Modifier exporting. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class SkinUtil
{
public:
	/** 
	 * Find if a given node contains a Skin Modifier.
	 * Use GetInterface(I_SKIN) to get ISkin interface.
	 */
	static Modifier*	findSkinModifier( INode* node );

	/** 
	 * Lists all bones used by the Skin modifier. 
	 */
	static void			listBones( ISkin* skin, NS(lang,Array)<INode*>& bones );

	/** 
	 * Inserts all skin vertices affected by the node to the vertex map. 
	 */
	static void			addWeights( ExportWeightMap* vmap, NS(lang,Array)<P(ExportVertex)>& vertexlist,
							INode* node, ISkin* skin, ISkinContextData* skincx );

	/** 
	 * Returns initial bone node transform. 
	 */
	static Matrix3		getInitBoneTM( INode* node, ISkin* skin );

	/** 
	 * Returns initial skin node transform. 
	 */
	static Matrix3		getInitSkinTM( INode* node, ISkin* skin );
};


#endif // _SKINUTIL_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
