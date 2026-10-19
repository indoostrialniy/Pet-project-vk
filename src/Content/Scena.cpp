/*
	This header contains realization of in-game actors used in game.
*/

#include "Scena.h"

#include "macros.h"		// prints
#include "auxiliary.h" 	// Stopwatch definition
#include "globals.h"	// g_Key

#include "fhStructs.h"


Scena::Scena( const FhMicroWorldInitializationInfo& info ) : MicroWorld (info.vulkanCore)
{
	printVarComment("\n\tConstructor of Scena: ", info.name)	
		
	MicroWorldName 	= info.name; 
	position 		= info.initPos;
	rotation 		= info.initRot;
			
	//~ FhEntityInitializationInfo SM_Tunnel_01_001;
		//~ SM_Tunnel_01_001.initSysname = "SM_Tunnel_01.001";
		//~ SM_Tunnel_01_001.initPos = glm::vec3(-9.05, 4.45, 3.75);
		//~ SM_Tunnel_01_001.initRot = glm::quat(1.0, 0.0, 0.0, 0.0);
		//~ SM_Tunnel_01_001.initMeshName = "SM_Tunnel_01.model";	//dont forget initCollisionType
		//~ //SM_Tunnel_01_001.initCollisionType = emptyCollision;
		//~ //SM_Tunnel_01_001.initMeshMaterialsVector = { "M_LabFloor_01", };
		//~ SM_Tunnel_01_001.initMeshMaterialsVector = {"Material",};
	//~ addEntity( SM_Tunnel_01_001 );	
	
}
Scena::~Scena() {	printVarComment("\n\tDestructor of Scena ", MicroWorldName)	}

