#include "DynamicLightSource.h"

#include "macros.h"
#include "fhRender.h"
#include "Laboratory.h"
#include "Entity.h"
#include "Mesh.h"

//~ DynamicLight::DynamicLight() {}
#include "fhRender.h"

//~ #define DYNAMICLIGHT_DEBUG


DynamicLight::DynamicLight( const FhMicroWorldInitializationInfo& info ) : MicroWorld( info.vulkanCore )
{
	MicroWorldName = info.name; 
	
	#ifdef DYNAMICLIGHT_DEBUG
		printVarComment("\n\tConstructor of: ", info.name)
	#endif
	
	position 	= info.initPos; // start position
	rotation 	= info.initRot;	// start rotation
	
	color 		= info.color;
	
	type 		= info.type;
	sector 		= info.sector;
	radius 		= info.radius;
	power 		= info.power;
	blendWidth 	= info.blendWidth;
	index 		= info.index;
	
	shadowExtent.width 	= SHADOWMAP_WIDTH;
	shadowExtent.height = SHADOWMAP_HEIGHT;
			
	switch( DynamicLight::type )
	{
		case FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL: 	dynamicLightMapsCount = 6; createShadowpassOmni(); break;
		
		case FH_DYNAMIC_LIGHTSOURCE_SPOT: 				dynamicLightMapsCount = 1; createShadowpassSpot(); break;
		
		//case FH_DYNAMIC_LIGHTSOURCE_SUN: 				dynamicLightMapsCount = 1; createShadowpassSun(); break;
	};
}
	
	
DynamicLight::~DynamicLight()
{
	#ifdef DYNAMICLIGHT_DEBUG
		printVarComment("Destructor of dynamic light: ", MicroWorldName)
	#endif
	
	switch( DynamicLight::type )
	{
		case FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL: 	destroyShadowpassOmni(); break;
		
		case FH_DYNAMIC_LIGHTSOURCE_SPOT: 				destroyShadowpassSpot(); break;
		
		//case FH_DYNAMIC_LIGHTSOURCE_SUN: 				destroyShadowpassSun(); break;
	};
}


void DynamicLight::createShadowpassSpot()
{
	if( !HANDLE_VALID(g_GraphicsCore.logicalDevice) ) {return;}
	
	// 1 - Renderpass attachments
	if( HANDLE_NULL(renderPass_Shadow) )
	{
		VkAttachmentDescription depthAttachment;
			depthAttachment.format = VK_FORMAT_D32_SFLOAT;//g_GraphicsCore.findDepthFormat();
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.flags = 0;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;						// 1 => без мультисемплинга
			// loadOp и storeOp относятся к цвету и глубине 
			// мы знаем, что координаты модели после всех матричных умножений преобразуются в вектор, у которого X и Y это экранные координаты,
			// а компонента Z - экранная глубина пикселя
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// заливать буфер одним цветом перед каждым кадром (назначать всем значениям некую константу)
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;					// результат отрисовки будет сохранен в память и может быть считан потом
			// пока что нам плевать на буфер трафарета, поэтому DONT_CARE
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			// текстуры и фреймбуферы  представляются как VkImage с определенным форматом пикселей, однако расположение пикселей в памяти может меняться
			// в зависимости от того, что мы пытаемся сделать с изображением
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// плевать, где текстурное прикрепление было расположено ранее
			// VK_IMAGE_LAYOUT_UNDEFINED не гарантирует, что данные из изображения будут сохранены, но мы все равно собирались делать заливку VK_ATTACHMENT_LOAD_OP_CLEAR
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		// (автоматически) целевое расположение текстурного прикрепления - цепочка показа
			// изображение должно быть перенесено в специальное расположение, доступное той функции/операции, которая будет обрабатывать это изображение далее

		// 1.1 - субпасс
		// рендерпасс может состоять из множества субпассов. Субпассы это операции рендеринга, зависящие от содержимого фреймбуфера от предыдущего рендерпасса
		// (к примеру, последовательность пост-процессинговых эффектов, применяющихся друг за другом)
		// Если сгруппировать эти операции рендеринга в едииный рендерпасс, то Vulkan может их реорганизовать и выжать больше производительности .
		// Для тестового треугольника оставим одиночный субпасс
		// каждый субпасс ссылается на 1 или несколько прикреплений, описанных ранее. Эти ссылки и есть структуры VkAttachmentReference
		
		// 1.2 - references
		
		std::array<VkAttachmentReference, 1> depthAttachmentsRefs {};
			depthAttachmentsRefs[0].attachment = 0;
			depthAttachmentsRefs[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			
		// а можно и в один массив все засунуть с pColorAttachments указывать весь colorAttachmentsRefs.data() но число минус 1, а в depth класть последнюю ячейку из него
			
		// 2.2 - subpass
		VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;	// т.к. Vulkan также может подерживать compute subpasses, надо явно указать, что этот субпасс - графический!
			subpass.colorAttachmentCount = 0;//static_cast<uint32_t>(colorAttachmentsRefs.size());			// количество цветовых прикреплений
			subpass.pColorAttachments = nullptr;//colorAttachmentsRefs.data();				// ссылка на текстурное прикрепление цвета
			
			subpass.pDepthStencilAttachment = depthAttachmentsRefs.data();	//&depthAttachmentRef;
		// субпассы автоматически учитывают изменения в расположении изображений 
		// эти переходы контролируются зависимостями субпасса, которые определяют память и исполнительные зависимости между субпассами
		
		
		// 3 - dependencies
		VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	// VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render pass
			dependency.dstSubpass = 0;		// The index 0 refers to our subpass, which is the first and only one
			// The dstSubpass must always be higher than srcSubpass to prevent cycles
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;	// ожидающие операции. wait for the swap chain to finish reading from the image
			dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;	// этапы, на которых эти ожидающие операции появляются
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


		// 4 - renderpass creation
		VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;							//static_cast<uint32_t>(attachments.size());				// указание количества прикреплений
			renderPassInfo.pAttachments = &depthAttachment;				//attachments.data();										// привязка прикреплений к рендерпасу
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(g_GraphicsCore.logicalDevice, &renderPassInfo, nullptr, &renderPass_Shadow);

		if( result != VK_SUCCESS ) 
		{
			g_GraphicsCore.checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create Shadow render pass!");
		}
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow render pass OK")
			DEBUG_INFO
		#endif
	}
	
	
	// 2 - Images
	{
		dynamicLightShadowImage.resize(dynamicLightMapsCount, VK_NULL_HANDLE);
		dynamicLightShadowImageMemory.resize(dynamicLightMapsCount, VK_NULL_HANDLE);
		dynamicLightShadowImageView.resize(dynamicLightMapsCount, VK_NULL_HANDLE);
		dynamicLightShadowImageSampler.resize(dynamicLightMapsCount, VK_NULL_HANDLE);
				
			
		for(int i=0; i<dynamicLightMapsCount; i++)
		{
			// depth
			VkFormat depthFormat = g_GraphicsCore.findDepthFormat();	// VK_FORMAT_D32_SFLOAT;//
			
			g_GraphicsCore.createImage(shadowExtent.width, shadowExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
						dynamicLightShadowImage[i], dynamicLightShadowImageMemory[i] );
			
			dynamicLightShadowImageView[i] = g_GraphicsCore.createImageView( dynamicLightShadowImage[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
			
			g_GraphicsCore.createTextureSampler( dynamicLightShadowImageSampler[i] );
		}
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow images OK")
			DEBUG_INFO
		#endif
	}
	
	
	// 3 - Framebuffer
	if( HANDLE_NULL(frameBuffer_Shadow) && HANDLE_VALID(renderPass_Shadow) )
	{		
		VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass_Shadow;	// с каким рендерпасом фреймбуфер должен быть совместим
			framebufferInfo.attachmentCount = static_cast<uint32_t>(dynamicLightShadowImageView.size());
			framebufferInfo.pAttachments = dynamicLightShadowImageView.data();
			framebufferInfo.width = shadowExtent.width;
			framebufferInfo.height = shadowExtent.height;
			framebufferInfo.layers = dynamicLightMapsCount;			// 1. Количество под-изображений. у кубмапы было бы 6, а у фонаря - одно

		if( vkCreateFramebuffer(g_GraphicsCore.logicalDevice, &framebufferInfo, nullptr, &frameBuffer_Shadow) != VK_SUCCESS )
		{
			throw std::runtime_error("failed to create Shadow framebuffer!");
		}
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow framebuffer OK")
			DEBUG_INFO
		#endif
	}
	
	
	// 4 - DescriptorSetLayout
	if( HANDLE_NULL(descriptorSetLayout_Shadow)  )
	{
		// схема разъема. описание точек привязки только ресурсов (буферов, изображений) к шейдерам
		/*
			VkDescriptorSetLayout describes shader binding points - which types of resource to bind to which place
			For example: to vertex shader we connect un UBO - this will be "binding 0"
						 to fragment shader we connect texture sampler - this will be "binding 1"
		*/
		
		std::array<VkDescriptorSetLayoutBinding, 1> bindings = {};
			// binding 0 -  project and view matrices - layout(binding = 0) uniform FhGeneralInfo {} commonData
				bindings[0].binding = 0;												// индекс точки привязки: layout(binding = 0) uniform DynamicLightSourceInfo из фрагментного шейдера
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;			// тип - uniform buffer
				bindings[0].descriptorCount = 1;										// UBO-шек может быть много, но сейчас в массиве - только одна. This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example
				bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;					// определили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
				bindings[0].pImmutableSamplers = nullptr; 								// Optional. Для изображений (image sampling)	
		
		// 3 - summarize all binding
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();
			//~ layoutInfo.bindingCount = 0;
			//~ layoutInfo.pBindings = nullptr;
			
			
		// 4 - create VkDescriptorSetLayout
		VkResult result = vkCreateDescriptorSetLayout(g_GraphicsCore.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout_Shadow);
		if( result != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(result);
			throw std::runtime_error("Failed to create Shadow descriptor set layout!");
		}
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow DescriptorSetLayout OK")
			DEBUG_INFO
		#endif
	}
	
	
	// 5 - Descriptor Pool
	if( HANDLE_NULL(descriptorPool_Shadow)  )
	{
		// набор разъемов. создание общего перечня сетов.
		/*
			Descriptor pool — это объект, из которого Vulkan выделяет descriptor set-ы. Ты заранее указываешь, сколько дескрипторов каких типов тебе нужно, и pool использует это как резерв памяти/ресурсов для будущих set-ов.

			VkDescriptorPool представляет собой ресурс, используемый для управления набором дескрипторов. 
			Pool позволяет эффективно управлять созданием и удалением дескрипторных наборов (descriptor sets).
			VkDescriptorPool служит инструментом эффективного распределения и повторного использования ресурсов, позволяя избежать 
			затратных операций выделения памяти при каждом изменении состояния графического конвейера.

		*/
		
		
		//printVarComment("Descriptor pool size: ", MAX_ENTITIES_IN_ACTION)
		//DEBUG_INFO
		
		// 1 - Сначала нужно описать, какие типы дескрипторов содержатся в сетах дескрипторов, и их количество/
		// То есть какие патроны набивать в магазин, помещаемый в разгрузку (пул)
		// Есть пока всего 2 типа передаваемых в шейдер (конвейер) ресурсов - UBO и Image sampler, создадим массив с описанием обоих типов ресурсов
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
			// 1.0 - в движке будет (MAX_FRAMES_IN_FLIGHT * MAX_ENTITIES_IN_ACTION) UBO-шек FhGeneralInfo
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount =  10; // дескриптор нужен для хранения инфы о самом источнике, а не объектах сцены
			
		// 2 - зная все возможные дескрипторы, описываем общий пул для хранения дескрипторов
		VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
			poolInfo.pPoolSizes = poolSizes.data();
			//~ poolInfo.poolSizeCount = 0;
			//~ poolInfo.pPoolSizes = nullptr;
			
			int deferredFrames = 1; 			// при отложенном рендеринге буферизация одинарная, всего один кадр
			poolInfo.maxSets = deferredFrames;// * MAX_ENTITIES_IN_ACTION; //  но зато отрисовываемых сущностей - ого-го!

		// 3 - и выделяем непрерывный блок памяти (pool memory) достаточного размера
		VkResult result = vkCreateDescriptorPool(g_GraphicsCore.logicalDevice, &poolInfo, nullptr, &descriptorPool_Shadow);
		if( result != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(result);
			throw std::runtime_error("failed to create Shadow descriptor pool!");
		}
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow pool OK")
			DEBUG_INFO
		#endif   
	}
	
	
	//~ createLightSourceDataBuffer();
	//~ updateLightSourceDataBuffer();
	
	// Descriptor Set
	// 5.1 - dynamicLightSpotSet
	if(    HANDLE_NULL(dynamicLightSpotSet) 
		&& HANDLE_VALID(descriptorPool_Shadow) 
		&& HANDLE_VALID(descriptorSetLayout_Shadow) 
		&& HANDLE_VALID(g_GraphicsCore.LpassDynamicLightSourceInfoBuffer) )
	{
		VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool_Shadow;				// указываем пул			
			allocInfo.descriptorSetCount = 1;										// и число сетов
			allocInfo.pSetLayouts = &descriptorSetLayout_Shadow; 			// указываем схему описания подключения ресурсов к конвейеру
	
		// 2 - вырезали кусок памяти из pool memory для конкретного descriptor set (взяли розетки со склада)
		VkResult result = vkAllocateDescriptorSets( g_GraphicsCore.logicalDevice, &allocInfo, &dynamicLightSpotSet );

		if( result != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("Failed to allocate L-pass spot descriptor set!");
		}
		
		// 3 - связываем реальные ресурсы с дескрипторами
		// 3.1 информация о буфере матриц PVM	
		
		// Получаем лимит устройства (один раз при инициализации)
		//~ uint64_t alignment = 64;//g_GraphicsCore.deviceLimits.minUniformBufferOffsetAlignment; 

		// Твой текущий расчет
		//~ uint64_t rawOffset = DynamicLight::index * sizeof(FhDynamicLightSourceInfo);

		// ОКРУГЛЕНИЕ ВВЕРХ до ближайшего кратного alignment
		// Формула: ((x + alignment - 1) / alignment) * alignment
		//~ uint64_t alignedOffset = (rawOffset + alignment - 1) & ~(alignment - 1); 
		// Или более читаемый вариант:
		// uint64_t alignedOffset = ((rawOffset + alignment - 1) / alignment) * alignment;

		//~ bufferInfos[0].offset = alignedOffset;
		//~ bufferInfos[0].range = sizeof(FhDynamicLightSourceInfo); // Диапазон оставляем как есть
		
		//~ const uint32_t stride = sizeof(FhDynamicLightSourceInfo); // 512
		//~ const VkDeviceSize offset = static_cast<VkDeviceSize>(DynamicLight::index) * stride; // 0, 512, 1024, ...\	
		
		//~ printMessage("\n\n\n\n=========================================")
		//~ printVariable(stride)
		//~ printVariable(offset)
		
			
		std::array<VkDescriptorBufferInfo, 1> bufferInfos{};
			// binding 0
			bufferInfos[0].buffer = g_GraphicsCore.LpassDynamicLightSourceInfoBuffer;	// ссылка на конкретный буфер на 16 структур об источниках UBO
			bufferInfos[0].offset = DynamicLight::index * sizeof(FhDynamicLightSourceInfo);// 512;// offset;// alignedOffset;//DynamicLight::index * sizeof(FhDynamicLightSourceInfo); // из 16 подряд структур сместимся до нужного куска
			bufferInfos[0].range = sizeof(FhDynamicLightSourceInfo);				// и от начала нужного куска нужна только одна структура
				
		// 4 - описание дескрипторов
		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

			// 4.1 6 PV matrices buffer: struct FhShadowpassCubeEdgesInfo
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = dynamicLightSpotSet;
			descriptorWrites[0].dstBinding = 0;							// во фрагментном (!) шейдере: layout(binding = 0) uniform DynamicLightSourceInfo
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfos[0];

		// 5 - обновление содержимого
		vkUpdateDescriptorSets( g_GraphicsCore.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("\t Descriptor set of Shadow spot-pass updated!") 
		#endif
	}
	
	
	
	// 6 - Graphics Pipeline Layout
	if( HANDLE_NULL(pipelineLayout_Shadow) && HANDLE_VALID(descriptorSetLayout_Shadow) )
	{
		// схема прибора. объединение VkDescriptorSetLayout + Push-Constants: общее описание точек привязки данных (ресурсов и переменных)
		// Даже если не намерены передавать в шейдер данные, все равно обязательно нужно создать один пустой VkPipelineLayout!
		// VkPipelineLayout надо обязательно прописать до создания самого конвейера!
		
		//setup push constants
		VkPushConstantRange push_constant;
			push_constant.offset = 0;									//this push constant range starts at the beginning
			push_constant.size = sizeof(FhShadowpassPushConstantsInfo);				//this push constant range takes up the size of a MeshPushConstants struct
			push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		//this push constant range is accessible only in the vertex shader
		
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			
			// конвейеру должны быть доступны:
			// 1 - описания точек привязки буферов и изображений
			pipelineLayoutInfo.setLayoutCount = 1;						
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_Shadow; 		// тут может быть массив, но пока только один
			
			// 2 - описания т.н. пуш-констант, которые являются другим способом передачи динамических значений в шейдер
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		
		VkResult result = vkCreatePipelineLayout(g_GraphicsCore.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout_Shadow);
		if( result != VK_SUCCESS ) 
		{
			g_GraphicsCore.checkResult(result);
			throw std::runtime_error("failed to create Shadow pipeline layout!");
		}
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow pipeline layout OK")
			DEBUG_INFO
		#endif
	}
	
	
	// 7 - Graphics Pipeline
	if( HANDLE_NULL(pipeline_Shadow) && HANDLE_VALID(renderPass_Shadow) && HANDLE_VALID(pipelineLayout_Shadow) )
	{
		// создание графического конвейера: этапы, шейдеры, смешивание + описание точек привязки для передачи данных в конвейер
		// 1 - программируемые этапы конвейера
		// 1.1 - загрузить байт-код
		auto vertShaderCode = g_GraphicsCore.readFile( FH_SHADER_PATH  "spot\\Shadow_pass_v.spv");		// auto vertShaderCode - это статичный вектор символов-байтов
		auto fragShaderCode = g_GraphicsCore.readFile( FH_SHADER_PATH  "spot\\Shadow_pass_f.spv");

		// 1.2 - собрать из байт-кода шейдерный модуль ("обернуть" байт-код в Vulkan)
		VkShaderModule vertShaderModule = g_GraphicsCore.createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = g_GraphicsCore.createShaderModule(fragShaderCode);
		
		//~ Shader modules are just a thin wrapper around the shader bytecode that we’ve
		//~ previously loaded from a file and the functions defined in it. The compilation
		//~ and linking of the SPIR-V bytecode to machine code for execution by the GPU
		//~ doesn’t happen until the graphics pipeline is created.
		
		// VkShaderModule располагается тут локально, так как не будет нужен после завершения настройки Pipeline,
		// поэтому в конце данной функции он вручную удаляется	

		// 1.3 - преобразуем шейдерные модули в шейдерные программы для этапов: вершинного и фрагментного
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// 18
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;								// 0x00000001
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";
			//vertShaderStageInfo.pSpecializationInfo = nullptr; // сюда можно указать заранее известные шейдерные константы, ради оптимизации

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// 18
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";
			//fragShaderStageInfo.pSpecializationInfo = nullptr;

		// 1.4 - в нижеприведенном массиве описываем последовательность графических этапов: сначала обработка вершин, потом - фрагментов
		// также сюда можно потом дополнительно добавлять геометрические, тесселяцию и т.д.
		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		// 2 - фиксированные этапы конвейера		
		// VkPipelineVertexInputStateCreateInfo describes the format of the vertex data that will be passed to the vertex shader
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};			
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;	// 19
			
		auto bindingDescription = Vertex2::getBindingDescription();
		auto attributeDescriptions = Vertex2::getAttributeDescriptions();
		
			vertexInputInfo.vertexBindingDescriptionCount = 1;						// количество привязок вершин
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; 		// настройка привязки
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>( attributeDescriptions.size() );		// количество атрибутов
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); 	// данные об атрибутах
			
			// в случае, если вершинные данные захардкожены, нечего настраивать
			//~ vertexInputInfo.vertexBindingDescriptionCount = 0;			// привязок нет. spacing between data and whether the data is per-vertex or per-instance
			//~ vertexInputInfo.pVertexBindingDescriptions = nullptr; 		// Optional
			//~ vertexInputInfo.vertexAttributeDescriptionCount = 0;		// атрибутов нет
			//~ vertexInputInfo.pVertexAttributeDescriptions = nullptr; 	// Optional
			
			
		// 2.2 - Input assembly stage		
		// VkPipelineInputAssemblyStateCreateInfo describes two things: what kind of geometry will be drawn from the vertices (point, line, triangle etc)
		// and if primitive restart should be enabled.
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;	// 20
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		//рисовать треугольники
			inputAssembly.primitiveRestartEnable = VK_FALSE;
		// используя Elements Buffer, можно переиспользовать вершины при отрисовке. Это плюс к оптимизации.


		// 2.3 - Viewport and Scissor
		// Вьюпорт описывает область фреймбуфера, куда выход будет рендерить. Почти всегда размер вьюпорта совпадает с размерами окна.
		// viewports define the transformation from the image to the framebuffer
		// влияет на сжатие/растяжение изображения на экране
		
		// Ножничные прямоугольники определяют, в каком регионе по-настоящему будут размещены пиксели. (это связано с перемещением окна по экрану?)
		// Любые пиксели вне этого прямоугольника будут отброшены растеризатором.
		// Т.е. ножницы работают скорее как фильтр, а не преобразователь
		
		// Viewport и scissors можно задать либо как статическую часть графического конвейера либо как динамическое состояние, задаваемое через command buffer

		// 2.3.1 - ДИНАМИЧЕСКИ ИЗМЕНЯЕМОЕ СОСТОЯНИЕ
		// при динамическом состоянии нужно явно указать в dynamicStates, какие именно параметры (а их не слишком много) будут динамически изменяемы (без пересоздания конвейера)
		// в большинстве случаем это более удобный/предпочитаемый вариант - дает больше гибкости
		// Нужно активировать соответствующие состояния для конвейера...
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		
		VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
			dynamicState.pDynamicStates = dynamicStates.data();
			
		// ... и прописать их количество в viewportState ...
		VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;	// 22
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;	
		// ... а реальные вьюпорт и ножницы затем можно будет указывать во время отрисовки (через command buffer)
		
		
		// 2.3.2 - СТАТИЧЕСКОЕ СОСТОЯНИЕ
		// при статическом состоянии, вьюпорт VkViewport и ножницы VkRect2D должны быть захардкожены в нижеприведенную структуру VkPipelineViewportStateCreateInfo
		// при этом их уже не получится менять - нужно будет переделывать полностью весь конвейер
		// закомментируем, так как намерены использовать динамическое состояние
		//~ VkViewport viewport{};
			//~ viewport.x = 0.0f;
			//~ viewport.y = 0.0f;
			//~ viewport.width = (float) swapChainExtent.width;
			//~ viewport.height = (float) swapChainExtent.height;
			//~ viewport.minDepth = 0.0f;
			//~ viewport.maxDepth = 1.0f;
		
		//~ VkRect2D scissor{};
			//~ scissor.offset = {0, 0};
			//~ scissor.extent = swapChainExtent;
		
		//~ VkPipelineViewportStateCreateInfo viewportState{};
			//~ viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;	// 22
			//~ viewportState.viewportCount = 1;
			//~ viewportState.pViewports = &viewport;
			//~ viewportState.scissorCount = 1;	
			//~ viewportState.pScissors = &scissor;			
			
		// независимо от состояния, параметры передаются как массивы, поскольку на некоторых видеокартах допускается использование нескольких viewport и scissors
			
			
					
		// 2.4 - растеризатор
		// performs depth testing, face culling and the scissor test, and it can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering).
		VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;	// если true, то это отключает вывод во фреймбуфер
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // заполнение полигона (еще есть линии и точки)
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE; // порядок следования вершин полигона для понимания, передний или задний он
			
			rasterizer.depthBiasEnable = VK_FALSE;

		// 2.5 - мультисемплер
		// способ сглаживать изображение. Требуется подключить фичу GPU
		VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


		VkPipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;		// тест глубины разрешен
			depthStencil.depthWriteEnable = VK_TRUE;	// запись в буфер глубины разрешена
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;	// предпочитаем МЕНЬШИЕ значения
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;	 // make sure that the format of the depth/stencil image contains a stencil component.


		// 2.6 - смешивание цветов
		// Когда фрагментный шейдер рассчитал цвет пикселя, надо правильно его совместить с цветом из буфера - смешать или скомбинировать.
		// конфигурация привязанного фреймбуфера:
		//~ VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			//~ colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			//~ colorBlendAttachment.blendEnable = VK_FALSE; // не совмещать (новый цвет заменяет старый из буфера)
		//~ std::array<VkPipelineColorBlendAttachmentState, 1> colorBlendAttachments = {colorBlendAttachment};

		// глобальные настройки совмещения цветов (между буферами?):
		VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 0;//static_cast<uint32_t>( colorBlendAttachments.size() );	//1;
			colorBlending.pAttachments = nullptr;//colorBlendAttachments.data();//&colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;


		// pipeline layout
		// 2.8 - объединим всю информацию для создания дескриптора графического конвейера
		VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;		//28
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;				// массив описаний этапов рендера
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			// Then we reference all of the structures describing the fixed-function stage.
			pipelineInfo.layout = pipelineLayout_Shadow;
			pipelineInfo.renderPass = renderPass_Shadow;			// createRenderPass() должна быть вызвана ДО этого момента!
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			
			pipelineInfo.pDepthStencilState = &depthStencil;

		VkResult result2 = vkCreateGraphicsPipelines(g_GraphicsCore.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_Shadow);
		if ( result2!= VK_SUCCESS)
		{
			g_GraphicsCore.checkResult(result2);
			throw std::runtime_error("failed to create Shadow graphics pipeline!");
		}

		// после создания конвейера отдельные шейдеры (вершинный/фрагментный) уже не нужны
		vkDestroyShaderModule(g_GraphicsCore.logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(g_GraphicsCore.logicalDevice, vertShaderModule, nullptr);
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow Pipeline OK")
			DEBUG_INFO
		#endif
	}
	
	
	
	status = true;
	
	#ifdef DYNAMICLIGHT_DEBUG
		printMessage("Shadowpass spot created")
	#endif
}
   
void DynamicLight::destroyShadowpassSpot()
{
	//~ destroyLightSourceDataBuffer();
	
	if( !status ) return;
	
	vkDestroyPipeline(g_GraphicsCore.logicalDevice, pipeline_Shadow, nullptr);

	vkDestroyPipelineLayout(g_GraphicsCore.logicalDevice, pipelineLayout_Shadow, nullptr);  
	
	vkDestroyDescriptorPool(g_GraphicsCore.logicalDevice, descriptorPool_Shadow, nullptr);
	
	vkDestroyDescriptorSetLayout(g_GraphicsCore.logicalDevice, descriptorSetLayout_Shadow, nullptr);
	
	if(frameBuffer_Shadow != VK_NULL_HANDLE) vkDestroyFramebuffer(g_GraphicsCore.logicalDevice, frameBuffer_Shadow, nullptr);
	
	for(auto img : dynamicLightShadowImageSampler)
	{
		vkDestroySampler(g_GraphicsCore.logicalDevice, 	img, nullptr);
	}
	
	for(auto img : dynamicLightShadowImageView)
	{
		vkDestroyImageView(g_GraphicsCore.logicalDevice, 	img, 	nullptr);
	}
	
	for(auto img : dynamicLightShadowImageMemory)
	{
		vkFreeMemory(g_GraphicsCore.logicalDevice, 		img, nullptr);
	}
	
	for(auto img : dynamicLightShadowImage)
	{
		vkDestroyImage(g_GraphicsCore.logicalDevice, 		img, 		nullptr);
	}
	
	vkDestroyRenderPass(g_GraphicsCore.logicalDevice, renderPass_Shadow, nullptr);
}




FhDynamicLightSourceInfo DynamicLight::getLightSourceInfo()
{	
	FhDynamicLightSourceInfo sourceInfo;
		sourceInfo.exists		= 1;		// раз объект класса есть, он априори существует
		sourceInfo.position 	= getActorPosition();
		sourceInfo.direction 	= getActorForwardVector();
		sourceInfo.color 		= color;
		sourceInfo.type 		= type;
		sourceInfo.sector 		= sector;
		sourceInfo.radius 		= radius;
		sourceInfo.power 		= power;
		sourceInfo.blendWidth 	= blendWidth;
		
		if(type == FH_DYNAMIC_LIGHTSOURCE_SPOT )
		{
			sourceInfo.PV[0]	= getLightSourcePV();
		}
		
		if(type == FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL )
		{
			glm::vec3 lightPos = getActorPosition();
	
			float ratio = 1.0f;//(float)shadowExtent.width / (float)shadowExtent.height;
			
			glm::mat4 proj = glm::perspective( glm::radians(sector), ratio, fLightViewNear, radius );
			// ВАЖНО: proj[1][1] должен быть положительным для Vulkan!
			// glm::perspective по умолчанию даёт отрицательный [1][1] (OpenGL-конвенция)
			//proj[1][1] *= -1;
			
			//~ float s = 10.0f; // shadow frustum size, подбери под сцену
			//~ glm::mat4 proj = glm::ortho(-s/2.0f, s/2.0f, -s/2.0f, s/2.0f, fLightViewNear, radius);

			glm::mat4 views[6];
				views[0] = glm::lookAt(lightPos, lightPos + glm::vec3( 1, 0, 0), glm::vec3(0, -1, 0));
				views[1] = glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
				views[2] = glm::lookAt(lightPos, lightPos + glm::vec3(0,  1, 0), glm::vec3(0, 0,  1));
				views[3] = glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
				views[4] = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0,  1), glm::vec3(0, -1, 0));
				views[5] = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
				//~ views[0] = glm::lookAt(lightPos, lightPos + glm::vec3( 1, 0, 0), glm::vec3(0, 0, 1));
				//~ views[1] = glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1));
				//~ views[2] = glm::lookAt(lightPos, lightPos + glm::vec3(0,  1, 0), glm::vec3(0, 0, 1));
				//~ views[3] = glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
				//~ views[4] = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0,  1), glm::vec3(0, 1, 0));
				//~ views[5] = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

			// Комбинируем: PV[face] = proj * views[face]
			for (int i=0; i < 6; i++) 
			{
				sourceInfo.PV[i] = proj * views[i];
			}
			
		}
		
	return sourceInfo;
}


glm::mat4 DynamicLight::getLightSourcePV()
{
	glm::vec3 POV = getActorPosition();
	glm::vec3 DOV = POV + getActorForwardVector();
	glm::vec3 UP = getActorUpVector();
			
	glm::mat4 view = glm::lookAt(POV, DOV, UP);
	
		
	glm::mat4 proj = glm::perspective(glm::radians(sector), (float) shadowExtent.width / (float) shadowExtent.height, fLightViewNear, radius );
			proj[1][1] *= -1; // GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. 
	
	
	return proj * view;
}


void DynamicLight::recordShadowpassCommand( VkCommandBuffer& commandBuffer )
{
	update();
	
	if( !HANDLE_VALID(commandBuffer) 		|| 
		!HANDLE_VALID(renderPass_Shadow) 	|| 
		!HANDLE_VALID(frameBuffer_Shadow) 	|| 
		!HANDLE_VALID(pipeline_Shadow) )
	{
		printVarComment("Lightsource not configured: ", MicroWorldName);
		DEBUG_INFO
		return;	
	}

	//~ switch( DynamicLight::type )
	//~ {
		//~ case FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL: printMessage("\n\n\tRun omnidirectional lightsource\n\n") break;
		//~ case FH_DYNAMIC_LIGHTSOURCE_SPOT: printMessage("\n\n\tRun Spot-lightsource\n\n") break;
	//~ };
	
	#ifdef DYNAMICLIGHT_DEBUG
		printMessage(__FUNCTION__)
	#endif
	
	// Shadow-pass
	{
		// 2 - рендерпас
		VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass_Shadow;							// the render pass itself and the attachments to bind			
			renderPassInfo.framebuffer = frameBuffer_Shadow;	// привязали к рендерпасу один из ранее созданных framebuffer кадров из цепочки, в который хотим рендерить
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = shadowExtent;
			
		std::array<VkClearValue, 1> clearValues{};
			clearValues[0].depthStencil = {1.0f, 0};
			
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			

		//~ printMessage("Flag A1")

		// 3 - привязка рендерпаса Start a new renderpass with the image index from swapchain as target to render onto
		vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
			// 3.1 - привязка графического (!!!) конвейера
			vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_Shadow); // Bind the shaders and configuration used to render the object

			// 3.2 ВЬЮПОРТ: т.к. ранее выбрали динамическое состояние для viewport и scissors, прописываем их...
			std::array<VkViewport, 6> viewports;	//	VkViewport viewport{};
				viewports[0].x = 0.0f;
				viewports[0].y = 0.0f;
				viewports[0].width = (float) shadowExtent.width;
				viewports[0].height = (float) shadowExtent.height;
				viewports[0].minDepth = 0.0f;
				viewports[0].maxDepth = 1.0f;
				
				viewports[1] = viewports[0];
				viewports[2] = viewports[0];
				viewports[3] = viewports[0];
				viewports[4] = viewports[0];
				viewports[5] = viewports[0];
				
			vkCmdSetViewport( commandBuffer, 0, dynamicLightMapsCount, viewports.data()); //vkCmdSetViewport( commandBuffer, 0, 1, &viewport);		// ... и передаем в шейдер

			std::array<VkRect2D, 6> scissors;//{ };
				scissors[0].offset = {0, 0};
				scissors[0].extent = shadowExtent;
				
				scissors[1] = scissors[0];
				scissors[2] = scissors[0];
				scissors[3] = scissors[0];
				scissors[4] = scissors[0];
				scissors[5] = scissors[0];
			vkCmdSetScissor( commandBuffer, 0, dynamicLightMapsCount, scissors.data());			// ... и передаем в шейдер

			// Внутри render pass'а может идти серия операций:
			// for (auto object : objects) {}

			// 3.3 привязываем VBO
			// Параметр массива VkBuffer в команде vkCmdBindVertexBuffers() нужен для поддержки сложных моделей вершинных данных, когда каждая такая часть хранится в отдельном буфере: VkBuffer vertexBuffers[] = { vertexBuffer, colorBuffer, uvBuffer };
			//VkBuffer vertexBuffers[] = { gameObjects[0]->vertexBuffer /*colorBuffer, uvBuffer*/ };
			VkDeviceSize offsets[] = {0 /*,0,0*/ }; // массив отступов по соответствующим буферам: vertex, color, uv etc
			//#define offsets 0
			
			FhShadowpassPushConstantsInfo constants;			// PUSH CONSTANTS
			glm::mat4 modelMatrix = glm::mat4(1.0f);	// матрица модели сущности
			
			//~ printMessage("Flag B1")
			
			// DRAW ENTITIES
			if( CHECK_PTR(g_Lab) )
			{
				for(auto act : g_Lab->actors)
				{
					for(auto ent : act.second->components)
					{
						if( CHECK_PTR(ent.second->entityMeshPtr) )
						{
							vkCmdBindVertexBuffers( commandBuffer, 0, 1, &ent.second->entityMeshPtr->getMeshVertexBuffer(), offsets);	
				
							//~ printMessage("Flag C1")
				
							if( type == FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL)
							{
								vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_Shadow, 0, 1, &dynamicLightOmniSet, 0, nullptr);	// привязали дексриптор, где хранятся ссылки на наши матрицы PVM и текстуры
							}
							if( type == FH_DYNAMIC_LIGHTSOURCE_SPOT)
							{
								vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_Shadow, 0, 1, &dynamicLightSpotSet, 0, nullptr);	// привязали дексриптор, где хранятся ссылки на наши матрицы PVM и текстуры
							}
							
							
							ent.second->configureModelMatrix( modelMatrix );
								
							constants.model = ent.second->entityMatrix;	//modelMatrix;
							
							constants.PV = getLightSourcePV();
							
							//~ printMessage("Flag C2")
							
							vkCmdPushConstants(commandBuffer, pipelineLayout_Shadow, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(FhShadowpassPushConstantsInfo), &constants);	//upload the matrix to the GPU via push constants
							
							vkCmdDraw(commandBuffer, static_cast<uint32_t>(ent.second->entityMeshPtr->getVerticesCount()), 1, 0, 0);	//vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertexOffset, firstInstanceOffset);
						}   
					}
				}
			}
			
			//~ printMessage("Flag B2")

		// 3.4 - Finalize the render pass and command buffer
		vkCmdEndRenderPass(commandBuffer);
		
		//~ printMessage("Flag A2")
	}

	
	
}



void DynamicLight::createShadowpassOmni()
{
	if( !HANDLE_VALID(g_GraphicsCore.logicalDevice) ) {return;}

	
	#ifdef DYNAMICLIGHT_DEBUG
		printMessage("\n\n\t\tcreate Shadowpass omni\n")
	#endif
	
	//dynamicLightMapsCount = 6;
	
	
	VkMemoryPropertyFlagBits properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	
	// image
	{
		VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = VK_FORMAT_D32_SFLOAT;
			imageInfo.extent = { shadowExtent.width, shadowExtent.height, 1 };
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = dynamicLightMapsCount;                          // ← 6 граней
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 
							| VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ;
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // ← важно для кубмапа
			
		VkResult result = vkCreateImage( g_GraphicsCore.logicalDevice, &imageInfo, nullptr, &cubeMap);
		
		if( result != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create cubemap image!");
		}
		
		
		
		// 2 - определение требований к памяти
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements( g_GraphicsCore.logicalDevice, cubeMap, &memRequirements);

		// 3 - выделение видеопамяти под изображение
		VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType( memRequirements.memoryTypeBits, properties, g_GraphicsCore.physicalDevice);

		VkResult allocResult = vkAllocateMemory( g_GraphicsCore.logicalDevice, &allocInfo, nullptr, &cubeMapMemory);
		if( allocResult != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(allocResult);
			DEBUG_INFO
			throw std::runtime_error("failed to allocate cubemap image memory!");
		}

		// 4 - связывание выделенной видеопамяти с host-side хэндлером
		vkBindImageMemory( g_GraphicsCore.logicalDevice, cubeMap, cubeMapMemory, 0); // 0 - это offset
	}
	
	
	// image view для рендеринга
	{
		// 1 - указание данных об изображении
		VkImageViewCreateInfo viewInfo{};									// структура для настройка обертки изображения
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = cubeMap;	//VkImage
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			viewInfo.format = VK_FORMAT_D32_SFLOAT;	//VK_FORMAT_R8G8B8A8_SRGB
			viewInfo.subresourceRange.aspectMask = aspectFlag;//VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = dynamicLightMapsCount; // 6

		// 2 - создание обертки		
		VkResult result = vkCreateImageView( g_GraphicsCore.logicalDevice, &viewInfo, nullptr, &cubeMapView);
		if( result != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create cube image view!");
		}
	}
	
	
	// image view для сэмплинга
	{
		VkImageViewCreateInfo cubeViewCI = {};
			cubeViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			cubeViewCI.image = cubeMap;               // тот же VkImage
			cubeViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE; // ← CUBE для сэмплинга
			cubeViewCI.format = VK_FORMAT_D32_SFLOAT;
			cubeViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			cubeViewCI.subresourceRange.baseMipLevel = 0;
			cubeViewCI.subresourceRange.levelCount = 1;
			cubeViewCI.subresourceRange.baseArrayLayer = 0;
			cubeViewCI.subresourceRange.layerCount = dynamicLightMapsCount;    // все 6 граней

		vkCreateImageView( g_GraphicsCore.logicalDevice, &cubeViewCI, nullptr, &cubeMapViewSampling);
	}
	
	// sampler для сэмплинга
	{
		// Для аппаратного PCF нужен sampler с compareOp:
		
		VkSamplerCreateInfo samplerCI = {};
			samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCI.compareEnable = VK_TRUE;                    // ← ключевой момент
			samplerCI.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;    // ← depth comparison
			samplerCI.mipLodBias = 0.0f;
			samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;// VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

		vkCreateSampler( g_GraphicsCore.logicalDevice, &samplerCI, nullptr, &cubeMapSampler);
	}
	
	// image view для сэмплинга и sampler будут подключаться к декриптор сету L-паса. ImageView - как VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
	
	
	
	// в рендерпасе для кубического маппинга нчиего особого.
	// Один attachment, без multi-attachment фокусов. Многослойность обеспечивается через framebuffer и gl_Layer.
	
	// 1 - Renderpass attachments
	if( HANDLE_NULL(renderPass_Shadow) )
	{
		// вообще один в один как для Spot!
		
		VkAttachmentDescription depthAttachment;
			depthAttachment.format = VK_FORMAT_D32_SFLOAT;//g_GraphicsCore.findDepthFormat();
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT; // 0
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;						// 1 => без мультисемплинга
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// заливать буфер одним цветом перед каждым кадром (назначать всем значениям некую константу)
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;					// результат отрисовки будет сохранен в память и может быть считан потом
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// плевать, где текстурное прикрепление было расположено ранее
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		// (автоматически) целевое расположение текстурного прикрепления - цепочка показа

		// 1.2 - references
		std::array<VkAttachmentReference, 1> depthAttachmentsRefs {};
			depthAttachmentsRefs[0].attachment = 0;
			depthAttachmentsRefs[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						
		// 2.2 - subpass
		VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;	// т.к. Vulkan также может подерживать compute subpasses, надо явно указать, что этот субпасс - графический!
			subpass.colorAttachmentCount = 0;//static_cast<uint32_t>(colorAttachmentsRefs.size());			// количество цветовых прикреплений
			subpass.pColorAttachments = nullptr;//colorAttachmentsRefs.data();				// ссылка на текстурное прикрепление цвета
			
			subpass.pDepthStencilAttachment = depthAttachmentsRefs.data();	//&depthAttachmentRef;
		// субпассы автоматически учитывают изменения в расположении изображений 
		// эти переходы контролируются зависимостями субпасса, которые определяют память и исполнительные зависимости между субпассами
		
		
		// 3 - dependencies
		VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	// VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render pass
			dependency.dstSubpass = 0;		// The index 0 refers to our subpass, which is the first and only one
			// The dstSubpass must always be higher than srcSubpass to prevent cycles
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;	// ожидающие операции. wait for the swap chain to finish reading from the image
			dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;	// этапы, на которых эти ожидающие операции появляются
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


		// 4 - renderpass creation
		VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;							//static_cast<uint32_t>(attachments.size());				// указание количества прикреплений
			renderPassInfo.pAttachments = &depthAttachment;				//attachments.data();										// привязка прикреплений к рендерпасу
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(g_GraphicsCore.logicalDevice, &renderPassInfo, nullptr, &renderPass_Shadow);

		if( result != VK_SUCCESS ) 
		{
			g_GraphicsCore.checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create omni Shadow render pass!");
		}
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow omni render pass OK")
			DEBUG_INFO
		#endif
	}
	
	// 2 - Framebuffers
	if( HANDLE_NULL(frameBuffer_Shadow) && HANDLE_VALID(renderPass_Shadow) && HANDLE_VALID(cubeMapView) )
	{		
		VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass_Shadow;	// с каким рендерпасом фреймбуфер должен быть совместим
			
			framebufferInfo.attachmentCount = 1;						//
			framebufferInfo.pAttachments = &cubeMapView;		// 2D_ARRAY view, все 6 слоёв
			
			framebufferInfo.width = shadowExtent.width;
			framebufferInfo.height = shadowExtent.height;
			framebufferInfo.layers = dynamicLightMapsCount;			// 1, потому что слои уже в view	// number of layers in image arrays. Our swap chain images are single images, so the number of layers is 1.

		if( vkCreateFramebuffer(g_GraphicsCore.logicalDevice, &framebufferInfo, nullptr, &frameBuffer_Shadow) != VK_SUCCESS )
		{
			throw std::runtime_error("failed to create Shadow Omni framebuffer!");
		}
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow omni framebuffer OK")
			DEBUG_INFO
		#endif	
	
	}
	
	// 4 - DescriptorSetLayout
	if( HANDLE_NULL(descriptorSetLayout_Shadow)  )
	{
		// схема разъема. описание точек привязки только ресурсов (буферов, изображений) к шейдерам		
		std::array<VkDescriptorSetLayoutBinding, 2> bindings = {};
		// layout(binding = 0) uniform FhShadowpassCubeEdgesInfo - project and view matrices
			bindings[0].binding = 0;												// индекс точки привязки: layout(binding = 0) uniform
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;			// тип - uniform buffer
			bindings[0].descriptorCount = 1;										// UBO-шек может быть много, но сейчас в массиве - только одна. This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example
			bindings[0].stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;					// определили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
			bindings[0].pImmutableSamplers = nullptr; 								// Optional. Для изображений (image sampling)	
		
		// layout(binding = 1) uniform FhDynamicLightSourceInfo  
			bindings[1].binding = 1;												
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;			// тип - uniform buffer
			bindings[1].descriptorCount = 1;										// UBO-шек может быть много, но сейчас в массиве - только одна. This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;					// определили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
			bindings[1].pImmutableSamplers = nullptr; 	
		
		
		// 3 - summarize all binding
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();
			
		// 4 - create VkDescriptorSetLayout
		VkResult result = vkCreateDescriptorSetLayout(g_GraphicsCore.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout_Shadow);
		if( result != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(result);
			throw std::runtime_error("Failed to create omni Shadow descriptor set layout!");
		}
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow omni DescriptorSetLayout OK")
			DEBUG_INFO
		#endif
	}
	
	
	// 5 - Descriptor Pool
	if( HANDLE_NULL(descriptorPool_Shadow)  )
	{
		// набор разъемов. создание общего перечня сетов.
		/*
			Descriptor pool — это объект, из которого Vulkan выделяет descriptor set-ы. Ты заранее указываешь, сколько дескрипторов каких типов тебе нужно, и pool использует это как резерв памяти/ресурсов для будущих set-ов.

			VkDescriptorPool представляет собой ресурс, используемый для управления набором дескрипторов. 
			Pool позволяет эффективно управлять созданием и удалением дескрипторных наборов (descriptor sets).
			VkDescriptorPool служит инструментом эффективного распределения и повторного использования ресурсов, позволяя избежать 
			затратных операций выделения памяти при каждом изменении состояния графического конвейера.

		*/
		
		
		//printVarComment("Descriptor pool size: ", MAX_ENTITIES_IN_ACTION)
		//DEBUG_INFO
		
		// 1 - Сначала нужно описать, какие типы дескрипторов содержатся в сетах дескрипторов, и их количество/
		// То есть какие патроны набивать в магазин, помещаемый в разгрузку (пул)
		// Есть пока всего 2 типа передаваемых в шейдер (конвейер) ресурсов - UBO и Image sampler, создадим массив с описанием обоих типов ресурсов
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
			// 1.0 - в движке будет (MAX_FRAMES_IN_FLIGHT * MAX_ENTITIES_IN_ACTION) UBO-шек FhGeneralInfo
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount =  MAX_ENTITIES_IN_ACTION;
			
			// 1.0 - в движке будет (MAX_FRAMES_IN_FLIGHT * MAX_ENTITIES_IN_ACTION) UBO-шек FhDynamicLightSourceInfo
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[1].descriptorCount =  MAX_ENTITIES_IN_ACTION;
			
		// 2 - зная все возможные дескрипторы, описываем общий пул для хранения дескрипторов
		VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
			poolInfo.pPoolSizes = poolSizes.data();
			int deferredFrames = 1; 			// при отложенном рендеринге буферизация одинарная, всего один кадр
			poolInfo.maxSets = deferredFrames * MAX_ENTITIES_IN_ACTION; //  но зато отрисовываемых сущностей - ого-го!

		// 3 - и выделяем непрерывный блок памяти (pool memory) достаточного размера
		VkResult result = vkCreateDescriptorPool(g_GraphicsCore.logicalDevice, &poolInfo, nullptr, &descriptorPool_Shadow);
		if( result != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(result);
			throw std::runtime_error("failed to create Shadow omni descriptor pool!");
		}
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow pool omni OK")
			DEBUG_INFO
		#endif   
	}
	
	createEdgeMatricesBuffer();
	updateEdgeMatricesBuffer();
	
	
	// 5.5 - dynamicLightOmniSet
	if(    HANDLE_NULL(dynamicLightOmniSet) 
		&& HANDLE_VALID(descriptorPool_Shadow) 
		&& HANDLE_VALID(descriptorSetLayout_Shadow) 
		&& HANDLE_VALID(g_GraphicsCore.LpassDynamicLightSourceInfoBuffer) 
		&& HANDLE_VALID(edgeMatricesBuffer) )
	{
		VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool_Shadow;				// указываем пул			
			allocInfo.descriptorSetCount = 1;										// и число сетов
			allocInfo.pSetLayouts = &descriptorSetLayout_Shadow; 			// указываем схему описания подключения ресурсов к конвейеру
	
		// 2 - вырезали кусок памяти из pool memory для конкретного descriptor set (взяли розетки со склада)
		VkResult result = vkAllocateDescriptorSets( g_GraphicsCore.logicalDevice, &allocInfo, &dynamicLightOmniSet );

		if( result != VK_SUCCESS) 
		{
			g_GraphicsCore.checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("Failed to allocate L-pass omni descriptor set!");
		}
		
		// 3 - связываем реальные ресурсы с дескрипторами
		// 3.1 информация о буфере матриц PVM		
		std::array<VkDescriptorBufferInfo, 2> bufferInfos{};
			// binding 0
			bufferInfos[0].buffer = edgeMatricesBuffer;	// ссылка на конкретный буфер UBO
			bufferInfos[0].offset = 0;
			bufferInfos[0].range = 1*sizeof(FhShadowpassCubeEdgesInfo);				// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
			
			// binding 1
			bufferInfos[1].buffer = g_GraphicsCore.LpassDynamicLightSourceInfoBuffer;	// ссылка на конкретный буфер на 16 структур об источниках UBO
			bufferInfos[1].offset = DynamicLight::index * sizeof(FhDynamicLightSourceInfo);// 512;// offset;// alignedOffset;//DynamicLight::index * sizeof(FhDynamicLightSourceInfo); // из 16 подряд структур сместимся до нужного куска
			bufferInfos[1].range = sizeof(FhDynamicLightSourceInfo);				// и от начала нужного куска нужна только одна структура
				
				
		// 4 - описание дескрипторов
		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			// 4.1 6 PV matrices buffer: struct FhShadowpassCubeEdgesInfo
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = dynamicLightOmniSet;
			descriptorWrites[0].dstBinding = 0;							// в геометрическом (!) шейдере: layout(binding = 0) uniform FhShadowpassCubeEdgesInfo
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfos[0];
			
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = dynamicLightOmniSet;
			descriptorWrites[1].dstBinding = 1;							// во фрагментном (!) шейдере: layout(binding = 1) uniform FhShadowpassCubeEdgesInfo
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &bufferInfos[1];

		// 5 - обновление содержимого
		vkUpdateDescriptorSets( g_GraphicsCore.logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		
		printMessage("\t Descriptor set of Shadow omni-pass updated!") 
	}
	
	// 6 - Graphics Pipeline Layout
	if( HANDLE_NULL(pipelineLayout_Shadow) && HANDLE_VALID(descriptorSetLayout_Shadow) )
	{
		// схема прибора. объединение VkDescriptorSetLayout + Push-Constants: общее описание точек привязки данных (ресурсов и переменных)
		// Даже если не намерены передавать в шейдер данные, все равно обязательно нужно создать один пустой VkPipelineLayout!
		// VkPipelineLayout надо обязательно прописать до создания самого конвейера!
		
		//setup push constants
		VkPushConstantRange push_constant;
			push_constant.offset = 0;									//this push constant range starts at the beginning
			push_constant.size = sizeof(FhShadowpassPushConstantsInfo);				//this push constant range takes up the size of a MeshPushConstants struct
			push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		//this push constant range is accessible only in the vertex shader
		
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			
			// конвейеру должны быть доступны:
			// 1 - описания точек привязки буферов и изображений
			pipelineLayoutInfo.setLayoutCount = 1;						
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_Shadow; 		// тут может быть массив, но пока только один
			
			// 2 - описания т.н. пуш-констант, которые являются другим способом передачи динамических значений в шейдер
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		
		VkResult result = vkCreatePipelineLayout(g_GraphicsCore.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout_Shadow);
		if( result != VK_SUCCESS ) 
		{
			g_GraphicsCore.checkResult(result);
			throw std::runtime_error("failed to create omni Shadow pipeline layout!");
		}
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow pipeline omni layout OK")
			DEBUG_INFO
		#endif
	}
	
	
	// 7 - Graphics Pipeline
	if( HANDLE_NULL(pipeline_Shadow) && HANDLE_VALID(renderPass_Shadow) && HANDLE_VALID(pipelineLayout_Shadow) )

	{
		// создание графического конвейера: этапы, шейдеры, смешивание + описание точек привязки для передачи данных в конвейер
		// 1 - программируемые этапы конвейера
		// 1.1 - загрузить байт-код
		auto vertShaderCode = g_GraphicsCore.readFile( FH_SHADER_PATH  "omni\\Shadow_pass_v_omni.spv");		// auto vertShaderCode - это статичный вектор символов-байтов
		auto geomShaderCode = g_GraphicsCore.readFile( FH_SHADER_PATH  "omni\\Shadow_pass_g_omni.spv");
		auto fragShaderCode = g_GraphicsCore.readFile( FH_SHADER_PATH  "omni\\Shadow_pass_f_omni.spv");

		// 1.2 - собрать из байт-кода шейдерный модуль ("обернуть" байт-код в Vulkan)
		VkShaderModule vertShaderModule = g_GraphicsCore.createShaderModule(vertShaderCode);
		VkShaderModule geomShaderModule = g_GraphicsCore.createShaderModule(geomShaderCode);
		VkShaderModule fragShaderModule = g_GraphicsCore.createShaderModule(fragShaderCode);
		
		//~ Shader modules are just a thin wrapper around the shader bytecode that we’ve
		//~ previously loaded from a file and the functions defined in it. The compilation
		//~ and linking of the SPIR-V bytecode to machine code for execution by the GPU
		//~ doesn’t happen until the graphics pipeline is created.
		
		// VkShaderModule располагается тут локально, так как не будет нужен после завершения настройки Pipeline,
		// поэтому в конце данной функции он вручную удаляется	

		// 1.3 - преобразуем шейдерные модули в шейдерные программы для этапов: вершинного и фрагментного
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// 18
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;								// 0x00000001
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";
			//vertShaderStageInfo.pSpecializationInfo = nullptr; // сюда можно указать заранее известные шейдерные константы, ради оптимизации

		VkPipelineShaderStageCreateInfo geomShaderStageInfo{};
			geomShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// 18
			geomShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;							//
			geomShaderStageInfo.module = geomShaderModule;
			geomShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// 18
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";
			//fragShaderStageInfo.pSpecializationInfo = nullptr;

		// 1.4 - в нижеприведенном массиве описываем последовательность графических этапов: сначала обработка вершин, потом - фрагментов
		// также сюда можно потом дополнительно добавлять геометрические, тесселяцию и т.д.
		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, geomShaderStageInfo, fragShaderStageInfo};

		// 2 - фиксированные этапы конвейера		
		// VkPipelineVertexInputStateCreateInfo describes the format of the vertex data that will be passed to the vertex shader
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};			
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;	// 19
			
		auto bindingDescription = Vertex2::getBindingDescription();
		auto attributeDescriptions = Vertex2::getAttributeDescriptions();
		
			vertexInputInfo.vertexBindingDescriptionCount = 1;						// количество привязок вершин
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; 		// настройка привязки
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>( attributeDescriptions.size() );		// количество атрибутов
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); 	// данные об атрибутах
			
			// в случае, если вершинные данные захардкожены, нечего настраивать
			//~ vertexInputInfo.vertexBindingDescriptionCount = 0;			// привязок нет. spacing between data and whether the data is per-vertex or per-instance
			//~ vertexInputInfo.pVertexBindingDescriptions = nullptr; 		// Optional
			//~ vertexInputInfo.vertexAttributeDescriptionCount = 0;		// атрибутов нет
			//~ vertexInputInfo.pVertexAttributeDescriptions = nullptr; 	// Optional
			
			
		// 2.2 - Input assembly stage		
		// VkPipelineInputAssemblyStateCreateInfo describes two things: what kind of geometry will be drawn from the vertices (point, line, triangle etc)
		// and if primitive restart should be enabled.
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;	// 20
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		//рисовать треугольники
			inputAssembly.primitiveRestartEnable = VK_FALSE;
		// используя Elements Buffer, можно переиспользовать вершины при отрисовке. Это плюс к оптимизации.


		// 2.3 - Viewport and Scissor
		// Вьюпорт описывает область фреймбуфера, куда выход будет рендерить. Почти всегда размер вьюпорта совпадает с размерами окна.
		// viewports define the transformation from the image to the framebuffer
		// влияет на сжатие/растяжение изображения на экране
		
		// Ножничные прямоугольники определяют, в каком регионе по-настоящему будут размещены пиксели. (это связано с перемещением окна по экрану?)
		// Любые пиксели вне этого прямоугольника будут отброшены растеризатором.
		// Т.е. ножницы работают скорее как фильтр, а не преобразователь
		
		// Viewport и scissors можно задать либо как статическую часть графического конвейера либо как динамическое состояние, задаваемое через command buffer

		// 2.3.1 - ДИНАМИЧЕСКИ ИЗМЕНЯЕМОЕ СОСТОЯНИЕ
		// при динамическом состоянии нужно явно указать в dynamicStates, какие именно параметры (а их не слишком много) будут динамически изменяемы (без пересоздания конвейера)
		// в большинстве случаем это более удобный/предпочитаемый вариант - дает больше гибкости
		// Нужно активировать соответствующие состояния для конвейера...
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		
		VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
			dynamicState.pDynamicStates = dynamicStates.data();
			
		// ... и прописать их количество в viewportState ...
		VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;	// 22
			viewportState.viewportCount = dynamicLightMapsCount; // 6
			viewportState.scissorCount = dynamicLightMapsCount;	 //6
			viewportState.flags = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT_EXT; // для Geometry-шейдера
		// ... а реальные вьюпорт и ножницы затем можно будет указывать во время отрисовки (через command buffer)
	
					
		// 2.4 - растеризатор
		// performs depth testing, face culling and the scissor test, and it can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering).
		VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;	// если true, то это отключает вывод во фреймбуфер
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // заполнение полигона (еще есть линии и точки)
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //VK_FRONT_FACE_CLOCKWISE; // порядок следования вершин полигона для понимания, передний или задний он
			
			rasterizer.depthBiasEnable = VK_FALSE;

		// 2.5 - мультисемплер
		// способ сглаживать изображение. Требуется подключить фичу GPU
		VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


		VkPipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;		// тест глубины разрешен
			depthStencil.depthWriteEnable = VK_TRUE;	// запись в буфер глубины разрешена
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL ;//VK_COMPARE_OP_LESS;	// предпочитаем МЕНЬШИЕ значения
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;	 // make sure that the format of the depth/stencil image contains a stencil component.


		// 2.6 - смешивание цветов
		// Когда фрагментный шейдер рассчитал цвет пикселя, надо правильно его совместить с цветом из буфера - смешать или скомбинировать.
		// конфигурация привязанного фреймбуфера:
		//~ VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			//~ colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			//~ colorBlendAttachment.blendEnable = VK_FALSE; // не совмещать (новый цвет заменяет старый из буфера)
		//~ std::array<VkPipelineColorBlendAttachmentState, 1> colorBlendAttachments = {colorBlendAttachment};

		// глобальные настройки совмещения цветов (между буферами?):
		VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 0;//static_cast<uint32_t>( colorBlendAttachments.size() );	//1;
			colorBlending.pAttachments = nullptr;//colorBlendAttachments.data();//&colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f;
			colorBlending.blendConstants[1] = 0.0f;
			colorBlending.blendConstants[2] = 0.0f;
			colorBlending.blendConstants[3] = 0.0f;


		// pipeline layout
		// 2.8 - объединим всю информацию для создания дескриптора графического конвейера
		VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;		//28
			pipelineInfo.stageCount = 3; // vert, geom, frag
			pipelineInfo.pStages = shaderStages;				// массив описаний этапов рендера
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			// Then we reference all of the structures describing the fixed-function stage.
			pipelineInfo.layout = pipelineLayout_Shadow;
			pipelineInfo.renderPass = renderPass_Shadow;			// createRenderPass() должна быть вызвана ДО этого момента!
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			
			pipelineInfo.pDepthStencilState = &depthStencil;

		VkResult result2 = vkCreateGraphicsPipelines(g_GraphicsCore.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_Shadow);
		if ( result2!= VK_SUCCESS)
		{
			g_GraphicsCore.checkResult(result2);
			throw std::runtime_error("failed to create omni Shadow graphics pipeline!");
		}

		// после создания конвейера отдельные шейдеры (вершинный/фрагментный) уже не нужны
		vkDestroyShaderModule(g_GraphicsCore.logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(g_GraphicsCore.logicalDevice, geomShaderModule, nullptr);
		vkDestroyShaderModule(g_GraphicsCore.logicalDevice, vertShaderModule, nullptr);
		
		#ifdef DYNAMICLIGHT_DEBUG
			printMessage("Shadow Pipeline OK")
			DEBUG_INFO
		#endif
	}
	
	
	
	status = true;
	
	#ifdef DYNAMICLIGHT_DEBUG
		printMessage("Shadowpass omni created\n\n")
	#endif
	
}


void DynamicLight::destroyShadowpassOmni()
{
	if( !status ) return;
	
	
	destroyEdgeMatricesBuffer();
	
	
	
	vkDestroyPipeline(g_GraphicsCore.logicalDevice, pipeline_Shadow, nullptr);

	vkDestroyPipelineLayout(g_GraphicsCore.logicalDevice, pipelineLayout_Shadow, nullptr);  
	
	vkDestroyDescriptorPool(g_GraphicsCore.logicalDevice, descriptorPool_Shadow, nullptr);
	
	vkDestroyDescriptorSetLayout(g_GraphicsCore.logicalDevice, descriptorSetLayout_Shadow, nullptr);
	
	if(frameBuffer_Shadow != VK_NULL_HANDLE) vkDestroyFramebuffer(g_GraphicsCore.logicalDevice, frameBuffer_Shadow, nullptr);
	
	vkDestroyImageView(g_GraphicsCore.logicalDevice, 	cubeMapViewSampling, 	nullptr);

	vkDestroySampler( g_GraphicsCore.logicalDevice,	cubeMapSampler, nullptr);
		
	vkDestroyImageView(g_GraphicsCore.logicalDevice, 	cubeMapView, 	nullptr);
	vkFreeMemory(g_GraphicsCore.logicalDevice, 		cubeMapMemory, nullptr);
	vkDestroyImage(g_GraphicsCore.logicalDevice, 		cubeMap, 		nullptr);
		
	vkDestroyRenderPass(g_GraphicsCore.logicalDevice, renderPass_Shadow, nullptr);
}





void DynamicLight::createEdgeMatricesBuffer()
{
	if( HANDLE_NULL(edgeMatricesBuffer) && HANDLE_NULL(edgeMatricesBufferMemory) && !CHECK_PTR(edgeMatricesBufferMapped) )
	{	
		// создание в видеопамяти буфера для шейдерных переменных и "прокидывание" прямого доступа к нему
		// 1 - размер буфера загружаемых в шейдер (ВЫРОВНЕННЫХ!) данных
		VkDeviceSize bufferSize = 1 * sizeof(FhShadowpassCubeEdgesInfo);			// размер буфера = 4*(4*4)*4 = 256 байта
		
		// 3.1 - создали host-side буфер и выделили ему ВИДИМУЮ! видеопамять
		g_GraphicsCore.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
										edgeMatricesBuffer, 
										edgeMatricesBufferMemory );

		// 3.2 - сделали прямую ссылку на эту ВИДИМУЮ память и сохранили ее в вектор uniformBuffersMemory
		vkMapMemory(g_GraphicsCore.logicalDevice, edgeMatricesBufferMemory, 0, bufferSize, 0, &edgeMatricesBufferMapped );
	}
}
	
void DynamicLight::destroyEdgeMatricesBuffer()
{
	vkDestroyBuffer(g_GraphicsCore.logicalDevice, edgeMatricesBuffer, nullptr);
	vkFreeMemory(g_GraphicsCore.logicalDevice,  edgeMatricesBufferMemory, nullptr);	
}

void DynamicLight::updateEdgeMatricesBuffer()
{
	if( DynamicLight::type == FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL) // !CHECK_PTR(edgeMatricesBufferMapped) )	
	{
		FhShadowpassCubeEdgesInfo matricesPV;
		
		glm::vec3 lightPos = getActorPosition();
		
		float ratio = 1.0f;//(float)shadowExtent.width / (float)shadowExtent.height;
		
		glm::mat4 proj = glm::perspective( glm::radians(sector), ratio, fLightViewNear, radius );
		// ВАЖНО: proj[1][1] должен быть положительным для Vulkan!
		// glm::perspective по умолчанию даёт отрицательный [1][1] (OpenGL-конвенция)
		//proj[1][1] *= -1;

		//~ float s = 10.0f; // shadow frustum size, подбери под сцену
		//~ glm::mat4 proj = glm::ortho(-s/2.0f, s/2.0f, -s/2.0f, s/2.0f, fLightViewNear, radius);

		glm::mat4 views[6];
			views[0] = glm::lookAt(lightPos, lightPos + glm::vec3( 1, 0, 0), glm::vec3(0, -1, 0));
			views[1] = glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
			views[2] = glm::lookAt(lightPos, lightPos + glm::vec3(0,  1, 0), glm::vec3(0, 0,  1));
			views[3] = glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
			views[4] = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0,  1), glm::vec3(0, -1, 0));
			views[5] = glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));

		// Комбинируем: PV[face] = proj * views[face]
		for (int i = 0; i < 6; i++) 
		{
			matricesPV.PV[i] = proj * views[i];
		}
		

		//~ printMessage("\n\nupdateEdgeMatricesBuffer()")
		//~ printVec3(lightPos)
		//~ for (int i = 0; i < 6; i++) 
		//~ {
			//~ glm::mat4 view = glm::lookAt(lightPos, lightPos + dirs[i], ups[i]);
			//~ matricesPV.PV[i] = proj * view;
			
			//~ printVec3(dirs[i])
			//~ printVec3(ups[i])
			
			//~ printMat4("\nview ", view )
			//~ printMat4("\nproj ", proj )
			
			//~ printMat4("\nPV ", matricesPV.PV[i] )
		//~ }

		if( CHECK_PTR(edgeMatricesBufferMapped) )
		{	
			memcpy( edgeMatricesBufferMapped, &matricesPV, 1*sizeof(FhShadowpassCubeEdgesInfo));
		}
	}
}




void DynamicLight::update()
{
	#ifdef DYNAMICLIGHT_DEBUG
		printMessage("Light update edges")
	#endif
	
	updateEdgeMatricesBuffer();
	
	#ifdef DYNAMICLIGHT_DEBUG
		printMessage("Light edges ok")
	#endif

	
	if(MicroWorldName == "LabSpot")
	{
		if(g_Key[GLFW_KEY_J]) addActorWorldOffset( glm::vec3(0, 0.1, 0) );
	
		if(g_Key[GLFW_KEY_L]) addActorWorldOffset( glm::vec3(0, -0.1, 0) );
		
		if(g_Key[GLFW_KEY_I]) addActorWorldOffset( glm::vec3(0.1, 0.0, 0) );
		
		if(g_Key[GLFW_KEY_K]) addActorWorldOffset( glm::vec3(-0.1, 0.0, 0) );
	}
	
	
}


