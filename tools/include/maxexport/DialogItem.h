#ifndef _DIALOGPARAM_H
#define _DIALOGPARAM_H


#include <lang/Debug.h>
#include <lang/Format.h>
#include <lang/Hashtable.h>
#include <lang/Exception.h>
#include <typeinfo>


class DialogItemBase;

class DialogValidatorBase :
	public lang::Object
{
public:
	virtual ~DialogValidatorBase() {}

	virtual bool			isValidImpl( const DialogItemBase* item ) const = 0;
	virtual lang::Format	getErrorImpl( const DialogItemBase* item ) const = 0;
};

class DialogItemBase :
	public lang::Object
{
public:
	NS(lang,String)			name;
	P(DialogValidatorBase)	validator;

	DialogItemBase() {}
	DialogItemBase( const NS(lang,String)& paramname, DialogValidatorBase* validatorfunc )		: name(paramname), validator(validatorfunc) {}
	virtual ~DialogItemBase() {}

	bool			valid() const	{return validator == 0 || validator->isValidImpl(this);}
	lang::Format	error() const	{return validator == 0 ? lang::Format("(no validator)") : validator->getErrorImpl(this);}
};

template <class T> class DialogItem :
	public DialogItemBase
{
public:
	T value;

	DialogItem() {}
	DialogItem( const NS(lang,String)& paramname, const T& defaultvalue, DialogValidatorBase* validatorfunc=0 )	: DialogItemBase(paramname,validatorfunc), value(defaultvalue) {}
	virtual ~DialogItem() {}
};

template <class T> class DialogValidatorBaseT :
	public DialogValidatorBase
{
public:
	virtual ~DialogValidatorBaseT() {}

	virtual bool			isValid( const DialogItem<T>& item ) const = 0;
	virtual lang::Format	getError( const DialogItem<T>& item ) const = 0;

private:
	bool isValidImpl( const DialogItemBase* item ) const
	{
		const DialogItem<T>* itemt = dynamic_cast<const DialogItem<T>*>( item );
		if ( !itemt )
			throw lang::Exception( Format("Invalid dialog item type validator:\nItem type is {0} and validator type is {1}.\nDialog item name was \"{2}\".", typeid(*item).name(), typeid(*this).name(), item->name) );
		return isValid( *itemt );
	}

	lang::Format getErrorImpl( const DialogItemBase* item ) const
	{
		return getError( *dynamic_cast<const DialogItem<T>*>(item) );
	}
};

template <class T> class DialogNumberValidator :
	public DialogValidatorBaseT<T>
{
public:
	DialogNumberValidator() {}
	DialogNumberValidator( const T& v0, const T& v1 ) : m_v0(v0), m_v1(v1) {}

	bool			isValid( const DialogItem<T>& v ) const				{return v.value >= m_v0 && v.value <= m_v1;}
	lang::Format	getError( const DialogItem<T>& v ) const			{return lang::Format("Value of \"{0}\" must be between {1} and {2} (was: {3})", v.name, m_v0, m_v1, v.value );}

private:
	T m_v0;
	T m_v1;
};


template <class T> bool validateDialogItems( lang::Hashtable< int, DialogItem<T> >& tab, lang::Format* err )
{
	for ( lang::HashtableIterator< int, DialogItem<T> > it = tab.begin() ; it != tab.end() ; ++it )
	{
		if ( !it.value().valid() )
		{
			*err = it.value().error();
			return false;
		}
	}
	return true;
}

template <class T> void printDialogItems( const char* paramvaluetype, lang::Hashtable< int, DialogItem<T> >& tab )
{
	for ( lang::HashtableIterator< int, DialogItem<T> > it = tab.begin() ; it != tab.end() ; ++it )
		lang::Debug::printf( "  %s \"%s\" = %s\n", paramvaluetype, it.value().name.c_str(), Format("{0}",it.value().value).format().c_str() );
}


#endif // _DIALOGPARAM_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
