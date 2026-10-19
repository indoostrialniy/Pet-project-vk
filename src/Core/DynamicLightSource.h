#pragma once

#include "MicroWorld.h"

#include "fhStructs.h" // FhMicroWorldInitializationInfo
#include "fhEnumerations.h"
#include "globals.h"

//~ class Graphics;

class DynamicLight : public MicroWorld 
{
public:
	DynamicLight( const FhMicroWorldInitializationInfo& info );
	~DynamicLight();
		
	VkExtent2D shadowExtent;
	
	float	fLightViewNear 	= 0.1f;
	
	uint32_t	type 		= FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;
	
	glm::vec3 	color 		= glm::vec3(1.0, 1.0, 1.0);
	
	float		sector		= 90.0f;	// ширина поля зрения
	float		radius		= 10.0f;	// дальность действия источника
	float		power		= 10.0f;
	float		blendWidth	= 10.0f;	//зона плавной границы для фонарика
	
	uint32_t	index		= 0;		// порядковый номер данного источника
	
	FhDynamicLightSourceInfo getLightSourceInfo();
	
	glm::mat4 getLightSourcePV();
    
    void recordShadowpassCommand( VkCommandBuffer& commandBuffer );
    
	// карты теней. От 1 до 6
    uint32_t					dynamicLightMapsCount = 1;
	
	// общее
	VkRenderPass				renderPass_Shadow 				= VK_NULL_HANDLE;
    VkDescriptorSetLayout 		descriptorSetLayout_Shadow 		= VK_NULL_HANDLE; 		// описание "портов" шейдера - т.е. точек подключения/привязки неких ресурсов. Для возможности подключать к шейдеру ресурсы (буферы/семплеры) обязательно нужен VkDescriptorSetLayout, даже если де-факто ресурсов не предвидится
    VkDescriptorPool 			descriptorPool_Shadow 			= VK_NULL_HANDLE;		// объект, выделяющий и владеющий местом для descriptor sets? пул дескрипторов для шейдерных переменных - один. Надо удалять вручную
    VkPipelineLayout 			pipelineLayout_Shadow 			= VK_NULL_HANDLE;		// обобщенное описание (доступа конвейера к) ЛЮБЫХ передаваемых в шейдеры переменных - ресурсов (descriptorSetLayout) и чисел (push-констант), которые могут быть обновлены во время отрисовки. pipeline layout используется при "жизни" программы, поэтому надо вручную удалить в конце
    VkPipeline 					pipeline_Shadow 				= VK_NULL_HANDLE;		// может ссылаться на descriptorSetLayout. Это "рецепт" генерации кадра: какие этапы + каким рендерпасом + какие могут быть точки подключения
    VkFramebuffer   			frameBuffer_Shadow				= VK_NULL_HANDLE; 

    
    // if FH_DYNAMIC_LIGHTSOURCE_SPOT
    std::vector<VkImage> 		dynamicLightShadowImage;
    std::vector<VkDeviceMemory> dynamicLightShadowImageMemory;
    std::vector<VkImageView> 	dynamicLightShadowImageView;							// to L-pass descriptor set
    std::vector<VkSampler> 		dynamicLightShadowImageSampler;							// to L-pass descriptor set
   	VkDescriptorSet 			dynamicLightSpotSet				= VK_NULL_HANDLE;		// для передачи во фрагментный шейдер параметров источника

    
    // if FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL
    VkImage 					cubeMap							= VK_NULL_HANDLE;
	VkDeviceMemory 				cubeMapMemory					= VK_NULL_HANDLE;
	VkImageView 				cubeMapView						= VK_NULL_HANDLE;
	VkImageView 				cubeMapViewSampling				= VK_NULL_HANDLE; 		// to L-pass descriptor set
	VkSampler 					cubeMapSampler					= VK_NULL_HANDLE; 		// to L-pass descriptor set
	VkDescriptorSet 			dynamicLightOmniSet				= VK_NULL_HANDLE;
	
	void createEdgeMatricesBuffer();
	void destroyEdgeMatricesBuffer();
	void updateEdgeMatricesBuffer();
	
	VkBuffer					edgeMatricesBuffer				= VK_NULL_HANDLE;
	VkDeviceMemory				edgeMatricesBufferMemory		= VK_NULL_HANDLE;
	void*						edgeMatricesBufferMapped		= nullptr;
	
	//------------------------------------------------------------------
	
    
    //~ VkImage         image       = VK_NULL_HANDLE;
    //~ VkDeviceMemory  memory      = VK_NULL_HANDLE;
    //~ VkImageView     cubeView    = VK_NULL_HANDLE;  // для сэмплинга в шейдере
    //~ std::vector<VkImageView>     faceViews;              // для depth attachment
    //~ VkSampler       sampler     = VK_NULL_HANDLE;
    //~ std::vector<VkFramebuffer>   frameBuffer_Shadow;           // по одному на грань
    
    
    
    
    void update();
	
    
    
    
    void createShadowpassSpot();
    void destroyShadowpassSpot();
    
    void createShadowpassOmni();
    void destroyShadowpassOmni();
	
};


