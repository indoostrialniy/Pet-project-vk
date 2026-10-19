#pragma once

#include "MovingPartsController.h"

/*
	general class for weapon
*/

class Graphics;

class Weapon : public MovingPartsController 
{
protected:
	Weapon(Graphics& core);
	
	
		
	void wpnShoot();
	
	void wpnChangePriklad();
	
	void wpnReload();

	enum WEAPON_SCENARIOS { WPN_SHOOT, WPN_PRIKLAD };	// расшифровка сценариев движения оружия

};


