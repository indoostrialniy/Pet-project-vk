/*
	This header contains realization of in-game actors used in game.
*/
#include "Helicopter.h"

#include "fhStructs.h"	// FhMicroWorldInitializationInfo, FhEntityInitializationInfo
#include "macros.h"		// prints



Helicopter::Helicopter( const FhMicroWorldInitializationInfo& info ) : Vehicle(info.vulkanCore)
{
	printVarComment("\n\tConstructor of Helicopter: ", info.name)	
	
	MicroWorldName 	= info.name; 
	position 		= info.initPos;
	rotation 		= info.initRot;
	
	
	FhEntityInitializationInfo Seahawk_001;
		Seahawk_001.initSysname = "Seahawk.001";
		Seahawk_001.initPos = glm::vec3(0.0, 0.0, 0.0);
		Seahawk_001.initRot = glm::quat(1.0, 0.0, 0.0, 0.0);
		Seahawk_001.initScale = glm::vec3(1.0, 1.0, 1.0);
		Seahawk_001.initMeshName = FH_MODEL_PATH "SM_Seahawk.model";	//dont forget initCollisionType
		//Seahawk_001.initCollisionType = emptyCollision;
		Seahawk_001.initMeshMaterialsVector = { "Tex_0044_0_dds",  "Tex_0045_0_dds",  "Material__44", }; 
	addEntity( Seahawk_001 );

	FhEntityInitializationInfo Seahawk_Door_001;
		Seahawk_Door_001.initSysname = "Seahawk_Door.001";
		Seahawk_Door_001.initPos = glm::vec3(-0.5538, -1.3453, 1.6356);
		Seahawk_Door_001.initRot = glm::quat(1.0, 0.0, 0.0, 0.0);
		Seahawk_Door_001.initScale = glm::vec3(1.0, 1.0, 1.0);
		Seahawk_Door_001.initMeshName = FH_MODEL_PATH "SM_Seahawk_Door.model";	//dont forget initCollisionType
		//Seahawk_Door_001.initCollisionType = emptyCollision;
		Seahawk_Door_001.initMeshMaterialsVector = { "Tex_0044_0_dds",  "Tex_0045_0_dds",  "Material__44", }; 
	addEntity( Seahawk_Door_001 );

	FhEntityInitializationInfo Seahawk_MainRotor_001;
		Seahawk_MainRotor_001.initSysname = "Seahawk_MainRotor.001";
		Seahawk_MainRotor_001.initPos = glm::vec3(-0.0078, 0.0192, 3.567);
		Seahawk_MainRotor_001.initRot = glm::quat(1.0, 0.0, 0.0, 0.0);
		Seahawk_MainRotor_001.initScale = glm::vec3(1.0, 1.0, 1.0);
		Seahawk_MainRotor_001.initMeshName = FH_MODEL_PATH "SM_Seahawk_MainRotor.model";	//dont forget initCollisionType
		//Seahawk_MainRotor_001.initCollisionType = emptyCollision;
		Seahawk_MainRotor_001.initMeshMaterialsVector = { "Tex_0044_0_dds",  "Tex_0045_0_dds",  "Material__44", }; 
	addEntity( Seahawk_MainRotor_001 );

	FhEntityInitializationInfo Seahawk_RearRotor_001;
		Seahawk_RearRotor_001.initSysname = "Seahawk_RearRotor.001";
		Seahawk_RearRotor_001.initPos = glm::vec3(-11.5118, 0.0666, 4.4599);
		Seahawk_RearRotor_001.initRot = glm::quat(1.0, 0.0, 0.0, 0.0);
		Seahawk_RearRotor_001.initScale = glm::vec3(1.0, 1.0, 1.0);
		Seahawk_RearRotor_001.initMeshName = FH_MODEL_PATH "SM_Seahawk_RearRotor.model";	//dont forget initCollisionType
		//Seahawk_RearRotor_001.initCollisionType = emptyCollision;
		Seahawk_RearRotor_001.initMeshMaterialsVector = { "Tex_0044_0_dds",  "Tex_0045_0_dds",  "Material__44", }; 
	addEntity( Seahawk_RearRotor_001 );


		
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


Helicopter::~Helicopter() {	printVarComment("\n\tDestructor of Helicopter ", MicroWorldName)	}

