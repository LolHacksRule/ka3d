#ifndef _MYLIGHT_H
#define _MYLIGHT_H


#include "MyNode.h"
#include <hgr/Light.h>


class MyLight :
	public MyNode
{
public:
	LightState	lightstate;

	explicit MyLight( INode* node3ds );
	~MyLight();

	hgr::Light*		node()	{return static_cast<hgr::Light*>(MyNode::node());}

	void	illuminate( const NS(math,float3)& point, 
				const NS(math,float3)& normal,
				NS(math,float3)* diffuse );

	const NS(math,float3x4)&	cachedWorldTransform() const	{return m_lightWorldTm;}

private:
	float			m_decayRadius;
	int				m_decayType;
	float			m_diffSoft;
	NS(math,float3x4)	m_lightWorldTm;
	NS(math,float3)	m_lightColor;
	NS(math,float3x4)	m_worldToLight;
	float			m_hotCos;
	float			m_fallCos;
	int				m_overshoot;
	int				m_spotShape;
	float			m_contrast;
	float			m_kA;
	float			m_kB;

	float	contrastFunc( float ndotl ) const;
	float	decay( float att, float dist, float r0 ) const;
	float	applyDistanceAtten( float dist ) const;

	MyLight( const MyLight& );
	MyLight& operator=( const MyLight& );
};


#endif // _MYLIGHT_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
