#include "MicroWorld.h"

#include "macros.h"
//~ #include "auxiliary.h" //Stopwatch definition

#include "Entity.h"
#include "fhRender.h"
#include "globals.h"
#include "Mesh.h"

#define MICROWORLD_DEBUG

MicroWorld::MicroWorld(Graphics& core) : g_GraphicsCore(core)
{
	#ifdef MICROWORLD_DEBUG
		printVarComment("\n\tEmpty Constructor of microworld: ", MicroWorldName)
	#endif
}

//~ MicroWorld::MicroWorld(std::string name ) 
//~ {
	//~ MicroWorldName = name; 
	
	//~ #ifdef MICROWORLD_DEBUG
		//~ printVarComment("\n\tDetalled Constructor 1 of microworld: ", name)
	//~ #endif
//~ }

//~ MicroWorld::MicroWorld(std::string name, glm::vec3 worldLoc, glm::quat worldRot)
//~ {
	//~ MicroWorldName = name;
	
	//~ #ifdef MICROWORLD_DEBUG 
		//~ printVarComment("\n\tDetalled Constructor 2 of microworld: ", name)	
	//~ #endif
	
	//~ position = worldLoc;
	//~ rotation = worldRot;
//~ }

MicroWorld::~MicroWorld()
{
	#ifdef MICROWORLD_DEBUG
		printVarComment("\n\tDestructor of microworld: ", MicroWorldName)
	#endif

}
	

bool MicroWorld::addEntity( FhEntityInitializationInfo& entityParameters )
{
	#ifdef MICROWORLD_DEBUG
		printVarComment("\n\taddEntity: ", entityParameters.initSysname)
	#endif
	
	if( !components.contains( entityParameters.initSysname ) )
	{		
		// Entity creates its Textures and Models by itself
		components[entityParameters.initSysname] = std::make_shared<Entity>( entityParameters, position, rotation, g_GraphicsCore );
		
		if( components[entityParameters.initSysname] )
		{								
			//printVarComment("Initialized: ", entityParameters.initSysname)
			return true;
		}
		else
		{
			std::cout << "Failed to create Entity [" << entityParameters.initSysname << "] in MicroWorld [" << MicroWorldName << "]!\n";
			DEBUG_INFO
			return false;
		}
		
	}
	else
	{
		std::cout << "Entity [" << entityParameters.initSysname << "] already exist in MicroWorld [" << MicroWorldName << "]!\n";
		DEBUG_INFO
		return false;
	}
}


bool MicroWorld::deleteEntity( const std::string& entityName )
{
	if( components.contains( entityName ) )
	{
		std::cout << "Cannot Delete Entity [" << entityName << "] from MicroWorld [" << MicroWorldName << "]!\n";
		return true;
	}
	else
	{
		std::cout << "No Entity [" << entityName << "] in MicroWorld [" << MicroWorldName << "]!\n";
		DEBUG_INFO
		return false;
	}
	
	
}

// AXIS
glm::vec3 MicroWorld::getActorForwardVector()
{
	return m3VectorsForwardLeftUp[0];
}
glm::vec3 MicroWorld::getActorLeftVector()
{
	return m3VectorsForwardLeftUp[1];
}
glm::vec3 MicroWorld::getActorUpVector()
{
	return m3VectorsForwardLeftUp[2];
}


// TAGS
bool MicroWorld::addTag( const std::string& tag )
{
	tags.push_back(tag);
	return true;
}

bool MicroWorld::removeTag( const std::string& tag )
{
	return true;
}




// TRANSFORMS
glm::vec3 MicroWorld::getActorPosition()
{
	return position;
}

glm::quat MicroWorld::getActorRotation()
{
	return rotation;
}

void MicroWorld::setActorWorldPosition	(glm::vec3 newPosition)
{
	position = newPosition;
}

void MicroWorld::setActorWorldRotation	(glm::quat newRotation)
{
	rotation = newRotation;
}

void MicroWorld::addActorWorldOffset(glm::vec3 offset)
{
	position += offset;
}

void MicroWorld::addActorWorldRotation(glm::quat rotator)
{
	rotation = rotator * rotation; // XXX ???
}












//~ void MicroWorld::configureModelMatrix(glm::mat4& matrix, const glm::vec3& setWorldLocation, const glm::quat& setWorldRotation)
//~ {
	//~ glm::vec3 entityPosition = glm::vec3(0, 0, 0);
	//~ glm::quat entityRotation = glm::quat(1, 0, 0, 0);

	//~ matrix = glm::translate(matrix, setWorldLocation + setWorldRotation * entityPosition) 	//	матрицу ориентации домножаем
		//~ * mat4_cast(setWorldRotation * entityRotation);  							//	на матрицу переноса и оставляем это в ссылочном аргументе
//~ }







