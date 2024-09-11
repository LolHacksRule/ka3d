#include "StdAfx.h"
#include "MyLight.h"
#include <lang/Math.h>
#include <lang/Debug.h>
#include <lang/Exception.h>
#include <config.h>


USING_NAMESPACE(lang)
USING_NAMESPACE(math)


MyLight::MyLight( INode* node3ds ) :
	MyNode( node3ds, new hgr::Light )
{
	// get realtime parameters

	ObjectState objstate = node3ds->EvalWorldState( 0 );
	assert( 0 != objstate.obj );
	assert( LIGHT_CLASS_ID == objstate.obj->SuperClassID() );

	GenLight* lob = static_cast<GenLight*>( objstate.obj );
	Interval valid( 0, 0 );
	int lightevalresult = lob->EvalLightState( 0, valid, &lightstate );
	assert( REF_SUCCEED == lightevalresult );

	switch ( lightstate.type )
	{
	case OMNI_LGT:		node()->setType( hgr::Light::TYPE_OMNI ); break;
	case DIRECT_LGT:	node()->setType( hgr::Light::TYPE_DIRECTIONAL ); break;
	case SPOT_LGT:		node()->setType( hgr::Light::TYPE_SPOT ); break;
	default:			throwError( Exception( Format("Unsupported 3dsmax light type ({0})", lob->Type()) ) );
	}

	node()->setColor( float3(lightstate.color.r, lightstate.color.g, lightstate.color.b) * lightstate.intens );
	Debug::printf( "Light color is %g %g %g\n", node()->color().x, node()->color().y, node()->color().z );
	if ( SPOT_LGT == lightstate.type )
	{
		node()->setInnerCone( Math::toRadians(lightstate.hotsize) );
		node()->setOuterCone( Math::toRadians(lightstate.fallsize) );
	}

	if ( 0 != lightstate.useAtten )
	{
		node()->setFarAttenStart( lightstate.attenStart );
		node()->setFarAttenEnd( lightstate.attenEnd );
	}

	// get other parameters
	m_decayRadius = lob->GetDecayRadius( 0 );
	m_decayType = lob->GetDecayType();
	m_diffSoft = lob->GetDiffuseSoft( 0 );
	m_lightWorldTm = node()->worldTransform();
	m_lightColor = node()->color();
	m_worldToLight = m_lightWorldTm.inverse();
	m_hotCos = Math::cos( Math::toRadians(lightstate.hotsize)*.5f );
	m_fallCos = Math::cos( Math::toRadians(lightstate.fallsize)*.5f );
	m_overshoot = lob->GetOvershoot();
	m_spotShape = lob->GetSpotShape();

	m_contrast = lob->GetContrast( 0 );
	float a = m_contrast/204.0f + 0.5f;
	m_kA = 2.0f - 1.0f/a;
	m_kB = 1.0f - m_kA;
}

MyLight::~MyLight()
{
}

void MyLight::illuminate( const float3& point, 
	const float3& normal,
	float3* diffuse )
{
	*diffuse = float3(0,0,0);
	if ( !lightstate.on )
		return;

	// geometric (N,L) atten
	float3 L = m_lightWorldTm.translation() - point;
	float dist = L.length();
	if ( dist < 1e-9f )
		return;
	L *= 1.f/dist;
	float ndotl = dot( L, normal );
	if ( ndotl <= 0.f )
		return;
	float atten = contrastFunc( ndotl );

	// distance atten
	atten *= applyDistanceAtten( dist );

	// shape atten
	if ( node()->type() == hgr::Light::TYPE_SPOT )
	{
		float angle = -dot( normalize0(m_lightWorldTm.getColumn(2)), L );
		if ( angle < m_fallCos )
			return;
		if( !m_overshoot && angle < m_hotCos )
		{
			float u = (angle-m_fallCos)/(m_hotCos-m_fallCos);
			float shapeatten = u*u*(3.0f-2.0f*u);
			atten *= shapeatten;
		}
	}

	*diffuse = m_lightColor * atten;
}

inline float MyLight::contrastFunc( float ndotl ) const
{
	if ( m_diffSoft != 0.0f ) 
	{
		float p = ndotl*ndotl*(2.0f-ndotl);
		ndotl = m_diffSoft*p + (1.0f-m_diffSoft)*ndotl;
	}
	return (m_contrast==0.0f)? ndotl : ndotl/(m_kA*ndotl+m_kB);
}

inline float MyLight::decay( float att, float dist, float r0 ) const
{
	if ( m_decayType == DECAY_NONE || dist == 0.0f ) 
		return att;
	float s = r0/dist;
	if ( s >= 1.0f ) 
		return att;
	return (m_decayType==DECAY_INVSQ)?  s*s*att: s*att;
}

inline float MyLight::applyDistanceAtten( float dist ) const
{
	float atten = 1.0f;
	if( lightstate.useNearAtten )
	{
	   	if ( dist < lightstate.nearAttenStart )
	   		return 0.f;
		if ( dist < lightstate.nearAttenEnd )	
		{
			float u = (dist - lightstate.nearAttenStart)/(lightstate.nearAttenEnd-lightstate.nearAttenStart);
			atten = u*u*(3-2*u);
		}
	}
	if ( lightstate.useAtten ) 
	{
	   	if ( dist > lightstate.attenEnd )
	   		return 0.f;
		if ( dist > lightstate.attenStart )	
		{
			float u = (lightstate.attenEnd-dist)/(lightstate.attenEnd-lightstate.attenStart);
			atten *= u*u*(3-2*u);
		}
	}

	atten = decay( atten, dist, m_decayRadius );
	return atten;
}

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
