#include "Level.h"

#include "MicroWorld.h"
#include "macros.h"
#include "auxiliary.h"

#include <fstream>  //ifstream
#include "Entity.h"



//~ #define LEVEL_DEBUG



Level::Level(Graphics& core) : g_GraphicsCore(core)
{
	#ifdef LEVEL_DEBUG
			std::cout << "Level constructor" << std::endl;
	#endif
}

Level::~Level()
{
	actors.clear();	 // очистка словаря приведет к удалению умных указателей, а значит, взовет их деструкторы								
}


std::shared_ptr<MicroWorld> Level::getActor(const std::string actorName)
{
	std::shared_ptr<MicroWorld> res;								// создадим новый shared_ptr для хранения ресурса
	
	if( actors.count(actorName) != 0 )									// если найден дубликат
	{
		#ifdef LEVEL_DEBUG
			std::cout << "\n\n\tObject [" << actorName << "] already has an instance, return shared_ptr reference." << std::endl;
		#endif
		
		res = actors[actorName];//.lock();									// отметим, что старого ресурса стало больше
	}
	else
	{
		std::cout << "\n\n\tNo actor [" << actorName << "] found! Return empty smartpointer. "; DEBUG_INFO
	}
	return res;																// и вернем указатель на запрашиваемый ресурс
}



std::string Level::getLevelPath(const std::string Name)	
{	
	//~ return getOperationSystem() == "Windows" ? "datas\\" + Name + "" : "./datas/" + Name + "" ;		//тернарный определитель пути
	return "datas\\" + Name + ".ltx";
}	


void Level::addComponentToMicroWorld( const std::string& actorName, FhEntityInitializationInfo& newEntity )
{	
	if(actors.contains(actorName))
	{
		#ifdef LEVEL_DEBUG
			printVarComment("Adding entity to actor: ", actorName)
		#endif
		
		actors[actorName]->addEntity( newEntity );
	}
	else
	{
		printVarComment("No actors found: ", actorName)
		DEBUG_INFO
	}
}


bool Level::loadLevel()
{
	// XXX actors["backgroundobjects"] = new MicroWorld("backgroundObjects");
	
	#ifdef LEVEL_DEBUG
		std::cout << "\nLoading level " << levelName << "...\n";		// << std::endl;
	#endif

	std::ifstream levelToLoadStream( getLevelPath(levelName) );

	if (levelToLoadStream.is_open())
	{
		std::string StrVar; //строкова¤ переменна¤ дл¤ хранени¤ строки текста из документа
		unsigned short int parametersFound = 0;

		while (getline(levelToLoadStream, StrVar)) //записываем в StrVar строки из файла, пока они есть
		{
			//циклом считали позицию камеры и ее поворот
			if (getValue(StrVar, 0) == "p")
			{
				if(actors.count("Camera") != 0)
				{
					glm::vec3 position2;
					readFVec3( position2, StrVar, 1 )
					actors["Camera"]->setActorWorldPosition( position2 );
				}
				parametersFound++;
			}
			
			if (getValue(StrVar, 0) == "v")
			{		
				if(actors.count("Camera") != 0)
				{
					glm::quat rotation2 = glm::quat(1,0,0,0);
					readQuatWXYZ(rotation2, StrVar, 1)	//считаем в rotation WXYZ кватернион из строки
					/*
						по умолчанию камера блендера смотрит в -Z, верх у нее в Y, а право - в X
						* Поэтому, повернув камеру там в нужную сторону, надо еще учесть, что оси всех остальных объектов до поворотов описываются единичной матрицей
						* Соответственно, надо внести поправку для кватерниона камеры, чтобы по умолчанию ее оси как бы тоже описывались единичной матрицей 
						* Это можно сделать домножением кватерниона камеры на glm::quat(0.5, -0.5, 0.5, 0.5)
					*/
					actors["Camera"]->setActorWorldRotation(  rotation2 * glm::quat(0.5, -0.5, 0.5, 0.5)  );
				}
				parametersFound++;
			}
			if (getValue(StrVar, 1) == "MESH") //как только наткнулись на блок описания объекта.
			{
				FhEntityInitializationInfo rookie;					// создадим инициализационную структуру
				
				//~ std::string initSysname; glm::vec3 initPos; glm::quat initRot;
				//~ std::string initMeshName; std::string initType;  std::string shaderName = "forward"; std::string lightmapName = "none";
				//~ std::vector <std::string> initMeshMaterialsVector;
		
				//~ printMessage("FLAG 3.1")
			
			
				while (getValue(StrVar, 0) != "END") //пока не наткнемся на сигнал конца описания сущности, то читаем блок описания сущности
				{
					std::string Slovo = "";
					Slovo = getValue(StrVar, 0); //выдрали первое слово из строки

					//if (StrVar[0] == '[') 	{								rookie.initType 	= getValue(StrVar, 1); }
					if (Slovo == "type") 	{								rookie.initType 	= stof(getValue(StrVar, 1)); }

					if (Slovo == "sysname") { 								rookie.initSysname 	= getValue(StrVar, 1); }
					if (Slovo == "position") { for (int i = 0; i < 3; i++) 	rookie.initPos[i] 	= stof(getValue(StrVar, i + 1)); }
					if (Slovo == "direction")      //выдираем кватернион поворота
					{
																			rookie.initRot.w 	= stof(getValue(StrVar, 1));
																			rookie.initRot.x 	= stof(getValue(StrVar, 2));
																			rookie.initRot.y 	= stof(getValue(StrVar, 3));
																			rookie.initRot.z 	= stof(getValue(StrVar, 4));
					}
					if (Slovo == "scale") { for (int i = 0; i < 3; i++) 	rookie.initScale[i] 	= stof(getValue(StrVar, i + 1)); }
					if (Slovo == "model") { 								rookie.initMeshName = "datas//meshes/"+getValue(StrVar, 1)+".model"; }
					if (Slovo == "material") { 								rookie.initMeshMaterialsVector.push_back(getValue(StrVar, 1)); }
					//if (Slovo == "shader") { 								rookie.shaderName 	= getValue(StrVar, 1); }

					//if (Slovo == "Lightmap") { 								rookie.lightmapName = getValue(StrVar, 3); }
					
					
					
					//~ if (StrVar[0] == '[') 	{								initType 	= getValue(StrVar, 1); }
					//~ if (Slovo == "sysname") { 								initSysname 	= getValue(StrVar, 1); }
					//~ if (Slovo == "position") { for (int i = 0; i < 3; i++) 	initPos[i] 	= stof(getValue(StrVar, i + 1)); }
					//~ if (Slovo == "direction")      //выдираем кватернион поворота
					//~ {
																			//~ initRot.w 	= stof(getValue(StrVar, 1));
																			//~ initRot.x 	= stof(getValue(StrVar, 2));
																			//~ initRot.y 	= stof(getValue(StrVar, 3));
																			//~ initRot.z 	= stof(getValue(StrVar, 4));
					//~ }
					//~ if (Slovo == "model") { 								initMeshName = getValue(StrVar, 1); }
					//~ if (Slovo == "material") { 								initMeshMaterialsVector.push_back(getValue(StrVar, 1)); }
					//~ if (Slovo == "shader") { 								shaderName 	= getValue(StrVar, 1); }

					//~ if (Slovo == "Lightmap") { 								lightmapName = getValue(StrVar, 3); }

					getline(levelToLoadStream, StrVar);
				}

				//addEntity( "Wall_01.023_Wall_01_MESH", glm::vec3(-8.0, 3.75, 0.0), glm::quat(1.0, 0.0, 0.0, 0.0),  "SM_Wall_01", {  }, "MESH", { "concrete_wall_007", },  "none",  "forward" );

				// передадим структуру в виде rvalue в функцию
				FUNC( addComponentToMicroWorld("Scena", rookie)  )
				
				//~ printMessage("FLAG 3.2")
				
				//FUNC( actors["Background"]->addEntity( std::move(rookie) ) )	//
				
				
				
				//actors["Background"]->addEntity(std::move(initSysname), std::move(initPos), std::move(initRot), std::move(initMeshName), {}, std::move(initType), std::move(initMeshMaterialsVector), std::move(lightmapName), std::move(shaderName) );//
				
				//printMessage("Entity loaded")
				//DEBUG_INFO
				
				
				//~ initMeshMaterialsVector.clear();

				//~ Entity* rookie = new Entity(initSysname, initPos, initRot, initMeshName, initType, initMeshMaterialsVector, lightmapName, shaderName); //создаем сущность со считанными параметрами

				//~ if (rookie->OK) 
				//~ { 
					//~ backgroundObjects->components[initSysname] = rookie; initMeshMaterialsVector.clear(); 
					//~ }
				//~ else { cout << "Fail to create entity [" << initSysname << "], delete pointer" << endl; delete rookie; initMeshMaterialsVector.clear(); }
			}
			//--тут закончили парсить блок данных объекта
		}


		levelToLoadStream.close();

		if (parametersFound == 2)
		{
			return true;
		}
		else
		{
			std::cout << "Some camera params not found! Position or rotation";	DEBUG_INFO return false;
		}
	}
	else
	{
		std::cout << "Cannot read level description: " << getLevelPath(levelName); DEBUG_INFO
		levelToLoadStream.close();
		return false;
	}
	//~ return false;
}




