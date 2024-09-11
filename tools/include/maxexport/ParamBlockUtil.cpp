#include "StdAfx.h"
#include "ParamBlockUtil.h"
#include "maxexportconfig.h"
#include <config.h>


const char* ParamBlockUtil::toString( ParamType2 type )
{
	switch ( type )
	{
	case TYPE_FLOAT: return "TYPE_FLOAT";
	case TYPE_INT: return "TYPE_INT";
	case TYPE_RGBA: return "TYPE_RGBA";
	case TYPE_POINT3: return "TYPE_POINT3";
	case TYPE_BOOL: return "TYPE_BOOL";
	case TYPE_ANGLE: return "TYPE_ANGLE";
	case TYPE_PCNT_FRAC: return "TYPE_PCNT_FRAC";
	case TYPE_WORLD: return "TYPE_WORLD";
	case TYPE_STRING: return "TYPE_STRING";
	case TYPE_FILENAME: return "TYPE_FILENAME";
	case TYPE_HSV: return "TYPE_HSV";
	case TYPE_COLOR_CHANNEL: return "TYPE_COLOR_CHANNEL";
	case TYPE_TIMEVALUE: return "TYPE_TIMEVALUE";
	case TYPE_RADIOBTN_INDEX: return "TYPE_RADIOBTN_INDEX";
	case TYPE_MTL: return "TYPE_MTL";
	case TYPE_TEXMAP: return "TYPE_TEXMAP";
	case TYPE_BITMAP: return "TYPE_BITMAP";
	case TYPE_INODE: return "TYPE_INODE";
	case TYPE_REFTARG: return "TYPE_REFTARG";
	case TYPE_INDEX: return "TYPE_INDEX";
	case TYPE_MATRIX3: return "TYPE_MATRIX3";
	case TYPE_PBLOCK2: return "TYPE_PBLOCK2";
#ifdef MAXEXPORT_HASDXMATERIAL
	case TYPE_POINT4: return "TYPE_POINT4";
	case TYPE_FRGBA: return "TYPE_FRGBA";
#endif
	case TYPE_FLOAT_TAB: return "TYPE_FLOAT_TAB";
	case TYPE_INT_TAB: return "TYPE_INT_TAB";
	case TYPE_RGBA_TAB: return "TYPE_RGBA_TAB";
	case TYPE_POINT3_TAB: return "TYPE_POINT3_TAB";
	case TYPE_BOOL_TAB: return "TYPE_BOOL_TAB";
	case TYPE_ANGLE_TAB: return "TYPE_ANGLE_TAB";
	case TYPE_PCNT_FRAC_TAB: return "TYPE_PCNT_FRAC_TAB";
	case TYPE_WORLD_TAB: return "TYPE_WORLD_TAB";
	case TYPE_STRING_TAB: return "TYPE_STRING_TAB";
	case TYPE_FILENAME_TAB: return "TYPE_FILENAME_TAB";
	case TYPE_HSV_TAB: return "TYPE_HSV_TAB";
	case TYPE_COLOR_CHANNEL_TAB: return "TYPE_COLOR_CHANNEL_TAB";
	case TYPE_TIMEVALUE_TAB: return "TYPE_TIMEVALUE_TAB";
	case TYPE_RADIOBTN_INDEX_TAB: return "TYPE_RADIOBTN_INDEX_TAB";
	case TYPE_MTL_TAB: return "TYPE_MTL_TAB";
	case TYPE_TEXMAP_TAB: return "TYPE_TEXMAP_TAB";
	case TYPE_BITMAP_TAB: return "TYPE_BITMAP_TAB";
	case TYPE_INODE_TAB: return "TYPE_INODE_TAB";
	case TYPE_REFTARG_TAB: return "TYPE_REFTARG_TAB";
	default: return "UNKNOWN";
	}
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
