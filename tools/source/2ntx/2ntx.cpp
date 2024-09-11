/*
changes:
1.1:
- added file size info
*/
#include "convertToNTX.h"
#include <io/all.h>
#include <img/all.h>
#include <lang/all.h>
#include <math/all.h>
#include <string.h>
#include <stdio.h>

#ifndef PLATFORM_NGI

#define VERSION "1.1"
//#define DEBUG_OUTPUT
//#define DEBUG_MAKE_GRADIENT_TGA


using namespace io;
using namespace gr;
using namespace img;
using namespace lang;
using namespace math;


int main( int argc, char* argv[] )
{
	try
	{
		if ( argc < 3 )
		{
			printf( "2ntx %s - Converts textures no native format\n", VERSION );
			printf( "Usage: 2ntx <format> <filename> <options>\n" );
			printf( "<format>: R8G8B8, R5G6B5, A8R8G8B8, etc.\n" );
			printf( "<options>:\n" );
			printf( "  -nosubdir     Don't process subdirectories\n" );
			printf( "  -colorkey     Enable N3D color keying for pixels with alpha channel < 50%\n" );
			return 0;
		}

		const char* formatdesc = argv[1];
		const char* filename = argv[2];
		bool recursesubdirs = true;
		bool colorkeyalpha = false;
		bool compress = false;
		for ( int i = 3 ; i < argc ; ++i )
		{
			const char* arg = argv[i];
			if ( !strcmp(arg,"-nosubdir") )
				recursesubdirs = false;
			else if ( !strcmp(arg,"-colorkey") )
				colorkeyalpha = true;
			else if ( !strcmp(arg,"-compress") )
				compress = true;
			else
				throwError( Exception(Format("Unsupported option: {0}", arg)) );
		}

		int flags = FindFile::FIND_FILESONLY;
		if ( recursesubdirs )
			flags |= FindFile::FIND_RECURSE;

		SurfaceFormat format( formatdesc );

#ifdef DEBUG_MAKE_GRADIENT_TGA
		Image temp( 256, 256 );
		for ( int j = 0 ; j < 256 ; ++j )
			for ( int i = 0 ; i < 256 ; ++i )
				temp.setPixel( i, j, (i+(i<<8)+(i<<16)) + ((j>128?i:255)<<24) );
		temp.save( "gradient.tga" );
#endif

		int bytesless = 0;
		for ( FindFile ff(filename,flags) ; ff.more() ; ff.next() )
		{
			PathName oldname = ff.data().path;
			bytesless += convertToNTX( oldname.toString(), format, colorkeyalpha, compress );
		}

		printf( "Summary: Image files take %d bytes less space than original files\n", bytesless );
	}
	catch ( Throwable& e )
	{
		printf( "Error: %s\n", e.getMessage().format().c_str() );
		return 1;
	}
	return 0;
}

#endif

