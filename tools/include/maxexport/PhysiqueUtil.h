#ifndef _PHYSIQUEUTIL_H
#define _PHYSIQUEUTIL_H


#include "ExportWeightMap.h"
#include <lang/Array.h>
#include <Phyexp.h>


/** 
 * Utilities for 3dsmax Character Studio Physique exporting. 
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
class PhysiqueUtil
{
public:
	/** 
	 * Find if a given node3ds contains a Physique Modifier.
	 */
	static Modifier*	findPhysiqueModifier( INode* node3ds );

	/** 
	 * Lists all bones used by the Physique modifier context. 
	 */
	static void			listBones( IPhyContextExport* phycontextexp, NS(lang,Array)<INode*>& bones );

	/** 
	 * Inserts all skin vertices affected by the node3ds to the vertex map. 
	 */
	static void			addWeights( ExportWeightMap* vmap, NS(lang,Array)<P(ExportVertex)>& vertexlist,
							INode* node3ds, IPhyContextExport* phycontextexp );

	/** 
	 * Returns initial bone node3ds transform. 
	 */
	static Matrix3		getInitBoneTM( INode* node3ds, IPhysiqueExport* phyexport );

	/** 
	 * Returns initial skin node3ds transform. 
	 */
	static Matrix3		getInitSkinTM( INode* node3ds, IPhysiqueExport* phyexport );
};


#endif // _PHYSIQUEUTIL_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
