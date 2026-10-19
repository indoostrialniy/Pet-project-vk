#pragma once

/*
	This header contains description of "Entity". 
	"Entity" is a minimal "tactical unit" of one object on scene. Every Entity cannot exist by itself and must belong to MicroWorld class instance as its "component".
	During configuring "Entity" grabs all required meshes, materials, textures, skeletons and other from game resources.
	
	"Entity" uses FhEntityInitializationInfo-struct for its initialization.
*/

#include <string>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vkTexture.h"
#include "Material.h"
#include "auxiliary.h"	// Vertex;
#include "fhStructs.h"


class Mesh;
class Graphics;

class Entity
{

public:
	// constructor takes struct of entity parameters and owner-actor position and rotation to initialize references MicroWorldPosition and MicroWorldRotation
	Entity( FhEntityInitializationInfo& entityParams, glm::vec3& actorLoc, glm::quat& actorRot, Graphics& engine ); 
	
	~Entity();

	Graphics& g_GraphicsCore;

	bool status = true;
	
	// self entity parameters
	glm::vec3 	position = glm::vec3(0.0, 0.0f, 0.0f);
	glm::quat 	rotation = glm::quat(1, 0, 0, 0);
	glm::vec3 	scale	 = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::mat4	entityMatrix = glm::mat4(1.0f);
	
	// owner actor parameters - copy of them from MicroWorld class. Initialized during constructor
	glm::vec3& 	MicroWorldPosition;
	glm::quat& 	MicroWorldRotation;
	
	// entity resources
	std::shared_ptr<Mesh>					entityMeshPtr;
	//~ std::shared_ptr<Material>				entityMaterialPtr;
	std::vector<std::shared_ptr<Material>>	entityMaterialsPtr;
		
    //~ std::shared_ptr<VkTexture>	TEXTURE; 
    
    //~ void createEntityUniformBuffers(); // создание в видеопамяти буфера для шейдерных переменных и "прокидывание" прямого доступа к нему
    //~ void destroyEntityUniformBuffers();
    //~ void updateEntityUniformBuffer(uint32_t currentImage);

	void createEntityMaterialInfoBuffer();
	void destroyEntityMaterialInfoBuffer();
	void updateEntityMaterialInfoBuffer(uint32_t currentImage);

	// G-pass section start
	VkDescriptorSet GpassDescriptorSet;	// объект, определяющий, какие именно ресурсы надо привязать к шейдеру. ubo (PVM) + texture. Биндится для отрисовки. Не надо удалять вручную. Удаляются автоматически вместе с пулом

    void createEntityGpassDescriptorSet( std::vector<std::shared_ptr<Material>> material, std::string entityName ); // текстура и сэмплер для нее встраиваются в descriptor set. удаление не нужно, ибо происходит автоматически
	// G-pass section end

    
	//~ // для каждого объекта нужны нижеприведенные хэндлеры по числу кадров в swapchain - MAX_FRAMES_IN_FLIGHT
	//~ std::vector<VkBuffer> 		 uniformBuffers;							// host-side хэндлер буфера для переменных (матриц)		
	//~ std::vector<VkDeviceMemory>  uniformBuffersMemory;					// кусок видеопамяти для буфера переменных
	//~ std::vector<void*> 			 uniformBuffersMapped;					// адрес видеопамяти для прямого доступа из функции updateEntityUniformBuffer() и т.п.
	
	// material infos buffer
	VkBuffer 		 			materialInfoBuffer;							// host-side хэндлер буфера для переменных (матриц)		
	VkDeviceMemory  			materialInfoBufferMemory;					// кусок видеопамяти для буфера переменных
	void*					 	materialInfoBufferMapped;					// адрес видеопамяти для прямого доступа из функции updateEntityUniformBuffer() и т.п.
	
	
	
	
	uint32_t entityType = FH_ENTITY_TYPE_RENDER_OPAQUE;
	
	std::string entityName;
	
	// auxiliary functions
	std::string  getTexturePath(std::string& filename);

	void configureModelMatrix(glm::mat4& matrix);//, const glm::vec3& setWorldLocation, const glm::quat& setWorldRotation);
	
	void setLocalPosition( glm::vec3 pos );
	void setLocalRotation( glm::quat rot );
	

	
};

