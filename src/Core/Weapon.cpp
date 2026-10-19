#include "Weapon.h"

#include "macros.h"
//~ #include "auxiliary.h" //Stopwatch definition

//~ #include "Entity.h"
//~ #include "fhRender.h"
//~ #include "globals.h"
//~ #include "Mesh.h"
#include <thread>


// список индексов всех возможных сценариев движения частей
// MovingPartsController::enum MOVEMENT_SCENARIOS {WPN_SHOOT, WPN_PRIKLAD}; // 0, 1 ...

Weapon::Weapon(Graphics& core) : MovingPartsController(core)
{
	printVarComment("\n\tEmpty Constructor of Weapon: ", MicroWorldName)
}


void Weapon::wpnShoot()
{	
	printVarComment("wpnShoot ", MicroWorldName)
	MovingPartsController::launchScenario(WPN_SHOOT); // запускаем сценарий по расшифровке ИЗ КЛАССА ОРУЖИЯ!
}
	
void Weapon::wpnChangePriklad()
{
	printVarComment("wpnChangePriklad ", MicroWorldName)
	MovingPartsController::launchScenario(WPN_PRIKLAD);
}


void Weapon::wpnReload()
{
	printVarComment("wpnReload ", MicroWorldName)	
	MovingPartsController::launchScenario(2);
	MovingPartsController::launchScenario(3);
}



