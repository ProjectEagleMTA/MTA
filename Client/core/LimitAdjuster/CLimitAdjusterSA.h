/*****************************************************************************
 *
 *  PROJECT:     Black Sun Games
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        Client/core/LimitAdjuster/CLimitAdjusterSA.h
 *  PURPOSE:     Limit Adjuster SA
 *
 *****************************************************************************/

#pragma once

class CLimitAdjusterSA
{
public:
	void Update(unsigned short usVehicle, unsigned short usPed, unsigned char ucIMGLimit);
	void IdeLimits(unsigned short usVehicle, unsigned short usPed);
	void UpdateLimitIde();
};

static CLimitAdjusterSA* m_pLimitAdjusterSA;
