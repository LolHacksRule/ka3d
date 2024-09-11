#include "StdAfx.h"
#include "WorldUnitsRescale.h"
#include <config.h>


WorldUnitsRescale::WorldUnitsRescale( Interface* ip, int unitsystem, bool enabled ) :
	m_ip( ip ),
	m_unitsystem( unitsystem ),
	m_scale( 0.f ),
	m_enabled( enabled )
{
	if ( enabled ) 
	{
		m_scale = (float)GetMasterScale( m_unitsystem );
		ip->RescaleWorldUnits( m_scale, false );
	}
}

WorldUnitsRescale::~WorldUnitsRescale()
{
	if ( m_enabled ) 
	{
		m_ip->RescaleWorldUnits( 1.f / m_scale, false );
	}
}
