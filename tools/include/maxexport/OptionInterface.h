#ifndef _OPTIONINTERFACE_H
#define _OPTIONINTERFACE_H


class OptionInterface
{
public:
	virtual bool			getOptionBoolean( const NS(lang,String)& name ) = 0;
	virtual float			getOptionNumber( const NS(lang,String)& name ) = 0;
	virtual NS(lang,String)	getOptionString( const NS(lang,String)& name ) = 0;
};


#endif // _OPTIONINTERFACE_H
