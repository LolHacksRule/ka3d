#ifndef _ARRAYUTIL_H
#define _ARRAYUTIL_H


#include <lang/Array.h>


class ArrayUtil
{
public:
	template <class T> static void	removeDuplicates( NS(lang,Array)<T>& a );
};


template <class T> void ArrayUtil::removeDuplicates( NS(lang,Array)<T>& a )
{
	std::sort( a.begin(), a.end() );
	a.resize( std::unique( a.begin(), a.end() ) - a.begin() );
}


#endif // _ARRAYUTIL_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
