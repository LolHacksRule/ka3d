#ifndef CONVERTTONTX_H
#define CONVERTTONTX_H


#include <gr/SurfaceFormat.h>
#include <lang/String.h>


/** 
 * Converts texture file to NTX file. Replaces file extension 
 * @param oldfilename Name of the texture (TGA)
 * @param format NTX target pixel format
 * @param colorkeyalpha Enable color keying if the source texture has alpha pixels with less than half intensity.
 * @param compress Compress texture data.
 * @return Number of bytes less in the new file when compared to original file.
 */
int convertToNTX( const NS(lang,String)& oldfilename, NS(gr,SurfaceFormat) format, 
		bool colorkeyalpha, bool compress );


#endif // CONVERTTONTX_H
