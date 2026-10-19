#pragma once
//~ #ifndef LEVEL_H
//~ #define LEVEL_H

//~ #include <iostream>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <map>
#include "macros.h"
#include "globals.h"

#include "fhStructs.h"

class MicroWorld;	//~ #include "MicroWorld.h"

struct FhEntityInitializationInfo;

class Graphics;

class Level ////базовый класс уровня для описания игрового уровня в целом. Здесь будут списки Сетов и типовая логика уровня. И рендер
{
protected:
	Level(Graphics& core);			// поскольку все уровни будут уточняться через дочерние классы, нужен пустой конструктор базового класса

public:	
	virtual ~Level();
	
	std::string levelName = "default";

	std::shared_ptr<MicroWorld> getActor(const std::string actorName);
	
	Graphics& g_GraphicsCore;		
	
	template <typename T>
	bool spawnActor( const FhMicroWorldInitializationInfo& info )
	{
		//printVarComment("spawnActor ", comboName)
		if( actors.count(info.name) == 0)
		{
			auto newActor = std::make_shared<T>( info );
			if( CHECK_PTR(newActor) ) //&& newActor->getStatus() )
			{
				actors[info.name] = newActor;				
				return true;
			}
			else
			{
				printVarComment("Fail to spawn actor. Error configuration ", info.name)
				DEBUG_INFO
				return false;
			}
		}
		else
		{
			printVarComment("Fail to spawn actor. Already exists: ", info.name)
			DEBUG_INFO
			return false;
		}
	}
	
	
	

//~ protected:
	std::map <std::string, std::shared_ptr<MicroWorld> > actors; //словарь актеров уровня

public:
	void addComponentToMicroWorld( const std::string& actorName, FhEntityInitializationInfo& newEntity );


protected:
		
	std::string getLevelPath(std::string Name);
	
	bool loadLevel();
	
};




//~ #endif
