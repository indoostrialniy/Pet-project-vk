/*
	This header contains realization of in-game actors used in game.
*/

#include "Guns.h"

#include "macros.h"		// prints

#include "fhStructs.h"


SR3::SR3(const FhMicroWorldInitializationInfo& info) : Weapon(info.vulkanCore)
{
	printVarComment("\n\tConstructor of SR3: ", info.name)	
	
	MicroWorldName 	= info.name; 
	position 		= info.initPos;
	rotation 		= info.initRot;
	
	FhEntityInitializationInfo Stvol_001;
		Stvol_001.initSysname = "Stvol.001";
		Stvol_001.initPos = glm::vec3(0.0, 0.0, 0.0);
		Stvol_001.initRot = glm::quat(1.0, 0.0, 0.0, 0.0);
		Stvol_001.initMeshName = "SM_SR3_Stvol.model";	//dont forget initCollisionType
		//Stvol_001.initCollisionType = emptyCollision;
		Stvol_001.initMeshMaterialsVector = { "M_SR3_01", }; 
	addEntity( Stvol_001 );

	
	// опись сценариев
	//~ FhMovementScenarioInfo lidMovingPart;										// создадим структуру с конфигурацией двигающегося компонента
		//~ lidMovingPart.name = "Zatvor.001";							// указываем конкретны компонент
		//~ lidMovingPart.transformationType = MMP_MOVABLE_REVERSE;	//ROTATABLE_MOVABLE_REVERSE;//_REVERSE;				// вращается		
		//~ lidMovingPart.targetPose = glm::vec3(-0.05, 0, 0);
		//~ lidMovingPart.fTimeForMoving = 0.1f;			
	//~ addScenario(lidMovingPart, WPN_SHOOT);		// добавили сценарий в контроллер
		
	//~ FhMovementScenarioInfo PrikladMovingPart;										// создадим структуру с конфигурацией двигающегося компонента
		//~ PrikladMovingPart.name = "Priklad.001";							// указываем конкретны компонент
		//~ PrikladMovingPart.transformationType = MMP_ROTATABLE;	//ROTATABLE_MOVABLE_REVERSE;//_REVERSE;				// вращается
		//~ PrikladMovingPart.targetQuat = glm::quat(0, 0, 0, -1);		// 180 around Z
		//~ PrikladMovingPart.fTimeForRotation = 1.0f;	// 2 секунд на разворот	
	//~ addScenario(PrikladMovingPart, WPN_PRIKLAD);	// добавили сценарий в контроллер
}


SR3::~SR3() {	printVarComment("\n\tDestructor of SR3 ", MicroWorldName)	}

