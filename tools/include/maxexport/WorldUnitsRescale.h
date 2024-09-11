#ifndef _WORLDUNITSRESCALE_H
#define _WORLDUNITSRESCALE_H


class WorldUnitsRescale
{
public:
	WorldUnitsRescale( Interface* ip, int unitsystem, bool enabled );
	~WorldUnitsRescale();

private:
	Interface*	m_ip;
	int			m_unitsystem;
	float		m_scale;
	bool		m_enabled;

	WorldUnitsRescale( const WorldUnitsRescale& );
	WorldUnitsRescale& operator=( const WorldUnitsRescale& );
};


#endif // _WORLDUNITSRESCALE_H

// Copyright (C) 2004 Jani Kajala. All rights reserved. Consult your license regarding permissions and restrictions.
