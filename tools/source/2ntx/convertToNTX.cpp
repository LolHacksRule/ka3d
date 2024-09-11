#include "convertToNTX.h"
#include <io/all.h>
#include <gr/SurfaceFormat.h>
#include <img/all.h>
#include <lang/all.h>
#include <math/all.h>
#include <string.h>
#include <stdio.h>


#define VERSION "1.0"
//#define DEBUG_OUTPUT
//#define DEBUG_MAKE_GRADIENT_TGA
//#define HUFFMAN_TEST


using namespace io;
using namespace gr;
using namespace img;
using namespace lang;
using namespace math;


static bool hasAlpha( const Image& image )
{
	const uint32_t* bits = image.bits();
	int pixels = image.width() * image.height();
	for ( int i = 0 ; i < pixels ; ++i )
	{
		if ( (bits[i]&0xFF000000) <= 0x80000000U )
			return true;
	}
	return false;
}

int convertToNTX( const String& oldfilename, SurfaceFormat format, bool colorkeyalpha, bool compress )
{
	PathName oldname( oldfilename );

	String parentpath = String(oldname.parent().toString());
	if ( parentpath != "" )
		parentpath = parentpath + "/";
	String newname = parentpath + String(oldname.basename()) + String(".ntx");
	
	Image img( oldname.toString() );

	int w = img.width();
	int h = img.height();
	int bytesperpixel = format.bitsPerPixel()/8;
	int pixels = w*h;
	Array<uint8_t> bits( pixels*bytesperpixel );
	uint8_t* dst = &bits[0];
	int dstpitch = w*bytesperpixel;
	SurfaceFormat dstpalfmt;
	const void* dstpal = 0;
	const SurfaceFormat srcfmt = img.format();
	const void* src = img.bits();
	int srcpitch = img.pitch();
	SurfaceFormat srcpalfmt;
	const void* srcpal = 0;

	format.copyPixels( dst, dstpitch, dstpalfmt, dstpal, srcfmt, src, srcpitch, srcpalfmt, srcpal, w, h );

	int flags = 0;
	if ( colorkeyalpha && hasAlpha(img) )
	{
		// find out non-black shade
		uint32_t nonblackshade = 0;
		for ( int i = 1 ; i < 256 ; ++i )
		{
			uint32_t color = i + (i<<8) + (i<<16);
			uint8_t bytes[4];
			format.copyPixels( bytes, dstpalfmt, dstpal, SurfaceFormat::SURFACE_A8R8G8B8, &color, SurfaceFormat(), 0, 1 );
			nonblackshade = ImageWriter::getBytesLE( bytes, bytesperpixel );
			if ( 0 != nonblackshade )
				break;
		}
		assert( nonblackshade != 0 );

		// replace black pixels with darkest non-black shade
		for ( int i = 0 ; i < pixels ; ++i )
		{
			uint32_t c = ImageWriter::getBytesLE( dst+i*bytesperpixel, bytesperpixel );
			if ( 0 == c )
				ImageWriter::setBytesLE( dst+i*bytesperpixel, nonblackshade, bytesperpixel );
		}

		// replace transparent pixels with black
		for ( int i = 0 ; i < pixels ; ++i )
		{
			uint32_t c = img.bits()[i];
			if ( (c&0xFF000000) <= 0x80000000U )
				ImageWriter::setBytesLE( dst+i*bytesperpixel, 0, bytesperpixel );
		}

		// enable color key in ntx file
		flags |= ImageWriter::NTX_COLORKEY;
	}

	// huffman-compression enabled?
	if ( compress )
		flags |= ImageWriter::NTX_COMPRESS;

	// retrieve original file size
	int oldfilesize = 0;
	{
		FileInputStream fin( oldfilename );
		oldfilesize = fin.available();
	}

	int userflags = 0;
	int ntxfilesize = ImageWriter::writeNTX( newname, dst, w, h, dstpitch, format, flags, userflags );
	printf( "%s (%d bytes)\n->%s (%d bytes)\n", oldname.toString(), oldfilesize, newname.c_str(), ntxfilesize );

#ifdef HUFFMAN_TEST
	// TEST: huffman-coding
	static bool once = true;
	if ( once )
	{
		once = false;

		wchar_t sz[] = L"this is an example of a huffman tree";
		uint16_t* data = (uint16_t*)sz;
		int count = sizeof(sz)/sizeof(sz[0]) - 1;

		Array<uint8_t> out;
		{Huffman16 hf;
		hf.compress( data, count, &out );}

		Array<uint16_t> in;
		{Huffman16 hf;
		hf.decompress( &out[0], out.size(), &in );}
		in.add( 0 );

		assert( !memcmp(sz,&in[0],sizeof(sz)) );
	}

	// huffman-encoding
	assert( bytesperpixel == 2 );
	Array<uint8_t> out;
	{Huffman16 hf;
	hf.compress( (uint16_t*)dst, pixels, &out );}
	{FileOutputStream fo( newname+".huf" );	fo.write( out.begin(), out.size() );}

	// huffman-decoding test
	{
		Array<uint8_t> in;
		{FileInputStream fi( newname+".huf" );
		in.resize( fi.available() );
		fi.read( in.begin(), in.size() );}
		
		Array<uint16_t> dst2;
		{Huffman16 hf;
		hf.decompress( in.begin(), in.size(), &dst2 );}
		assert( dst2.size() == pixels );
		assert( !memcmp(dst2.begin(),dst,pixels*2) );
	}

	/*static int totalorig = 0;
	static int totalntx = 0;
	totalorig += oldfilesize - out.size();
	totalntx += ntxfilesize - out.size();
	printf( "Huffman-encoded size: %d (saved %d vs ntx, %d vs original)\n", out.size(), totalntx, totalorig );*/
#endif

#ifdef DEBUG_OUTPUT
	Image temp( newname );
	temp.save( newname+".png" );
#endif

	return oldfilesize - ntxfilesize;
}
