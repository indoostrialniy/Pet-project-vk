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
	
	FhMicroWorldInitializationInfo sun(g_GraphicsCore);
			sun.name = "SUN";
			sun.initPos = glm::vec3(5.98, 14.61, 73);//7, 1, 3);
			sun.initRot = glm::quat(0.774, -0.093, 0.615, -0.116);
			sun.color = glm::vec3(1, 0.958, 0.541);//1.0, 0.594, 0.834);
			sun.type = FH_DYNAMIC_LIGHTSOURCE_SUN;
			sun.sector = 90.0f;
			sun.radius = 140.0f;
			sun.power = 10.0f;
			sun.exists = 1;
			sun.index = 1;	// индекс текущего источника
	spawnActor<DynamicLight>( sun ); // back
	
	FhMicroWorldInitializationInfo spot(g_GraphicsCore);
			spot.name = "SPOT";
			spot.initPos = glm::vec3(21, -40, 28);//7, 1, 3);
			spot.initRot = glm::quat(0.707,0,0.707,0);
			spot.color = glm::vec3(1.0, 0.594, 0.834);
			spot.type = FH_DYNAMIC_LIGHTSOURCE_SPOT;
			spot.sector = 30.0f;
			spot.radius = 40.0f;
			spot.power = 5.0f;
			spot.exists = 1;
			spot.index = 2;	// индекс текущего источника
	//~ spawnActor<DynamicLight>( spot ); // back
	
		//~ FhMicroWorldInitializationInfo shahtaSpot(g_GraphicsCore);
			//~ shahtaSpot.name = "LabSpot";
			//~ //shahtaSpot.initPos = glm::vec3(-1.55, -4.41, 1.18);//7, 1, 3);
			//~ //shahtaSpot.initRot = glm::quat(1,0,0,0);
			//~ shahtaSpot.initPos = glm::vec3(19.839, -28.262, 4.5095);//7, 1, 3);
			//~ shahtaSpot.initRot = glm::quat(1,0,0,0);
			//~ shahtaSpot.color = glm::vec3(1.0, 0.594, 0.034);
			//~ //shahtaSpot.type = FH_DYNAMIC_LIGHTSOURCE_SPOT;
			//~ shahtaSpot.type = FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;
			//~ shahtaSpot.sector = 90.0f;
			//~ shahtaSpot.radius = 14.0f;
			//~ shahtaSpot.power = 8.0f;
			//~ shahtaSpot.exists = 1;
			//~ shahtaSpot.index = 1;	// индекс текущего источника
			
	//~ spawnActor<DynamicLight>( shahtaSpot ); // background level objects: tunnel, server etc
	
	//~ FhMicroWorldInitializationInfo heliSpot(g_GraphicsCore);
			//~ heliSpot.name = "HeliSpot";
			//~ heliSpot.initPos = glm::vec3(21.199, 0.45, 2.802);//7, 1, 3);
			//~ heliSpot.initRot = glm::quat(0.743, -0.065, 0.666, 0);
			//~ heliSpot.color = glm::vec3(0.814, 1.0, 0.507);	//glm::vec3(0.25, 0.65, 1.0);
			//~ //heliSpot.type = FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;
			//~ heliSpot.type = FH_DYNAMIC_LIGHTSOURCE_SPOT;
			//~ heliSpot.sector = 90.0f;
			//~ heliSpot.radius = 7.0f;
			//~ heliSpot.power = 5.0f;
			//~ heliSpot.exists = 1;
			//~ heliSpot.index = 2;	// индекс текущего источника
			
	//~ spawnActor<DynamicLight>( heliSpot ); // background level objects: tunnel, server etc

	//~ FhMicroWorldInitializationInfo hallSpot(g_GraphicsCore);
			//~ hallSpot.name = "ElevatorSpot";
			//~ hallSpot.initPos = glm::vec3(1.04, 5.72, -2.81);//7, 1, 3);
			//~ ////~ hallSpot.initRot = glm::quat(0.630, 0.527, -0.36, -0.43);//glm::quat(0.707, 0, -0.707, 0);
			//~ hallSpot.initRot = glm::quat(0.25, 0.65, 1.0, 0);
			//~ hallSpot.color = glm::vec3(0.25,0.98,0.34);
			//~ ////~ hallSpot.type = FH_DYNAMIC_LIGHTSOURCE_SPOT;
			//~ hallSpot.type = FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;
			//~ hallSpot.sector = 90.0f;
			//~ hallSpot.radius = 7.0f;
			//~ hallSpot.power = 5.0f;
			//~ hallSpot.exists = 1;
			//~ hallSpot.index = 3;	// индекс текущего источника
			
	//~ spawnActor<DynamicLight>( hallSpot ); // background level objects: tunnel, server etc

	
	FhMicroWorldInitializationInfo heli(g_GraphicsCore);
			heli.name = "Helicopter";
			heli.initPos = glm::vec3(8.19, -17.1, 1.19);
			heli.initRot = glm::quat(1,0,0,0);
	//~ spawnActor<Helicopter>( heli ); // background level objects: tunnel, server etc

	//~ spawnActor<SR3>("SR3", glm::vec3(-10.048, 5.741, 3.414), glm::quat(1,0,0,0)); //glm::quat(0.732,-0.488,0.260,-0.398) );	
	
	printMessage("ACTORS LOADED, NOW LOADING LEVEL")
	
	loadLevel();
	
	//~ printMessage("FLAG 4")
}


Laboratory::~Laboratory()
{
	std::cout << "Destructor of Laboratory " << levelName << std::endl;
}

