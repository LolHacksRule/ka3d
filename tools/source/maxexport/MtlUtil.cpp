#include "StdAfx.h"
#include "MtlUtil.h"
#include <iparamm2.h>
#include <lang/Math.h>
#include <lang/Exception.h>
#include <config.h>


#define DXMATERIAL_DYNAMIC_UI Class_ID(0xef12512, 0x11351ed1)


USING_NAMESPACE(lang)


bool MtlUtil::hasFaceMtl( Face& face, Mtl* nodemtl )
{
	if ( nodemtl->NumSubMtls() > 0 && !isDynamicDxMaterial(nodemtl) )
	{
		int id = (unsigned)face.getMatID() % (unsigned)nodemtl->NumSubMtls();
		Mtl* submtl = nodemtl->GetSubMtl( id );
		return submtl != 0;
	}
	return true;
}

Mtl* MtlUtil::getFaceMtl( Face& face, Mtl* nodemtl )
{
	if ( nodemtl->NumSubMtls() > 0 && !isDynamicDxMaterial(nodemtl) )
	{
		int id = (unsigned)face.getMatID() % (unsigned)nodemtl->NumSubMtls();
		Mtl* submtl = nodemtl->GetSubMtl( id );
		if ( submtl == 0 )
			throwError( Exception( Format("Multi-Sub Material \"{0}\" has undefined sub-material id {1}", String(nodemtl->GetName()), id+1) ) );
		return submtl;
	}
	return nodemtl;
}

BitmapTex* MtlUtil::getBitmapTex( StdMat* stdmtl, int chn )
{
	assert( stdmtl != 0 );

	const String mtlname( stdmtl->GetName() );

	StdMat2* stdmtl2 = 0;
	if ( stdmtl->SupportsShaders() )
	{
		stdmtl2 = static_cast<StdMat2*>( stdmtl );
		chn = stdmtl2->StdIDToChannel( chn );
	}

	if ( stdmtl->MapEnabled(chn) )
	{
		Texmap*	texmap = stdmtl->GetSubTexmap( chn );
		if ( 0 != texmap &&  // channel set
			(!stdmtl2 || 2 == stdmtl2->GetMapState(chn)) ) // set & enabled
		{
			if ( texmap->ClassID() != Class_ID(BMTEX_CLASS_ID,0) )
				throwError( Exception( Format("Material \"{0}\" has map channel which is not type 'Bitmap' (unsupported!)", mtlname) ) );
			
			BitmapTex* bmptex = static_cast<BitmapTex*>( texmap );
			if ( !bmptex->GetMapName() )
				return 0;
			String bmpname = bmptex->GetMapName();

			// check that crop mode has not been used
			IParamBlock2* pb = bmptex->GetParamBlock( 0 );
			int applycrop=0, cropmode=0;
			float cropu=0.f, cropv=0.f, cropw=1.f, croph=1.f;
			Interval valid = FOREVER;
			pb->GetValue( pb->IndextoID(0), 0, cropu, valid );
			pb->GetValue( pb->IndextoID(1), 0, cropv, valid );
			pb->GetValue( pb->IndextoID(2), 0, cropw, valid );
			pb->GetValue( pb->IndextoID(3), 0, croph, valid );
			pb->GetValue( pb->IndextoID(6), 0, applycrop, valid );
			pb->GetValue( pb->IndextoID(7), 0, cropmode, valid );
			if ( applycrop && 
				(Math::abs(cropu) > 1e-9f || Math::abs(cropv-1.f) > 1e-9f ||
				Math::abs(cropw) > 1e-9f || Math::abs(croph-1.f) > 1e-9f) )
			{
				throwError( Exception( Format("Material \"{0}\" has Bitmap \"{1}\" which uses Cropping/Placement parameters (unsupported!)", mtlname, bmpname) ) );
			}

			// check that UV offset/tiling has not been used
			StdUVGen* uvgen = bmptex->GetUVGen();
			if ( uvgen )
			{
				if ( Math::abs(uvgen->GetUOffs(0)) > 1e-9f ||
					Math::abs(uvgen->GetVOffs(0)) > 1e-9f ||
					//Math::abs(uvgen->GetUScl(0)-1.f) > 1e-9f ||
					//Math::abs(uvgen->GetVScl(0)-1.f) > 1e-9f ||
					Math::abs(uvgen->GetUAng(0)) > 1e-9f ||
					Math::abs(uvgen->GetVAng(0)) > 1e-9f ||
					Math::abs(uvgen->GetWAng(0)) > 1e-9f )
				{
					throwError( Exception( Format("Material \"{0}\" has Bitmap \"{1}\" which uses Coordinates Offset/Tiling parameters (unsupported!)", mtlname, bmpname) ) );
				}
			}

			return bmptex;
		}
	}
	return 0;
}

bool MtlUtil::isDynamicDxMaterial( Mtl* newmtl )
{
	DllDir* lookup = GetCOREInterface()->GetDllDirectory();
	ClassDirectory& dirLookup = lookup->ClassDir();

	ClassDesc* cd = dirLookup.FindClass(MATERIAL_CLASS_ID,newmtl->ClassID());
	if( cd && cd->SubClassID() == DXMATERIAL_DYNAMIC_UI )
		return true;
	else
		return false;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
