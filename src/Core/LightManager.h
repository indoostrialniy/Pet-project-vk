#pragma once

//~ #include "MicroWorld.h"

#include "fhStructs.h" // FhMicroWorldInitializationInfo
#include "fhEnumerations.h"
#include "globals.h"

class Graphics;

class LightManager
{
public:
	LightManager(Graphics& core);
	~LightManager();
	
	// хранит 2 типа источников - omni и spot
	// соответственно, 2 пайплайна и 2 группы ресурсов
	
	Graphics& g_GraphicsCore;
	
	std::vector<std::shared_ptr<DynamicLight>> spots;
	
	std::vector<std::shared_ptr<DynamicLight>> omnis;
	
	VkExtent2D shadowExtent;
	    
    void recordShadowpassCommand( VkCommandBuffer& commandBuffer );
    
    void configureRenderpass(VkRenderPass& pass);
    
    
    
    // if FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL. MAX_DYNAMIC_LIGHT_SOURCES = 16
    void createOmnidirectionalPipeline();
    void destroyOmnidirectionalPipeline();
    
    constexpr int 					cubemapEdgesCount = 6;
    
    VkRenderPass					renderPass_omni 				= VK_NULL_HANDLE;
    VkDescriptorSetLayout 			descriptorSetLayout_omni 		= VK_NULL_HANDLE; 		// описание "портов" шейдера - т.е. точек подключения/привязки неких ресурсов. Для возможности подключать к шейдеру ресурсы (буферы/семплеры) обязательно нужен VkDescriptorSetLayout, даже если де-факто ресурсов не предвидится
    VkDescriptorPool 				descriptorPool_omni 			= VK_NULL_HANDLE;		// объект, выделяющий и владеющий местом для descriptor sets? пул дескрипторов для шейдерных переменных - один. Надо удалять вручную
    VkPipelineLayout 				pipelineLayout_omni 			= VK_NULL_HANDLE;		// обобщенное описание (доступа конвейера к) ЛЮБЫХ передаваемых в шейдеры переменных - ресурсов (descriptorSetLayout) и чисел (push-констант), которые могут быть обновлены во время отрисовки. pipeline layout используется при "жизни" программы, поэтому надо вручную удалить в конце
    VkPipeline 						pipeline_omni 					= VK_NULL_HANDLE;		// может ссылаться на descriptorSetLayout. Это "рецепт" генерации кадра: какие этапы + каким рендерпасом + какие могут быть точки подключения
    
    std::vector<VkFramebuffer>   	frameBuffers_omni;
    std::vector<VkImage>			cubeMaps;
	std::vector<VkDeviceMemory> 	cubeMapsMemory;	
	std::vector<VkImageView> 		cubeMapsView;
	std::vector<VkImageView> 		cubeMapsViewSampling;				// to L-pass descriptor set
	std::vector<VkSampler> 			cubeMapsSampler;					// to L-pass descriptor set
	std::vector<VkDescriptorSet> 	cubemapDescriptorSet;			//= VK_NULL_HANDLE;
    
    void createEdgeMatricesBuffer();
	void destroyEdgeMatricesBuffer();
	void updateEdgeMatricesBuffer();
	
	VkBuffer						edgeMatricesBuffer				= VK_NULL_HANDLE;
	VkDeviceMemory					edgeMatricesBufferMemory		= VK_NULL_HANDLE;
	void*							edgeMatricesBufferMapped		= nullptr;
    
    
    
    
    // if FH_DYNAMIC_LIGHTSOURCE_SPOT
    void createSpotPipeline();
    void destroySpotPipeline();
    
    VkRenderPass					renderPass_spot 				= VK_NULL_HANDLE;
    VkDescriptorSetLayout 			descriptorSetLayout_spot 		= VK_NULL_HANDLE; 		// описание "портов" шейдера - т.е. точек подключения/привязки неких ресурсов. Для возможности подключать к шейдеру ресурсы (буферы/семплеры) обязательно нужен VkDescriptorSetLayout, даже если де-факто ресурсов не предвидится
    VkDescriptorPool 				descriptorPool_spot 			= VK_NULL_HANDLE;		// объект, выделяющий и владеющий местом для descriptor sets? пул дескрипторов для шейдерных переменных - один. Надо удалять вручную
    VkPipelineLayout 				pipelineLayout_spot 			= VK_NULL_HANDLE;		// обобщенное описание (доступа конвейера к) ЛЮБЫХ передаваемых в шейдеры переменных - ресурсов (descriptorSetLayout) и чисел (push-констант), которые могут быть обновлены во время отрисовки. pipeline layout используется при "жизни" программы, поэтому надо вручную удалить в конце
    VkPipeline 						pipeline_spot 					= VK_NULL_HANDLE;		// может ссылаться на descriptorSetLayout. Это "рецепт" генерации кадра: какие этапы + каким рендерпасом + какие могут быть точки подключения
    
    std::vector<VkFramebuffer>   	frameBuffers_spot;
    std::vector<VkImage> 			spotsImage;
    std::vector<VkDeviceMemory> 	spotsImageMemory;
    std::vector<VkImageView> 		spotsImageView;							// to L-pass descriptor set
    std::vector<VkSampler> 			spotsImageSampler;							// to L-pass descriptor set
   	VkDescriptorSet 				dynamicLightSpotSet				= VK_NULL_HANDLE;		// для передачи во фрагментный шейдер параметров источника

	
	//------------------------------------------------------------------
	
    
	
};


