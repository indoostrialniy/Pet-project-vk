#include "Entity.h"

#include "macros.h"
#include "auxiliary.h"

#include "Mesh.h"
#include "fhRender.h"
#include <chrono>
#include "Camera.h"
#include "globals.h"


//~ #define ENTITY_DEBUG

Entity::Entity( FhEntityInitializationInfo& entityParams, glm::vec3& actorLoc, glm::quat& actorRot, Graphics& engine ) 
	: MicroWorldPosition(actorLoc), MicroWorldRotation(actorRot), g_GraphicsCore(engine) // initializer list
{ 
	
	#ifdef ENTITY_DEBUG
		printVarComment("\tConstructor of entity!", entityParams.initSysname) 
	#endif
	
	position = entityParams.initPos;
	rotation = entityParams.initRot;
	scale	 = entityParams.initScale;
	
	
	entityType = entityParams.initType;
	
	entityName = entityParams.initSysname;
	
	//~ printVec3(position)
	//~ printQuat(rotation)
	
	//~ printVec3(actorLoc)
	//~ printQuat(actorRot)
	
	//~ TEXTURE = VkTextures.getAsset( getTexturePath(entityParams.textureName) );
	
	// для одного материала
	// if( entityParams.initMeshMaterialsVector.size() != 0 ) entityMaterialPtr = Materials.getAsset( entityParams.initMeshMaterialsVector[0] );
	
	// для вектора материалов
	for(auto mat : entityParams.initMeshMaterialsVector)	// перебор имен материалов объекта
	{
		//~ printMessage("Create material")
		auto material = g_GraphicsCore.Materials.getAsset( mat );			// очередной материал

		//~ printMessage("add to vector")

		entityMaterialsPtr.push_back(material);
		// printVariable(mat)
		// printVariable(material->resourceName)
		// system("pause");
		
		//~ if( CHECK_PTR(material) )// && material->materialTextures.size() != 0) 
		//~ {
			//~ for(auto map : material->materialTextures) // набор готовых указателей на VkTextures
			//~ {
				//~ // printVariable(map->_filename)
				//~ // system("pause");
				//~ createEntityDescriptorSets( map->_textureImageView, map->_textureSampler, entityParams.initSysname );	// когда буфер и текстуры созданы...
				//~ break; // выход после 1 картинки
			//~ }
			
			//~ // createEntityDescriptorSets( material->materialTextures[0]->_textureImageView, material->materialTextures[0]->_textureSampler, entityParams.initSysname );	// когда буфер и текстуры созданы...
		
			//~ entityMaterialsPtr.push_back( material );
			//~ break;
		//~ }
	}
	//~ printMessage("Entity materials configured")
	
	
	entityMeshPtr = g_GraphicsCore.Meshes.getAsset( entityParams.initMeshName );//std::make_shared<Mesh>(entityParams.initMeshName);
	
	//~ createEntityUniformBuffers();	// создание подключаемого буфера UBO с матрицами
	
	createEntityMaterialInfoBuffer();
	
	//~ if( CHECK_PTR(entityMaterialPtr) && entityMaterialPtr->materialTextures.size() != 0 && CHECK_PTR(entityMaterialPtr->materialTextures[0]) ) 
	//~ if( CHECK_PTR(entityMaterialsPtr) && entityMaterialsPtr->materialTextures.size() != 0 && CHECK_PTR(entityMaterialPtr->materialTextures[0]) ) 
	{
		//~ createEntityDescriptorSets( entityMaterialPtr->materialTextures[0]->_textureImageView, entityMaterialPtr->materialTextures[0]->_textureSampler, entityParams.initSysname );	// когда буфер и текстуры созданы...
		//~ createEntityDescriptorSets( entityMaterialPtr, entityParams.initSysname );	// когда буфер и текстуры созданы...
	}
	
	//~ printMessage("ready to create entity descriptor")	

	createEntityGpassDescriptorSet( entityMaterialsPtr, entityParams.initSysname );	// когда буфер и текстуры созданы...
	
	
	updateEntityMaterialInfoBuffer(0);
}


Entity::~Entity() 
{
	
	destroyEntityMaterialInfoBuffer();
	
	//~ printMessage("\nDestructor of entity!\n") 

	//~ TEXTURE.reset();
}


//~ void Entity::createEntityUniformBuffers()
//~ {
	//~ // создание в видеопамяти буфера для шейдерных переменных и "прокидывание" прямого доступа к нему
		
	//~ // 1 - размер буфера загружаемых в шейдер (ВЫРОВНЕННЫХ!) данных
	//~ VkDeviceSize bufferSize = sizeof(UniformBufferObject);			// размер буфера = 3*(4*4)*4 = 192 байта
			
	//~ // 2 - усечение/актуализация размера векторов данных объекта: для каждого кадра свой буфер, кусок памяти и прочая обвязка
	//~ uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	//~ uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	//~ uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
			
	//~ // 3 - для каждого кадра из цепочки показа - аналогично как делали для буфера вершинных данных, только без vkUnmapMemory!
	//~ for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	//~ {		
		//~ //printMessage("bdbd")
		//~ // 3.1 - создали host-side буфер и выделили ему ВИДИМУЮ! видеопамять
		//~ g_GraphicsCore.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
										//~ uniformBuffers[i], 
										//~ uniformBuffersMemory[i] );
	
		//~ // 3.2 - сделали прямую ссылку на эту ВИДИМУЮ память и сохранили ее в вектор uniformBuffersMemory
		//~ vkMapMemory(g_GraphicsCore.logicalDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i] );

		//~ // XXX буфер остается помеченым до конца жизни программы. Эта техника называется “persistent mapping” и работает на всех имплементациях Vulkan

	//~ }
	

//~ }


//~ void Entity::destroyEntityUniformBuffers()
//~ {	
	//~ for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	//~ {
		//~ vkDestroyBuffer(g_GraphicsCore.logicalDevice, uniformBuffers[i], nullptr);
		//~ vkFreeMemory(g_GraphicsCore.logicalDevice,  uniformBuffersMemory[i], nullptr);			
	//~ }
//~ }

//~ void Entity::updateEntityUniformBuffer(uint32_t currentImage)
//~ {
	//~ static auto startTime = std::chrono::high_resolution_clock::now();

	//~ auto currentTime = std::chrono::high_resolution_clock::now();
	//~ float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
	//~ UniformBufferObject ubo{};	
	
	//~ ubo.materialInfo = glm::mat4(1.0f);	//CHECK_PTR(entityMaterialPtr) ? entityMaterialPtr->getMatInfo() : glm::mat4(1.0f);
	
		
	//~ glm::vec3 POV = CHECK_PTR(g_Camera) ? g_Camera->getActorPosition() : glm::vec3(-1.0f, 0.0f, 0.1f);
	//~ glm::vec3 DOV = POV + (CHECK_PTR(g_Camera) ? g_Camera->getActorForwardVector() : glm::vec3(1.0f, 0.0f, 0.0f));//*Camera::rotation;	//glm::vec3(0.0f, 0.0f, 0.0f);
	//~ glm::vec3 UP = glm::vec3(0.0f, 0.0f, 1.0f);
		
	//~ ubo.view = glm::lookAt(POV, DOV, UP);
		
	//~ if(CHECK_PTR(g_Camera)) ubo.proj = glm::perspective(glm::radians(g_Camera->fCameraFOV), (float) g_GraphicsCore.swapChainExtent.width / (float) g_GraphicsCore.swapChainExtent.height, g_Camera->fCameraViewNear, g_Camera->fCameraViewDepth);
	//~ ubo.proj[1][1] *= -1; // GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. 
	

	//~ memcpy( uniformBuffersMapped[currentImage], &ubo, sizeof(ubo)); // memcpy)dest, src, count
//~ }



void Entity::createEntityMaterialInfoBuffer()
{
	// создание в видеопамяти буфера для шейдерных переменных и "прокидывание" прямого доступа к нему
		
	//~ printMessage("create entity material info buffer")	
	
	// 1 - размер буфера загружаемых в шейдер (ВЫРОВНЕННЫХ!) данных
	VkDeviceSize bufferSize = MAX_MATERIALS_PER_ENTITY * sizeof(FhMaterialInfo);			// размер буфера = 4*(4*4)*4 = 256 байта
	
	//printMessage("bdbd")
	// 3.1 - создали host-side буфер и выделили ему ВИДИМУЮ! видеопамять
	g_GraphicsCore.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
									materialInfoBuffer, 
									materialInfoBufferMemory );

	// 3.2 - сделали прямую ссылку на эту ВИДИМУЮ память и сохранили ее в вектор uniformBuffersMemory
	vkMapMemory(g_GraphicsCore.logicalDevice, materialInfoBufferMemory, 0, bufferSize, 0, &materialInfoBufferMapped );
	
	//~ printMessage("createEntityMaterialInfoBuffer OK")	

}

void Entity::destroyEntityMaterialInfoBuffer()
{
	vkDestroyBuffer(g_GraphicsCore.logicalDevice, materialInfoBuffer, nullptr);
	vkFreeMemory(g_GraphicsCore.logicalDevice,  materialInfoBufferMemory, nullptr);		
}

void Entity::updateEntityMaterialInfoBuffer(uint32_t currentImage)
{
	//FhMaterialInfo MaterialInfos[MAX_MATERIALS_PER_ENTITY];
	
	std::array<FhMaterialInfo, MAX_MATERIALS_PER_ENTITY> MaterialInfos;
	
	int i=0;
	for(auto mat : entityMaterialsPtr)
	{
		//printVarComment("Check mat: ", i)
		
		if(i<MAX_MATERIALS_PER_ENTITY) // пока максимум 4 материала
		{
			MaterialInfos[i] = entityMaterialsPtr[i]->getMaterialInfo();
			i++;
		}
	}
	
	//~ memcpy( materialInfoBufferMapped, &MaterialInfos, sizeof(MaterialInfos)); // memcpy)dest, src, count

	memcpy( materialInfoBufferMapped, MaterialInfos.data(), MAX_MATERIALS_PER_ENTITY*sizeof(FhMaterialInfo)); // memcpy)dest, src, count
}






void Entity::createEntityGpassDescriptorSet( std::vector<std::shared_ptr<Material>> materials, std::string entityName )
{
	
	//~ printMessage("Vanguiu, zdes")
	
	VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = g_GraphicsCore.descriptorPool_G;				// указываем пул			
		allocInfo.descriptorSetCount = 1;										// и число сетов
		allocInfo.pSetLayouts = &g_GraphicsCore.descriptorSetLayout_G; 			// указываем схему описания подключения ресурсов к конвейеру
		
	// 2 - вырезали кусок памяти из pool memory для конкретного descriptor set (взяли розетки со склада)
	VkResult result = vkAllocateDescriptorSets(g_GraphicsCore.logicalDevice, &allocInfo, &GpassDescriptorSet );

	if( result != VK_SUCCESS) 
	{
		g_GraphicsCore.checkResult(result);
		DEBUG_INFO
		throw std::runtime_error("Failed to allocate G-pass descriptor set for entity!");
	}
		
		
	//~ printMessage("flag 1")	
		
	// 3 - связываем реальные ресурсы с дескрипторами
	// 3.1 информация о буфере матриц PVM		
	std::array<VkDescriptorBufferInfo, 2> bufferInfos{};
		// binding 0
		bufferInfos[0].buffer = g_GraphicsCore.GpassGeneralInfoBuffer;	// ссылка на конкретный буфер UBO
		bufferInfos[0].offset = 0;
		bufferInfos[0].range = sizeof(FhGeneralInfo);				// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
	
		// binding 5
		bufferInfos[1].buffer = materialInfoBuffer;						// ссылка на конкретный буфер матриц конфигов материалов
		bufferInfos[1].offset = 0;
		bufferInfos[1].range = MAX_MATERIALS_PER_ENTITY*sizeof(FhMaterialInfo);			// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
	
		//~ // binding 6
		//~ bufferInfos[2].buffer = g_GraphicsCore.lightsUniformBuffers;	// ссылка на конкретный буфер lights
		//~ bufferInfos[2].offset = 0;
		//~ bufferInfos[2].range = sizeof(FhIlluminationInfo);				// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
	
	//~ printMessage("flag 2")		

	// 3.2 - информация о текстурах
	//~ VkDescriptorImageInfo imageInfo{};
		//~ imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//~ imageInfo.imageView = imageView;//textureImageView;
		//~ imageInfo.sampler = sampler;//textureSampler;
		
	// к шейдеру прикрепляются максимум 64 картинки: MAX_MATERIALS_PER_ENTITY массива текстур (т.е. материалы), каждый по MAX_SAMPLERS_PER_MATERIAL сэмплера
	std::array<VkDescriptorImageInfo, MAX_MATERIALS_PER_ENTITY*MAX_SAMPLERS_PER_MATERIAL> materials_textures {};			

	for(int i=0; i<MAX_MATERIALS_PER_ENTITY*MAX_SAMPLERS_PER_MATERIAL; i++ ) 
	{
		int tex = i%MAX_SAMPLERS_PER_MATERIAL;	// 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 
		int mat = i/MAX_SAMPLERS_PER_MATERIAL;	// 0 0 0 0 0 0 0 0 0 0 0  0  0  0  0  0  1 1 1 1 1 1 1 1 1 1 1  1  1  1  1  1
		 
		 
		//~ if(tex == 0) { printVarComment("\n\t\t", mat)	}
		//~ else { printVarComment("\t\t\t", mat) }
		
		//~ std::shared_ptr<Material> material = g_GraphicsCore.Materials.getAsset(); // первый валидный материал
		 
		//~ if(!materials.empty() && material != nullptr && !material->vpMaterialTextures.empty() )	
		if(!materials.empty() && !materials[0]->vpMaterialTextures.empty() ) // если есть хоть один материал и в нем есть хоть одна текстура
		{
			// везде будет 0-я картинка 0-го материала
			//printMessage("\t\t\t\tSetted first valid image")
			#ifdef ENTITY_DEBUG
				std::cout << "\t\t\tEntity [" << entityName << "] get first valid material sampler to material [" << mat << "] and texture [" << tex << "]." << std::endl; //printVarComment("Material ", mat)
			#endif
			
			
			materials_textures[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			materials_textures[i].imageView = materials[0]->vpMaterialTextures[0]->_textureImageView;//materials[0]->vpMaterialTextures[0]->_textureImageView;
			materials_textures[i].sampler = materials[0]->vpMaterialTextures[0]->_textureSampler;// materials[0]->vpMaterialTextures[0]->_textureSampler;
		}
		else
		{
			#ifdef ENTITY_DEBUG
				printVarComment("Error: no valid image texture for entity: ", entityName)
				DEBUG_INFO
			#endif
		}
		
		//~ if(  	 materials.size() >= (mat+1) // i != 0
						//~ && materials[mat]->vpMaterialTextures.size() >= (tex+1) 
						//~ && CHECK_PTR(materials[mat]->vpMaterialTextures[tex]) )
		if(  	 materials.size() >= (mat+1) // точно есть mat-й матерал
						&& materials[mat]->vpMaterialTextures.size() >= (tex+1) // точно есть tex-ая текстура
						&& CHECK_PTR(materials[mat]->vpMaterialTextures[tex]) )
		{
			#ifdef ENTITY_DEBUG
				std::cout << "\t\t\tEntity [" << entityName << "] has material [" << mat << "] and it has texture [" << tex << "]." << std::endl; //printVarComment("Material ", mat)
			#endif
			materials_textures[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			materials_textures[i].imageView = materials[mat]->vpMaterialTextures[tex]->_textureImageView;
			materials_textures[i].sampler = materials[mat]->vpMaterialTextures[tex]->_textureSampler;
		}
		
	}
	
	//~ printMessage("flag 3")	
	
	// 4 - описание дескрипторов
	std::array<VkWriteDescriptorSet, 6> descriptorWrites{};

		// 4.1 PVM buffer: struct FhGeneralInfo
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = GpassDescriptorSet;
		descriptorWrites[0].dstBinding = 0;							// в вершинном (!) шейдере: layout(binding = 0) uniform UniformBufferObject
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfos[0];	//&bufferInfo;

		// 4.2 Image samplers 0,1,2,3
		for(int b=1; b<5; b++)	// заполнение биндингов материалов 0,1,2,3, каждый по 4 текстуры, все 16 текстур описаны в materials_textures 
		{
			descriptorWrites[b].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[b].dstSet = GpassDescriptorSet;
			descriptorWrites[b].dstBinding = b;							// в фрагментном (!) шейдере: layout(binding = 1) uniform sampler2D material0_textures;
			descriptorWrites[b].dstArrayElement = 0;
			descriptorWrites[b].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[b].descriptorCount = MAX_SAMPLERS_PER_MATERIAL;// MAX_MATERIALS_PER_ENTITY;	// второй binding вычисляется как dstBinding + (descriptorCount-1)? Количество текстур в дескрипторе
			descriptorWrites[b].pImageInfo = materials_textures.data() + (b-1)*MAX_SAMPLERS_PER_MATERIAL;//MAX_MATERIALS_PER_ENTITY;	// из общего массива materials_textures берем начало с соответствующим смещением
		}

		//~ printMessage("flag 4")	

		// material infos buffer: struct FhMaterialInfo
		descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[5].dstSet = GpassDescriptorSet;
		descriptorWrites[5].dstBinding = 5;							// в вершинном (!) шейдере: layout(binding = 5) uniform FhMaterialInfo
		descriptorWrites[5].dstArrayElement = 0;
		descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[5].descriptorCount = 1;
		descriptorWrites[5].pBufferInfo = &bufferInfos[1];

		//~ // LIGHTS buffer: struct FhIlluminationInfo
		//~ descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//~ descriptorWrites[6].dstSet = GpassDescriptorSet;
		//~ descriptorWrites[6].dstBinding = 6;							// в вершинном (!) шейдере: layout(binding = 6) uniform FhIlluminationInfo 
		//~ descriptorWrites[6].dstArrayElement = 0;
		//~ descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//~ descriptorWrites[6].descriptorCount = 1;
		//~ descriptorWrites[6].pBufferInfo = &bufferInfos[2];
		

	//~ printMessage("Descriptor set of entity updated")	


	// 5 - обновление содержимого
	vkUpdateDescriptorSets(g_GraphicsCore.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	#ifdef ENTITY_DEBUG
		printVarComment("\t Descriptor set of entity updated!", entityName ) 
	#endif
}


std::string  Entity::getTexturePath(std::string& filename)	
{	
	#ifdef _WIN32
		return "datas\\textures\\" + filename + ""; 
	#elif
		return "./datas/textures/" + filename + "" ;		
	#endif
}



void Entity::configureModelMatrix(glm::mat4& matrix)//, const glm::vec3& setWorldLocation, const glm::quat& setWorldRotation)
{
	//~ glm::vec3 entityPosition = glm::vec3(0, 0, 0);
	//~ glm::quat entityRotation = glm::quat(1, 0, 0, 0);

	//~ matrix = glm::translate(matrix, setWorldLocation + setWorldRotation * entityPosition) 	//	матрицу ориентации домножаем
		//~ * mat4_cast(setWorldRotation * entityRotation);  							//	на матрицу переноса и оставляем это в ссылочном аргументе
		
	//~ printMessage("configureModelMatrix ")	
	
		
	entityMatrix = glm::translate(glm::mat4(1.0f), MicroWorldPosition + MicroWorldRotation * position) 	//	матрицу ориентации домножаем
		* mat4_cast(MicroWorldRotation * rotation) * glm::scale(glm::mat4(1.0f), scale);
		
	//~ printVec3(MicroWorldPosition)
	//~ printQuat(MicroWorldRotation)
	//~ printMat4("Matrix: ", matrix)
}

void Entity::setLocalPosition( glm::vec3 pos )
{
	position = pos;
}

void Entity::setLocalRotation( glm::quat rot )
{
	rotation = rot;
}



//~ uint32_t Entity::getMeshVerticesCount()
//~ {
	//~ if( CHECK_PTR(entityMeshPtr) )
	//~ {
		//~ return static_cast<uint32_t>(entityMeshPtr->meshVertexDataVector.size() );
	//~ }
	//~ else
	//~ {
		//~ return static_cast<uint32_t>( 0 );
	//~ }
//~ }




