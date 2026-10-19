/*
	This file contains realization of level "Laboratory".
*/

#include "Laboratory.h"

#include "macros.h"

#include "Scena.h" // Scena, 
#include "Camera.h"
#include "Guns.h" // SR3
#include "Helicopter.h"
#include "DynamicLightSource.h" // dynamic light


//~ Laboratory::Laboratory()	
//~ {	
	//~ std::cout << "Empty constructor of Laboratory " << levelName << std::endl;
//~ }

Laboratory::Laboratory(std::string name, Graphics& core) : Level(core)
{
	levelName = name;
	
	std::cout << "Detailed constructor of " << levelName << std::endl;

	//~ #ifdef LABORATORY_DEBUG
		printVarComment("\t Loading level:", levelName)
	//~ #endif
	
		FhMicroWorldInitializationInfo camera(g_GraphicsCore);
			camera.name = "Camera";
	
	spawnActor<Camera>( camera );
	
	
	//~ printMessage("FLAG 1")
	
		FhMicroWorldInitializationInfo scena(g_GraphicsCore);
			scena.name = "Scena";
	spawnActor<Scena>( scena ); // background level objects: tunnel, server etc
	
	//~ printMessage("FLAG 2")
	
		FhMicroWorldInitializationInfo labSpot(g_GraphicsCore);
			labSpot.name = "LabSpot";
			labSpot.initPos = glm::vec3(6.972, 8.972, 2.964);//7, 1, 3);
			//~ labSpot.initRot = glm::quat(0.630, 0.527, -0.36, -0.43);//glm::quat(0.707, 0, -0.707, 0);
			labSpot.initRot = glm::quat(1,0,0,0);
			labSpot.color = glm::vec3(0.5,0.98,0.34);
			//~ labSpot.type = FH_DYNAMIC_LIGHTSOURCE_SPOT;
			labSpot.type = FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;
			labSpot.sector = 90.0f;
			labSpot.radius = 7.0f;
			labSpot.power = 5.0f;
			labSpot.exists = 1;
			labSpot.index = 1;	// индекс текущего источника
			
	spawnActor<DynamicLight>( labSpot ); // background level objects: tunnel, server etc
	
	FhMicroWorldInitializationInfo heliSpot(g_GraphicsCore);
			heliSpot.name = "HeliSpot";
			heliSpot.initPos = glm::vec3(8.063, -14.8, 3.563);//7, 1, 3);
			heliSpot.initRot = glm::quat(1,0,0,0);
			heliSpot.color = glm::vec3(1.0);;//glm::vec3(0.25, 0.65, 1.0);
			heliSpot.type = FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;
			//~ heliSpot.type = FH_DYNAMIC_LIGHTSOURCE_SPOT;
			heliSpot.sector = 90.0f;
			heliSpot.radius = 7.0f;
			heliSpot.power = 3.0f;
			heliSpot.exists = 1;
			heliSpot.index = 2;	// индекс текущего источника
			
	spawnActor<DynamicLight>( heliSpot ); // background level objects: tunnel, server etc

	
	FhMicroWorldInitializationInfo heli(g_GraphicsCore);
			heli.name = "Helicopter";
			heli.initPos = glm::vec3(8.19, -17.1, 1.19);
			heli.initRot = glm::quat(1,0,0,0);
	spawnActor<Helicopter>( heli ); // background level objects: tunnel, server etc

	//~ spawnActor<SR3>("SR3", glm::vec3(-10.048, 5.741, 3.414), glm::quat(1,0,0,0)); //glm::quat(0.732,-0.488,0.260,-0.398) );	
	
	printMessage("ACTORS LOADED, NOW LOADING LEVEL")
	
	loadLevel();
	
	//~ printMessage("FLAG 4")
}


Laboratory::~Laboratory()
{
	std::cout << "Destructor of Laboratory " << levelName << std::endl;
}

