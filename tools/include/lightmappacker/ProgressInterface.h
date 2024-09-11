#ifndef _LIGHTMAPPACKER_PROGRESSINTERFACE_H
#define _LIGHTMAPPACKER_PROGRESSINTERFACE_H


namespace lang {
	class String;}


namespace lightmappacker
{


/**
 * Interface to progress bar during long lightmap builds.
 */
class ProgressInterface
{
public:
	/** Sets static text describing current task. */
	virtual void	setText( const NS(lang,String)& text ) = 0;

	/** Sets progress bar relative [0,1] position. */
	virtual void	setProgress( float pos ) = 0;
};


} // lightmappacker


#endif // _LIGHTMAPPACKER_PROGRESSINTERFACE_H
