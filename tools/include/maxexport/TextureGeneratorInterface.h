#ifndef _TEXTUREGENERATORINTERFACE_H
#define _TEXTUREGENERATORINTERFACE_H


#include <lang/String.h>


class TextureGeneratorInterface
{
public:
	virtual bool			isGeneratedTexture( const NS(lang,String)& filename ) const = 0;

	virtual NS(lang,String)	generateWhiteTexture() = 0;
	virtual NS(lang,String)	generateShadingTexture( float exponent ) = 0;
};


#endif // _TEXTUREGENERATORINTERFACE_H
