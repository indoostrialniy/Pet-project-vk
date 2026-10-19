#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct FhEntityInitializationInfo;
class Entity;

class Graphics;



class MicroWorld 
{
protected:
	MicroWorld(Graphics& core);
	
	//~ MicroWorld(std::string name ); 
	
	//~ MicroWorld(std::string name, glm::vec3 worldLoc, glm::quat worldRot);
	
	virtual ~MicroWorld();
	
	Graphics& g_GraphicsCore;

	// загрузка меша требует доступа к функциям Vulkan
	//bool addEntity(const std::string& MicroWorld_EntityName, const std::string& MicroWorld_TextureName, const std::string& MicroWorld_ModelName); // Graphics& MicroWorld_App, 

public:
	bool addEntity( FhEntityInitializationInfo& entityParameters );
	bool deleteEntity( const std::string& entityName );




	std::map<std::string, std::shared_ptr<Entity>> components; 		// словарь указателей на компоненты-сущности



//~ protected:

		
	// переменные
	std::string 		MicroWorldName = "MicroWorld";
	
	
// AXIS
public:
	glm::vec3			getActorForwardVector();
	glm::vec3 			getActorLeftVector();
	glm::vec3			getActorUpVector();
protected:
	glm::mat3 			m3VectorsForwardLeftUp 		= glm::mat3(1.0f);	// единичная матрица векторов вперед, влево и вверх, характеризующая оси комбо



public:
	bool 				visible = true;		// доступен ли сет для рендера. У физических сущностей всегда true, а вот у источников света, которые тоже сеты - варьируется
	
	bool 				status = false;	
	
	
	
// TAGS
public:
	bool 				addTag					(const std::string& tag);
	bool 				removeTag				(const std::string& tag);
protected:
	std::vector <std::string> tags; 					// вектор тегов. Добавление/удаление тегов извне может использоваться для скриптования
	

// TRANSFORMS
public:
	glm::vec3 			getActorPosition();
	glm::quat 			getActorRotation();
	void				setActorWorldPosition	(glm::vec3 newPosition);
	void				setActorWorldRotation	(glm::quat newRotation);
	void 				addActorWorldOffset		(glm::vec3 offset);
	void 				addActorWorldRotation	(glm::quat rotator);
protected:
	glm::vec3 			position = glm::vec3(0, 0, 0);			// координаты сета в мире, к ним часто придется обращаться через метод getWorldLocation()
	glm::quat 			rotation = glm::quat(1, 0, 0, 0); 		// на основании этого кватерниона можно получить матрицу ориентации, к нему часто придется обращаться через метод getWorldRotation()
		

};





	////glm::mat3 			m3DefaultVectorsForwardLeftUp 	= glm::mat3(1.0f);	// единичная матрица векторов по умолчанию
		
	//void 				configureModelMatrix(glm::mat4& matrix, const glm::vec3& setWorldLocation, const glm::quat& setWorldRotation);
