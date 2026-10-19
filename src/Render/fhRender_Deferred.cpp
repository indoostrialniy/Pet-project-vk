#include "fhRender.h"

#include "fhEnumerations.h"	// G_PASS_ATTACHMENTS

#include "macros.h"

#include "Camera.h"

#include "Laboratory.h"
#include "DynamicLightSource.h"

//~ #include <set>
//~ #include <algorithm>


void Graphics::createGpass()
{
	// 1 - Renderpass attachments
	{
		if( !HANDLE_NULL(renderPass_G) ) {DEBUG_INFO return;}
		
		std::array<VkAttachmentDescription, gPassAttachmentsCount> attachments = {};
		for(int i=0; i<gPassAttachmentsCount; i++)
		{
			attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;						// 1 => без мультисемплинга
			// loadOp и storeOp относятся к цвету и глубине 
			// мы знаем, что координаты модели после всех матричных умножений преобразуются в вектор, у которого X и Y это экранные координаты,
			// а компонента Z - экранная глубина пикселя
			attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// заливать буфер одним цветом перед каждым кадром (назначать всем значениям некую константу)
			attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;					// результат отрисовки будет сохранен в память и может быть считан потом
			// пока что нам плевать на буфер трафарета, поэтому DONT_CARE
			attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			// текстуры и фреймбуферы  представляются как VkImage с определенным форматом пикселей, однако расположение пикселей в памяти может меняться
			// в зависимости от того, что мы пытаемся сделать с изображением
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// плевать, где текстурное прикрепление было расположено ранее
			// VK_IMAGE_LAYOUT_UNDEFINED не гарантирует, что данные из изображения будут сохранены, но мы все равно собирались делать заливку VK_ATTACHMENT_LOAD_OP_CLEAR
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		// (автоматически) целевое расположение текстурного прикрепления - цепочка показа
			// изображение должно быть перенесено в специальное расположение, доступное той функции/операции, которая будет обрабатывать это изображение далее
		}
		
		// color group
		attachments[G_PASS_COLOR_ATTACHMENT].format 				= VK_FORMAT_R16G16B16A16_SFLOAT; // HDR
		attachments[G_PASS_NORMAL_ATTACHMENT].format 				= VK_FORMAT_R16G16B16A16_SFLOAT;//VK_FORMAT_R8G8B8A8_SNORM; // -1...1
		attachments[G_PASS_POSITION_ATTACHMENT].format 				= VK_FORMAT_R32G32B32A32_SFLOAT;
		attachments[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT].format 	= VK_FORMAT_R16G16B16A16_SFLOAT; // HDR
		attachments[G_PASS_EMISSION_ATTACHMENT].format 				= VK_FORMAT_R16G16B16A16_SFLOAT; // HDR

		// depth group
		attachments[G_PASS_DEPTH_ATTACHMENT].format = findDepthFormat();		// D32_SFLOAT
		attachments[G_PASS_DEPTH_ATTACHMENT].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		
		// 1.1 - субпасс
		// рендерпасс может состоять из множества субпассов. Субпассы это операции рендеринга, зависящие от содержимого фреймбуфера от предыдущего рендерпасса
		// (к примеру, последовательность пост-процессинговых эффектов, применяющихся друг за другом)
		// Если сгруппировать эти операции рендеринга в едииный рендерпасс, то Vulkan может их реорганизовать и выжать больше производительности .
		// Для тестового треугольника оставим одиночный субпасс
		// каждый субпасс ссылается на 1 или несколько прикреплений, описанных ранее. Эти ссылки и есть структуры VkAttachmentReference
		
		// 1.2 - references
		// Объект VkAttachmentReference обращается к прикреплениям с помощью индексов массивов
		std::array<VkAttachmentReference, gPassAttachmentsCount-1> colorAttachmentsRefs {};
			colorAttachmentsRefs[G_PASS_COLOR_ATTACHMENT].attachment = G_PASS_COLOR_ATTACHMENT; 			// По этому индексу (colorAttachmentRef.attachment = 0) фрагментный шейдер директивой layout(location = 0) out vec4 outColor напрямую обращается к текстурному прикреплению (буферу).
			colorAttachmentsRefs[G_PASS_COLOR_ATTACHMENT].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 	// We intend to use the attachment to function as a color buffer. Vulkan автоматически перенесет текстурное прикрепление в указанное расположение, когда субпасс стартует.

			colorAttachmentsRefs[G_PASS_NORMAL_ATTACHMENT].attachment = G_PASS_NORMAL_ATTACHMENT; 			// По этому индексу (colorAttachmentRef.attachment = 1) фрагментный шейдер директивой layout(location = 1) out vec4 outColor напрямую обращается к текстурному прикреплению (буферу).
			colorAttachmentsRefs[G_PASS_NORMAL_ATTACHMENT].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 	// We intend to use the attachment to function as a color buffer. Vulkan автоматически перенесет текстурное прикрепление в указанное расположение, когда субпасс стартует.

			colorAttachmentsRefs[G_PASS_POSITION_ATTACHMENT].attachment = G_PASS_POSITION_ATTACHMENT; 			// По этому индексу (colorAttachmentRef.attachment = 2) фрагментный шейдер директивой layout(location = 2) out vec4 outColor напрямую обращается к текстурному прикреплению (буферу).
			colorAttachmentsRefs[G_PASS_POSITION_ATTACHMENT].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 	// We intend to use the attachment to function as a color buffer. Vulkan автоматически перенесет текстурное прикрепление в указанное расположение, когда субпасс стартует.
		
			colorAttachmentsRefs[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT].attachment = G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT; 			// По этому индексу (colorAttachmentRef.attachment = 2) фрагментный шейдер директивой layout(location = 2) out vec4 outColor напрямую обращается к текстурному прикреплению (буферу).
			colorAttachmentsRefs[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 	// We intend to use the attachment to function as a color buffer. Vulkan автоматически перенесет текстурное прикрепление в указанное расположение, когда субпасс стартует.
		
			colorAttachmentsRefs[G_PASS_EMISSION_ATTACHMENT].attachment = G_PASS_EMISSION_ATTACHMENT; 			// По этому индексу (colorAttachmentRef.attachment = 2) фрагментный шейдер директивой layout(location = 2) out vec4 outColor напрямую обращается к текстурному прикреплению (буферу).
			colorAttachmentsRefs[G_PASS_EMISSION_ATTACHMENT].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 	// We intend to use the attachment to function as a color buffer. Vulkan автоматически перенесет текстурное прикрепление в указанное расположение, когда субпасс стартует.
		
		std::array<VkAttachmentReference, 1> depthAttachmentsRefs {};
			depthAttachmentsRefs[0].attachment = G_PASS_DEPTH_ATTACHMENT;
			depthAttachmentsRefs[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			
		// а можно и в один массив все засунуть с pColorAttachments указывать весь colorAttachmentsRefs.data() но число минус 1, а в depth класть последнюю ячейку из него
			
			
		// 2.2 - subpass
		VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;	// т.к. Vulkan также может подерживать compute subpasses, надо явно указать, что этот субпасс - графический!
			
			subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentsRefs.size());			// количество цветовых прикреплений
			subpass.pColorAttachments = colorAttachmentsRefs.data();				// ссылка на текстурное прикрепление цвета
			
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
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());				// указание количества прикреплений
			renderPassInfo.pAttachments = attachments.data();										// привязка прикреплений к рендерпасу
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

		

		VkResult result = vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass_G);

		if( result != VK_SUCCESS ) 
		{
			checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create G render pass!");
		}
	}
	
	
	// 2 - Images for this attachments
	{
		// add SAMPLER_BIT?
		// color
		createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					gPassImage[G_PASS_COLOR_ATTACHMENT], gPassImageMemory[G_PASS_COLOR_ATTACHMENT]);
		gPassImageView[G_PASS_COLOR_ATTACHMENT] = createImageView(gPassImage[G_PASS_COLOR_ATTACHMENT], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
		createTextureSampler( gPassImageSampler[G_PASS_COLOR_ATTACHMENT] );
		
		// normal
		createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					gPassImage[G_PASS_NORMAL_ATTACHMENT], gPassImageMemory[G_PASS_NORMAL_ATTACHMENT]);
		gPassImageView[G_PASS_NORMAL_ATTACHMENT] = createImageView(gPassImage[G_PASS_NORMAL_ATTACHMENT], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
		createTextureSampler( gPassImageSampler[G_PASS_NORMAL_ATTACHMENT] );
		
		// position
		createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					gPassImage[G_PASS_POSITION_ATTACHMENT], gPassImageMemory[G_PASS_POSITION_ATTACHMENT]);
		gPassImageView[G_PASS_POSITION_ATTACHMENT] = createImageView(gPassImage[G_PASS_POSITION_ATTACHMENT], VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
		createTextureSampler( gPassImageSampler[G_PASS_POSITION_ATTACHMENT] );
		
		// PBR
		createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					gPassImage[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT], gPassImageMemory[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT]);
		gPassImageView[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT] = createImageView(gPassImage[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
		createTextureSampler( gPassImageSampler[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT] );
		
		// Emission
		createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					gPassImage[G_PASS_EMISSION_ATTACHMENT], gPassImageMemory[G_PASS_EMISSION_ATTACHMENT]);
		gPassImageView[G_PASS_EMISSION_ATTACHMENT] = createImageView(gPassImage[G_PASS_EMISSION_ATTACHMENT], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
		createTextureSampler( gPassImageSampler[G_PASS_EMISSION_ATTACHMENT] );
		
		// depth
		VkFormat depthFormat = findDepthFormat();
		createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					gPassImage[G_PASS_DEPTH_ATTACHMENT], gPassImageMemory[G_PASS_DEPTH_ATTACHMENT]);
		gPassImageView[G_PASS_DEPTH_ATTACHMENT] = createImageView(gPassImage[G_PASS_DEPTH_ATTACHMENT], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		createTextureSampler( gPassImageSampler[G_PASS_DEPTH_ATTACHMENT] );
	}
	
	
	// 3 - Framebuffer
	{
		if( !HANDLE_NULL(frameBuffer_G) ) { DEBUG_INFO return;}
		
		// gPassImageView already contains all views for G pass!
		
		VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass_G;	// с каким рендерпасом фреймбуфер должен быть совместим
			framebufferInfo.attachmentCount = static_cast<uint32_t>(gPassImageView.size());
			framebufferInfo.pAttachments = gPassImageView.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;			// number of layers in image arrays. Our swap chain images are single images, so the number of layers is 1.

		if( vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &frameBuffer_G) != VK_SUCCESS )
		{
			throw std::runtime_error("failed to create G framebuffer!");
		}
		
	}
	

	// 4 - DescriptorSetLayout
	{
		if( !HANDLE_NULL(descriptorSetLayout_G) ) {DEBUG_INFO return;}
		
		// схема разъема. описание точек привязки только ресурсов (буферов, изображений) к шейдерам
		/*
			VkDescriptorSetLayout describes shader binding points - which types of resource to bind to which place
			For example: to vertex shader we connect un UBO - this will be "binding 0"
						 to fragment shader we connect texture sampler - this will be "binding 1"
		*/
		
		std::array<VkDescriptorSetLayoutBinding, 6> bindings = {};
			// binding 0 -  project and view matrices - layout(binding = 0) uniform FhGeneralInfo {} commonData
				bindings[0].binding = 0;												// индекс точки привязки: layout(binding = 0) uniform FhGeneralInfo из вершинного шейдера
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;			// тип - uniform buffer
				bindings[0].descriptorCount = 1;										// UBO-шек может быть много, но сейчас в массиве - только одна. This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example
				bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;					// определили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
				bindings[0].pImmutableSamplers = nullptr; 								// Optional. Для изображений (image sampling)
		
			// binding 1->1+MAX_MATERIALS_PER_ENTITY = material samplers[16]
				bindings[1].binding = 1;												// layout(binding = 1) uniform sampler2D material0_textures[16]; из фрагментного шейдера
				bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// тип - изображение
				bindings[1].descriptorCount = MAX_SAMPLERS_PER_MATERIAL;				// массив на 16 ячеек
				bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;	// обращение к текстурам ожидается из вершинного и фрагментного шейдера
				bindings[1].pImmutableSamplers = nullptr;
			
				bindings[2] = bindings[1];	// material_1. All the same except binding index
				bindings[2].binding = 2;
				
				bindings[3] = bindings[1];	// material_2
				bindings[3].binding = 3;
				
				bindings[4] = bindings[1];	// material_3
				bindings[4].binding = 4;
			
			// binding 5 - FhMaterialInfo structs
				bindings[5].binding = 5;											// индекс точки привязки: layout(binding = 5) uniform FhMaterialInfo {}  из фрагментного шейдера
				bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// тип - uniform buffer
				bindings[5].descriptorCount = 1;											// количество прикрепляемых объектов		// UBO-шек может быть много, но сейчас в массиве - только одна. This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example
				bindings[5].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;		// определили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
				bindings[5].pImmutableSamplers = nullptr; 
			
			//~ // binding 6 - lights???
				//~ bindings[6].binding = 6;													// индекс точки привязки: layout(binding = 0) uniform UniformBufferObject из вершинного шейдера
				//~ bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// тип - uniform buffer
				//~ bindings[6].descriptorCount = 1;											// количество прикрепляемых объектов		// UBO-шек может быть много, но сейчас в массиве - только одна. This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example
				//~ bindings[6].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;						// определлили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
				//~ bindings[6].pImmutableSamplers = nullptr; 									// Optional. Для изображений (image sampling)
				
		
		// 3 - summarize all binding
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();
			
		// 4 - create VkDescriptorSetLayout
		VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout_G);
		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			throw std::runtime_error("Failed to create G descriptor set layout!");
		}
	}
	
	
	// 5 - Descriptor Pool
	{
		// набор разъемов. создание общего перечня сетов.
		/*
			Descriptor pool — это объект, из которого Vulkan выделяет descriptor set-ы. Ты заранее указываешь, сколько дескрипторов каких типов тебе нужно, и pool использует это как резерв памяти/ресурсов для будущих set-ов.

			VkDescriptorPool представляет собой ресурс, используемый для управления набором дескрипторов. 
			Pool позволяет эффективно управлять созданием и удалением дескрипторных наборов (descriptor sets).
			VkDescriptorPool служит инструментом эффективного распределения и повторного использования ресурсов, позволяя избежать 
			затратных операций выделения памяти при каждом изменении состояния графического конвейера.

		*/
		
		
		printVarComment("Descriptor pool size: ", MAX_ENTITIES_IN_ACTION)
		DEBUG_INFO
		
		// 1 - Сначала нужно описать, какие типы дескрипторов содержатся в сетах дескрипторов, и их количество/
		// То есть какие патроны набивать в магазин, помещаемый в разгрузку (пул)
		// Есть пока всего 2 типа передаваемых в шейдер (конвейер) ресурсов - UBO и Image sampler, создадим массив с описанием обоих типов ресурсов
		std::array<VkDescriptorPoolSize, 6> poolSizes{};
			
			// 1.0 - в движке будет (MAX_FRAMES_IN_FLIGHT * MAX_ENTITIES_IN_ACTION) UBO-шек FhGeneralInfo
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount =  MAX_ENTITIES_IN_ACTION;
			
			// 1.1 - в движке будет descriptorCount сэмплеров текстур цвета
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = MAX_ENTITIES_IN_ACTION * MAX_MATERIALS_PER_ENTITY;	// в каждом шейдере - массив на 16 ячеек текстур
			
			poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[2].descriptorCount = MAX_ENTITIES_IN_ACTION * MAX_MATERIALS_PER_ENTITY; // XXX в каждом шейдере - массив на 4 ячейки нормалей
			
			poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[3].descriptorCount = MAX_ENTITIES_IN_ACTION * MAX_MATERIALS_PER_ENTITY; // XXX в каждом шейдере - массив на 4 ячейки нормалей
			
			poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[4].descriptorCount = MAX_ENTITIES_IN_ACTION * MAX_MATERIALS_PER_ENTITY; // XXX в каждом шейдере - массив на 4 ячейки нормалей
			
			// 1.5 - в движке будет descriptorCount наборов матриц материалов
			poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[5].descriptorCount = MAX_ENTITIES_IN_ACTION;
			
			//~ // 1.6 - в движке будет descriptorCount наборов матриц света
			//~ poolSizes[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			//~ poolSizes[6].descriptorCount = MAX_ENTITIES_IN_ACTION;
			

		// 2 - зная все возможные дескрипторы, описываем общий пул для хранения дескрипторов
		VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
			poolInfo.pPoolSizes = poolSizes.data();
			int deferredFrames = 1; // при отложенном рендеринге буферизация одинарная, всего один кадр
			poolInfo.maxSets = deferredFrames * MAX_ENTITIES_IN_ACTION; //  но зато отрисовываемых сущностей - ого-го!

		// 3 - и выделяем непрерывный блок памяти (pool memory) достаточного размера
		VkResult result = vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool_G);
		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			throw std::runtime_error("failed to create П descriptor pool!");
		}   
	}
	
	
	// 6 - Graphics Pipeline Layout
	{
		// схема прибора. объединение VkDescriptorSetLayout + Push-Constants: общее описание точек привязки данных (ресурсов и переменных)
		// Даже если не намерены передавать в шейдер данные, все равно обязательно нужно создать один пустой VkPipelineLayout!
		// VkPipelineLayout надо обязательно прописать до создания самого конвейера!
		
		//setup push constants
		VkPushConstantRange push_constant;
			push_constant.offset = 0;									//this push constant range starts at the beginning
			push_constant.size = sizeof(FhGpassPushConstantsInfo);				//this push constant range takes up the size of a MeshPushConstants struct
			push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		//this push constant range is accessible only in the vertex shader
		
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			
			// конвейеру должны быть доступны:
			// 1 - описания точек привязки буферов и изображений
			pipelineLayoutInfo.setLayoutCount = 1;						
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_G; 		// тут может быть массив, но пока только один
			
			// 2 - описания т.н. пуш-констант, которые являются другим способом передачи динамических значений в шейдер
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		
		VkResult result = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout_G);
		if( result != VK_SUCCESS ) 
		{
			checkResult(result);
			throw std::runtime_error("failed to create G pipeline layout!");
		}
	}
	
	
	// 7 - Graphics Pipeline
	{
		// создание графического конвейера: этапы, шейдеры, смешивание + описание точек привязки для передачи данных в конвейер
		// 1 - программируемые этапы конвейера
		// 1.1 - загрузить байт-код
		auto vertShaderCode = readFile( FH_SHADER_PATH  "G_pass_v.spv");		// auto vertShaderCode - это статичный вектор символов-байтов
		auto fragShaderCode = readFile( FH_SHADER_PATH  "G_pass_f.spv");

		// 1.2 - собрать из байт-кода шейдерный модуль ("обернуть" байт-код в Vulkan)
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
		
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
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;	// предпочитаем МЕНЬШИЕ значения
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;	 // make sure that the format of the depth/stencil image contains a stencil component.


		// 2.6 - смешивание цветов
		// Когда фрагментный шейдер рассчитал цвет пикселя, надо правильно его совместить с цветом из буфера - смешать или скомбинировать.
		// конфигурация привязанного фреймбуфера:
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_TRUE;//VK_FALSE; // не совмещать (новый цвет заменяет старый из буфера)

			// RGB: newColor * srcAlpha + oldColor * (1 - srcAlpha)
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

			// Alpha: обычно просто копируем альфа из фрагмента (или тоже складываем)
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		std::array<VkPipelineColorBlendAttachmentState, 3> colorBlendAttachments = {colorBlendAttachment,colorBlendAttachment,colorBlendAttachment};


		// глобальные настройки совмещения цветов (между буферами?):
		VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = static_cast<uint32_t>( colorBlendAttachments.size() );	//1;
			colorBlending.pAttachments = colorBlendAttachments.data();//&colorBlendAttachment;
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
			pipelineInfo.layout = pipelineLayout_G;
			pipelineInfo.renderPass = renderPass_G;			// createRenderPass() должна быть вызвана ДО этого момента!
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			
			pipelineInfo.pDepthStencilState = &depthStencil;

		VkResult result2 = vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_G);
		if ( result2!= VK_SUCCESS)
		{
			checkResult(result2);
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		// после создания конвейера отдельные шейдеры (вершинный/фрагментный) уже не нужны
		vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);

	}
	
	
	createGpassGeneralInfoBuffer();
	
}
   
   
void Graphics::destroyGpass()
{
	// in reverse order
	
	destroyGpassGeneralInfoBuffer();
	
	vkDestroyPipeline(logicalDevice, pipeline_G, nullptr);

	vkDestroyPipelineLayout(logicalDevice, pipelineLayout_G, nullptr);  
	
	vkDestroyDescriptorPool(logicalDevice, descriptorPool_G, nullptr);
	
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout_G, nullptr);
	
	if(frameBuffer_G != VK_NULL_HANDLE) vkDestroyFramebuffer(logicalDevice, frameBuffer_G, nullptr);
	
	for(int i=0; i<gPassAttachmentsCount; i++)
	{			
		vkDestroySampler(logicalDevice, 	gPassImageSampler[i], nullptr);
		vkDestroyImageView(logicalDevice, 	gPassImageView[i], 	nullptr);
		vkFreeMemory(logicalDevice, 		gPassImageMemory[i], nullptr);
		vkDestroyImage(logicalDevice, 		gPassImage[i], 		nullptr);
	}
	
	vkDestroyRenderPass(logicalDevice, renderPass_G, nullptr);
}


void Graphics::createGpassGeneralInfoBuffer()
{
	// создание в видеопамяти буфера для шейдерных переменных и "прокидывание" прямого доступа к нему
		
	// 1 - размер буфера загружаемых в шейдер (ВЫРОВНЕННЫХ!) данных
	VkDeviceSize bufferSize = sizeof(FhGeneralInfo);			// размер буфера =  байта
				
	// 2 - аналогично как делали для буфера вершинных данных, только без vkUnmapMemory!
	// 2.1 - создали host-side буфер и выделили ему ВИДИМУЮ! видеопамять
	createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
									GpassGeneralInfoBuffer, 
									GpassGeneralInfoBufferMemory );

	// 2.2 - сделали прямую ссылку на эту ВИДИМУЮ память и сохранили ее в вектор uniformBuffersMemory
	vkMapMemory(logicalDevice, GpassGeneralInfoBufferMemory, 0, bufferSize, 0, &GpassGeneralInfoBufferMapped );

	// XXX буфер остается помеченым до конца жизни программы. Эта техника называется “persistent mapping” и работает на всех имплементациях Vulkan	
}


void Graphics::destroyGpassGeneralInfoBuffer()
{	
	vkDestroyBuffer(logicalDevice, GpassGeneralInfoBuffer, nullptr);
	vkFreeMemory(logicalDevice,  GpassGeneralInfoBufferMemory, nullptr);			
}

void Graphics::updateGpassGeneralInfoBuffer()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
	FhGeneralInfo ubo{};	
			
		glm::vec3 POV = CHECK_PTR(g_Camera) ? g_Camera->getActorPosition() : glm::vec3(-1.0f, 0.0f, 0.1f);
		glm::vec3 DOV = POV + (CHECK_PTR(g_Camera) ? g_Camera->getActorForwardVector() : glm::vec3(1.0f, 0.0f, 0.0f));//*Camera::rotation;	//glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 UP = glm::vec3(0.0f, 0.0f, 1.0f);
		
		//~ glm::vec3 POV = glm::vec3(7.35, -6.925, 4.958);
		//~ glm::vec3 DOV = glm::vec3(0.0f, 0.0f, 0.0f);//*Camera::rotation;	//glm::vec3(0.0f, 0.0f, 0.0f);
		//~ glm::vec3 UP = glm::vec3(0.0f, 0.0f, 1.0f);
		
	ubo.view = glm::lookAt(POV, DOV, UP);
		
	if(CHECK_PTR(g_Camera)) ubo.proj = glm::perspective(glm::radians(g_Camera->fCameraFOV), (float) swapChainExtent.width / (float) swapChainExtent.height, g_Camera->fCameraViewNear, g_Camera->fCameraViewDepth);
	ubo.proj[1][1] *= -1; // GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. 
	
	ubo.Time = time;
	
	ubo.cameraPos = CHECK_PTR(g_Camera) ? g_Camera->getActorPosition() : glm::vec3(14.087f, -14.19f, 7.33f);
	
	memcpy( GpassGeneralInfoBufferMapped, &ubo, sizeof(ubo)); // memcpy)dest, src, count
}





void Graphics::createFullscreenQuad()
{
	
	FhFullscreenQuadVertex vertex0{};
	
	float border = 1.0f;
	
	// first triangle
		vertex0.pos 			= {	border, -border };	// x y 
		vertex0.texCoord		= {	1, 0 };		//Vulkan invert the V axis
	fullscreenQuadVertexDataVector.push_back(vertex0); // правый верхний угол
		
		vertex0.pos 			= {	-border, border };	// x y 
		vertex0.texCoord		= {	0, 1 };		//Vulkan invert the V axis
	fullscreenQuadVertexDataVector.push_back(vertex0); // левый нижний угол
	
		vertex0.pos 			= {	border, border };	// x y 
		vertex0.texCoord		= {	1, 1 };		//Vulkan invert the V axis
	fullscreenQuadVertexDataVector.push_back(vertex0); // правый нижний угол
	
	// second triangle
		vertex0.pos 			= {	border, -border };	// x y 
		vertex0.texCoord		= {	1, 0 };		//Vulkan invert the V axis
	fullscreenQuadVertexDataVector.push_back(vertex0); // правый верхний угол
	
		vertex0.pos 			= {	-border, -border };	// x y 
		vertex0.texCoord		= {	0, 0 };		//Vulkan invert the V axis
	fullscreenQuadVertexDataVector.push_back(vertex0); // левый верхний угол
	
		vertex0.pos 			= {	-border, border };	// x y 
		vertex0.texCoord		= {	0, 1 };		//Vulkan invert the V axis
	fullscreenQuadVertexDataVector.push_back(vertex0); // левый нижний угол
	
	//~ std::cout << "\n";
	//~ printVec3(vertex.pos)
	//~ printVec3(vertex.normal)
	//~ printVec3(vertex.texCoord_mat)
	//printVariable(vertex.materialIndex)
		
	// make GPU-side buffer objects: meshVertexBuffer, meshVertexBufferMemory	

	// 1 - размер области памяти для хранения вершинных данных = размер поля для 1 вершины * число вершин
	VkDeviceSize bufferSize = sizeof(fullscreenQuadVertexDataVector[0]) * fullscreenQuadVertexDataVector.size(); // глобальная vertices хранит все вершинные данные треугольника
		
	// 2 - создаем промежуточный буфер, видимый хосту
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	// 2.1 - буфер источник (SRC_BIT)
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	
	// 2.2 - заносим данные в промежуточный буфер
	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);		// делаем кусок видеопамяти для промежуточного буфера видимым для хоста
		memcpy(data, fullscreenQuadVertexDataVector.data(), (size_t) bufferSize);							// записываем в него вершинные данные
	vkUnmapMemory(logicalDevice, stagingBufferMemory);								// "запечатываем" кусок обратно
		
	// 3 - создали целевой буфер (DST_BIT) на оптимальной памяти видеокарты, но не видимый хосту!
	// назначение буфера - хранить вершины (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fullscreenQuadVertexBuffer, fullscreenQuadVertexBufferMemory);
	
	
	// 4 - перемещаем данные о вершинах из промежуточного в целевой
	copyBuffer(stagingBuffer, fullscreenQuadVertexBuffer, bufferSize);
	
	//printMessage("dsdsvssdvsdvdslv dslv slvsl v")
	// 5 - удаляем промежуточный буфер и очищаем видеопамять
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
	
}


void Graphics::destroyFullscreenQuad()
{
	destroyVertexBuffer( fullscreenQuadVertexBuffer, fullscreenQuadVertexBufferMemory);	
}







void Graphics::createLpass()
{
	// 1 - Renderpass attachments
	{
		std::array<VkAttachmentDescription, lPassAttachmentsCount> attachments = {};
		for(int i=0; i<lPassAttachmentsCount; i++)
		{
			attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;						// 1 => без мультисемплинга
			// loadOp и storeOp относятся к цвету и глубине 
			// мы знаем, что координаты модели после всех матричных умножений преобразуются в вектор, у которого X и Y это экранные координаты,
			// а компонента Z - экранная глубина пикселя
			attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// заливать буфер одним цветом перед каждым кадром (назначать всем значениям некую константу)
			attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;					// результат отрисовки будет сохранен в память и может быть считан потом
			// пока что нам плевать на буфер трафарета, поэтому DONT_CARE
			attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			// текстуры и фреймбуферы  представляются как VkImage с определенным форматом пикселей, однако расположение пикселей в памяти может меняться
			// в зависимости от того, что мы пытаемся сделать с изображением
			attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// плевать, где текстурное прикрепление было расположено ранее
			// VK_IMAGE_LAYOUT_UNDEFINED не гарантирует, что данные из изображения будут сохранены, но мы все равно собирались делать заливку VK_ATTACHMENT_LOAD_OP_CLEAR
			attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;		// (автоматически) целевое расположение текстурного прикрепления - цепочка показа
			// изображение должно быть перенесено в специальное расположение, доступное той функции/операции, которая будет обрабатывать это изображение далее
		}
		
		// color group
		attachments[L_PASS_COLOR_ATTACHMENT].format = VK_FORMAT_R16G16B16A16_SFLOAT; // HDR
		attachments[L_PASS_BLOOM_ATTACHMENT].format = VK_FORMAT_R16G16B16A16_SFLOAT; //
		
		
		// 1.1 - субпасс
		// рендерпасс может состоять из множества субпассов. Субпассы это операции рендеринга, зависящие от содержимого фреймбуфера от предыдущего рендерпасса
		// (к примеру, последовательность пост-процессинговых эффектов, применяющихся друг за другом)
		// Если сгруппировать эти операции рендеринга в едииный рендерпасс, то Vulkan может их реорганизовать и выжать больше производительности .
		// Для тестового треугольника оставим одиночный субпасс
		// каждый субпасс ссылается на 1 или несколько прикреплений, описанных ранее. Эти ссылки и есть структуры VkAttachmentReference
		
		// 1.2 - references
		// Объект VkAttachmentReference обращается к прикреплениям с помощью индексов массивов
		std::array<VkAttachmentReference, lPassAttachmentsCount> colorAttachmentsRefs {};
			colorAttachmentsRefs[L_PASS_COLOR_ATTACHMENT].attachment = L_PASS_COLOR_ATTACHMENT; 			// По этому индексу (colorAttachmentRef.attachment = 0) фрагментный шейдер директивой layout(location = 0) out vec4 outColor напрямую обращается к текстурному прикреплению (буферу).
			colorAttachmentsRefs[L_PASS_COLOR_ATTACHMENT].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 	// We intend to use the attachment to function as a color buffer. Vulkan автоматически перенесет текстурное прикрепление в указанное расположение, когда субпасс стартует.

			colorAttachmentsRefs[L_PASS_BLOOM_ATTACHMENT].attachment = L_PASS_BLOOM_ATTACHMENT; 			// По этому индексу (colorAttachmentRef.attachment = 1) фрагментный шейдер директивой layout(location = 1) out vec4 outColor напрямую обращается к текстурному прикреплению (буферу).
			colorAttachmentsRefs[L_PASS_BLOOM_ATTACHMENT].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 	// We intend to use the attachment to function as a color buffer. Vulkan автоматически перенесет текстурное прикрепление в указанное расположение, когда субпасс стартует.			
			
		// 2.2 - subpass
		VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;	// т.к. Vulkan также может подерживать compute subpasses, надо явно указать, что этот субпасс - графический!
			
			subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentsRefs.size());			// количество цветовых прикреплений
			subpass.pColorAttachments = colorAttachmentsRefs.data();				// ссылка на текстурное прикрепление цвета
			
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
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());				// указание количества прикреплений
			renderPassInfo.pAttachments = attachments.data();										// привязка прикреплений к рендерпасу
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass_L);

		if( result != VK_SUCCESS ) 
		{
			checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create L render pass!");
		}
	}
	
	
	// 2 - Images for this attachments
	{
		// color
		createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					lPassImage[L_PASS_COLOR_ATTACHMENT], lPassImageMemory[L_PASS_COLOR_ATTACHMENT]);
		lPassImageView[L_PASS_COLOR_ATTACHMENT] = createImageView(lPassImage[L_PASS_COLOR_ATTACHMENT], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
		createTextureSampler( lPassImageSampler[L_PASS_COLOR_ATTACHMENT] );
		
		// normal
		createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
					lPassImage[L_PASS_BLOOM_ATTACHMENT], lPassImageMemory[L_PASS_BLOOM_ATTACHMENT]);
		lPassImageView[L_PASS_BLOOM_ATTACHMENT] = createImageView(lPassImage[L_PASS_BLOOM_ATTACHMENT], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
		createTextureSampler( lPassImageSampler[L_PASS_BLOOM_ATTACHMENT] );
		
	}
	
	
	// 3 - Framebuffer
	{
		// lPassImageView already contains all views for L pass!
		
		VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass_L;	// с каким рендерпасом фреймбуфер должен быть совместим
			framebufferInfo.attachmentCount = static_cast<uint32_t>(lPassImageView.size());
			framebufferInfo.pAttachments = lPassImageView.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;			// number of layers in image arrays. Our swap chain images are single images, so the number of layers is 1.

		if( vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &frameBuffer_L) != VK_SUCCESS )
		{
			throw std::runtime_error("failed to create L framebuffer!");
		}
		
	}
	

	// 4 - DescriptorSetLayout
	{
		// схема разъема. описание точек привязки только ресурсов (буферов, изображений) к шейдерам
		/*
			VkDescriptorSetLayout describes shader binding points - which types of resource to bind to which place
			For example: to vertex shader we connect un UBO - this will be "binding 0"
						 to fragment shader we connect texture sampler - this will be "binding 1"
		*/
		
		std::array<VkDescriptorSetLayoutBinding, 5> bindings = {};
			// binding 0->gPassAttachmentsCount - g-pass samplers[3-5]
				bindings[0].binding = 0;												// layout(binding = 1) uniform sampler2D material0_textures[16]; из фрагментного шейдера
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// тип - изображение
				bindings[0].descriptorCount = gPassAttachmentsCount-1;					// все из G-паса, кроме глубины
				bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;					// обращение к текстурам ожидается из фрагментного шейдера
				bindings[0].pImmutableSamplers = nullptr;
			
			// binding 1 - illumination infos matrices
				bindings[1].binding = 1;													// индекс точки привязки: layout(binding = 1) uniform {}  из  шейдера
				bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// тип - uniform buffer
				bindings[1].descriptorCount = 1;											// количество прикрепляемых объектов		
				bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;						// определили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
				bindings[1].pImmutableSamplers = nullptr; 
				
			// binding 2 - GpassGeneralInfoBuffer - FhGeneralInfo same as in vertex stage of G-pass
				bindings[2].binding = 2;													// индекс точки привязки: layout(binding = 2) uniform FhGeneralInfo {}  из вершинного шейдера
				bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// тип - uniform buffer
				bindings[2].descriptorCount = 1;											// количество прикрепляемых объектов		
				bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;						// определили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
				bindings[2].pImmutableSamplers = nullptr; 
			
			// binding 3-> - shadowmapSpots samplers[16]
				bindings[3].binding = 3;												// layout(binding = 3) uniform sampler2D shadowMapsSpots[16] из фрагментного шейдера
				bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// тип - изображение
				bindings[3].descriptorCount = MAX_DYNAMIC_LIGHT_SOURCES;				//
				bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;					// обращение к текстурам ожидается из фрагментного шейдера
				bindings[3].pImmutableSamplers = nullptr;
				
			// binding 4-> - shadowmapOmni samplers[16]
				bindings[4].binding = 4;												// layout(binding = 4) uniform sampler2D shadowMapsOmni[16] из фрагментного шейдера
				bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// тип - изображение
				bindings[4].descriptorCount = MAX_DYNAMIC_LIGHT_SOURCES;				//
				bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;					// обращение к текстурам ожидается из фрагментного шейдера
				bindings[4].pImmutableSamplers = nullptr;
		
		// 3 - summarize all binding
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();
			
		// 4 - create VkDescriptorSetLayout
		VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout_L);
		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			throw std::runtime_error("Failed to create L descriptor set layout!");
		}
	}
	
	
	// 5 - Descriptor Pool
	{
		// набор разъемов. создание общего перечня сетов.
		/*
			Descriptor pool — это объект, из которого Vulkan выделяет descriptor set-ы. Ты заранее указываешь, сколько дескрипторов каких типов тебе нужно, и pool использует это как резерв памяти/ресурсов для будущих set-ов.

			VkDescriptorPool представляет собой ресурс, используемый для управления набором дескрипторов. 
			Pool позволяет эффективно управлять созданием и удалением дескрипторных наборов (descriptor sets).
			VkDescriptorPool служит инструментом эффективного распределения и повторного использования ресурсов, позволяя избежать 
			затратных операций выделения памяти при каждом изменении состояния графического конвейера.

		*/
		
		// 1 - Сначала нужно описать, какие типы дескрипторов содержатся в сетах дескрипторов, и их количество/
		// То есть какие патроны набивать в магазин, помещаемый в разгрузку (пул)
		// Есть пока всего 2 типа передаваемых в шейдер (конвейер) ресурсов - UBO и Image sampler, создадим массив с описанием обоих типов ресурсов
		std::array<VkDescriptorPoolSize, 5> poolSizes{};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[0].descriptorCount =  gPassAttachmentsCount-1;	// в каждом шейдере - массив на 3-5 ячеек текстур с предыдущего G-паса
			
			// 1.0 - в движке будет 1 UBO для 16 illumination info structs
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[1].descriptorCount =  1;		
			
			// 1.2 - в движке будет 1 UBO для GpassGeneralInfoBuffer для общей информации
			poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[2].descriptorCount =  1;
			
			poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// spots light sources
			poolSizes[3].descriptorCount =  MAX_DYNAMIC_LIGHT_SOURCES;
			
			poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// omni light sources
			poolSizes[4].descriptorCount =  MAX_DYNAMIC_LIGHT_SOURCES;
			

		// 2 - зная все возможные дескрипторы, описываем общий пул для хранения дескрипторов
		VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() );
			poolInfo.pPoolSizes = poolSizes.data();
			int deferredFrames = 1; // при отложенном рендеринге буферизация одинарная, всего один кадр
			poolInfo.maxSets = deferredFrames * 1; //   полноэкранный квад тоже лишь один

		// 3 - и выделяем непрерывный блок памяти (pool memory) достаточного размера
		VkResult result = vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool_L);
		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			throw std::runtime_error("failed to create L descriptor pool!");
		}   
	}
	
	// LpassDynamicLightSourceInfoBuffer создавать строго ДО Descriptor Set!
	//~ createLpassDynamicLightSourceInfoBuffer();
	
	// 6 - Descriptor Set
	{
		VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool_L;				// указываем пул			
			allocInfo.descriptorSetCount = 1;										// и число сетов
			allocInfo.pSetLayouts = &descriptorSetLayout_L; 			// указываем схему описания подключения ресурсов к конвейеру
	
		// 2 - вырезали кусок памяти из pool memory для конкретного descriptor set (взяли розетки со склада)
		VkResult result = vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet_L );

		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("Failed to allocate L-pass descriptor set for fullscreen quad!");
		}
		
		// 3 - связываем реальные ресурсы с дескрипторами
		// 3.1 информация о буфере матриц PVM		
		std::array<VkDescriptorBufferInfo, 2> bufferInfos{};
			// binding 0
			bufferInfos[0].buffer = LpassDynamicLightSourceInfoBuffer;	// ссылка на конкретный буфер UBO
			bufferInfos[0].offset = 0;
			
			
			//~ // 1. Получаем лимит устройства
			//~ VkPhysicalDeviceProperties deviceProperties;				// info struct
			//~ vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
			
			//~ uint32_t alignment = deviceProperties.limits.minUniformBufferOffsetAlignment; // 64

			//~ // 2. Считаем реальный размер структуры
			//~ const size_t structSize = sizeof(FhDynamicLightSourceInfo); // 512

			//~ // 3. Вычисляем выровненный шаг (stride)
			//~ // Формула: (size + alignment - 1) & ~(alignment - 1)
			//~ const uint32_t stride = (structSize + alignment - 1) & ~(alignment - 1);
			//~ // Для 452 и 64: stride = 464? Нет! 452 -> 448? Нет. Давайте посчитаем:
			//~ // 452 / 64 = 7.0625. Округляем вверх: 8 * 64 = 512.
			//~ // stride = 512 байт.
			//~ // 1 - размер буфера загружаемых в шейдер (ВЫРОВНЕННЫХ!) данных			
			//~ VkDeviceSize bufferSize = MAX_DYNAMIC_LIGHT_SOURCES * stride;
			
			//~ printVariable(stride) // 512
			
			bufferInfos[0].range = VK_WHOLE_SIZE;// bufferSize;//MAX_DYNAMIC_LIGHT_SOURCES*sizeof(FhDynamicLightSourceInfo);				// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
			// MAX_DYNAMIC_LIGHT_SOURCES*sizeof(FhDynamicLightSourceInfo) = 16 * 464 = 7424, а должно быть 8192
			
		//~ printVariable(sizeof(FhDynamicLightSourceInfo)) // 464
		//~ printVariable(alignment) // 64
		//~ printVariable(stride) // 512
		//~ printVariable(bufferSize) // 8192
		
		
		
			// binding 2
			bufferInfos[1].buffer = GpassGeneralInfoBuffer;	// ссылка на конкретный буфер FhGeneralInfo
			bufferInfos[1].offset = 0;
			bufferInfos[1].range = sizeof(FhGeneralInfo);				// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
		
		
				
		// 3.2 - информация о текстурах
		// к шейдеру прикрепляются максимум 2-5 картинки: массив текстур на gPassAttachmentsCount-1 сэмплера с предыдущего прохода (G-pass), без глубины!
		std::array<VkDescriptorImageInfo, gPassAttachmentsCount-1> materials_textures {};			
			// color resouce
			materials_textures[G_PASS_COLOR_ATTACHMENT].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			materials_textures[G_PASS_COLOR_ATTACHMENT].imageView = gPassImageView[G_PASS_COLOR_ATTACHMENT];
			materials_textures[G_PASS_COLOR_ATTACHMENT].sampler = gPassImageSampler[G_PASS_COLOR_ATTACHMENT];
			
			// normal
			materials_textures[G_PASS_NORMAL_ATTACHMENT].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			materials_textures[G_PASS_NORMAL_ATTACHMENT].imageView = gPassImageView[G_PASS_NORMAL_ATTACHMENT];
			materials_textures[G_PASS_NORMAL_ATTACHMENT].sampler = gPassImageSampler[G_PASS_NORMAL_ATTACHMENT];
			
			// position
			materials_textures[G_PASS_POSITION_ATTACHMENT].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			materials_textures[G_PASS_POSITION_ATTACHMENT].imageView = gPassImageView[G_PASS_POSITION_ATTACHMENT];
			materials_textures[G_PASS_POSITION_ATTACHMENT].sampler = gPassImageSampler[G_PASS_POSITION_ATTACHMENT];
			
			// PBR
			materials_textures[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			materials_textures[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT].imageView = gPassImageView[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT];
			materials_textures[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT].sampler = gPassImageSampler[G_PASS_METAL_ROUGHNESS_PBR_ATTACHMENT];
			
			// Emission
			materials_textures[G_PASS_EMISSION_ATTACHMENT].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			materials_textures[G_PASS_EMISSION_ATTACHMENT].imageView = gPassImageView[G_PASS_EMISSION_ATTACHMENT];
			materials_textures[G_PASS_EMISSION_ATTACHMENT].sampler = gPassImageSampler[G_PASS_EMISSION_ATTACHMENT];
	
	
		// 3.3 информация о картах теней
		std::array<VkDescriptorImageInfo, MAX_DYNAMIC_LIGHT_SOURCES> shadowmap_textures {};	
		for(int i=0; i<MAX_DYNAMIC_LIGHT_SOURCES; i++)
		{		
			shadowmap_textures[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			shadowmap_textures[i].imageView = gPassImageView[G_PASS_COLOR_ATTACHMENT];	// затычка
			shadowmap_textures[i].sampler = gPassImageSampler[G_PASS_COLOR_ATTACHMENT];	// затычка
		}
		
		printMessage("Try to attach dynamic lights shadowmaps")
		if( CHECK_PTR(g_Lab) ) 
		{
			std::shared_ptr<DynamicLight> source = dynamic_pointer_cast<DynamicLight>(g_Lab->getActor("LabSpot"));
			if( source != nullptr )
			{
				if(source->type == FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL)
				{
					//printMessage("Cannot attach OMNI source to L-pass!")
					//DEBUG_INFO
					//shadowmap_textures[1].imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
					
					if( HANDLE_VALID(source->cubeMapViewSampling) && HANDLE_VALID(source->cubeMapSampler) )
					{
						shadowmap_textures[source->index].imageView = source->cubeMapViewSampling;
						shadowmap_textures[source->index].sampler = source->cubeMapSampler;
						printMessage("Omni OK!")
					}	
				}
				if(source->type == FH_DYNAMIC_LIGHTSOURCE_SPOT)
				{
					if( HANDLE_VALID(source->dynamicLightShadowImageView[0]) && HANDLE_VALID(source->dynamicLightShadowImageSampler[0]) )
					{
						printMessage("Spot OK!")
						shadowmap_textures[source->index].imageView = source->dynamicLightShadowImageView[0];
						shadowmap_textures[source->index].sampler = source->dynamicLightShadowImageSampler[0];
					}
				}
				
			}
		
			std::shared_ptr<DynamicLight> helispot = dynamic_pointer_cast<DynamicLight>(g_Lab->getActor("HeliSpot"));
			if( helispot != nullptr )
			{
				if(helispot->type == FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL)
				{
					//printMessage("Cannot attach OMNI helispot to L-pass!")
					//DEBUG_INFO
					//shadowmap_textures[1].imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
					
					if( HANDLE_VALID(helispot->cubeMapViewSampling) && HANDLE_VALID(helispot->cubeMapSampler) )
					{
						shadowmap_textures[helispot->index].imageView = helispot->cubeMapViewSampling;
						shadowmap_textures[helispot->index].sampler = helispot->cubeMapSampler;
						printMessage("helispot Omni OK!")
					}	
				}
				if(helispot->type == FH_DYNAMIC_LIGHTSOURCE_SPOT)
				{
					if( HANDLE_VALID(helispot->dynamicLightShadowImageView[0]) && HANDLE_VALID(helispot->dynamicLightShadowImageSampler[0]) )
					{
						printMessage("helispot Spot OK!")
						shadowmap_textures[helispot->index].imageView = helispot->dynamicLightShadowImageView[0];
						shadowmap_textures[helispot->index].sampler = helispot->dynamicLightShadowImageSampler[0];
					}
				}
				
			}
		
		}
		
		
		// 4 - описание дескрипторов
		std::array<VkWriteDescriptorSet, 5> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSet_L;
			descriptorWrites[0].dstBinding = 0;							// в фрагментном (!) шейдере: layout(binding = 0) uniform sampler2D maps[2];
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = gPassAttachmentsCount-1;	// Количество текстур в массиве maps[]
			descriptorWrites[0].pImageInfo = materials_textures.data();

			// 4.1 Illumination buffer: struct FhDynamicLightSourceInfo
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSet_L;
			descriptorWrites[1].dstBinding = 1;							// в вершинном (!) шейдере: layout(binding = 1) uniform DynamicLightSourceInfo
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &bufferInfos[0];	//&bufferInfo;
			
			// 4.2 GpassGeneralInfoBuffer : struct FhGeneralInfo
			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = descriptorSet_L;
			descriptorWrites[2].dstBinding = 2;							// в вершинном (!) шейдере: layout(binding = 2) uniform FhGeneralInfo
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pBufferInfo = &bufferInfos[1];	//&bufferInfo;
			
			// 4.3 - shadowmapsSpots samplers2D
			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = descriptorSet_L;
			descriptorWrites[3].dstBinding = 3;							// в фрагментном (!) шейдере: layout(binding = 3) uniform sampler2D shadowMaps[16];
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[3].descriptorCount = MAX_DYNAMIC_LIGHT_SOURCES;	// Количество текстур в массиве maps[]
			descriptorWrites[3].pImageInfo = shadowmap_textures.data();

			// 4.4 - shadowmapsOmni samplers2D
			descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[4].dstSet = descriptorSet_L;
			descriptorWrites[4].dstBinding = 4;							// в фрагментном (!) шейдере: layout(binding = 3) uniform sampler2D shadowMaps[16];
			descriptorWrites[4].dstArrayElement = 0;
			descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[4].descriptorCount = MAX_DYNAMIC_LIGHT_SOURCES;	// Количество текстур в массиве maps[]
			descriptorWrites[4].pImageInfo = shadowmap_textures.data(); // duplicate samplers block! Not an error!


		// 5 - обновление содержимого
		vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		
		printMessage("\t Descriptor set of L-pass updated!") 
		
	}
	
	// 7 - Graphics Pipeline Layout
	{
		// схема прибора. объединение VkDescriptorSetLayout + Push-Constants: общее описание точек привязки данных (ресурсов и переменных)
		// Даже если не намерены передавать в шейдер данные, все равно обязательно нужно создать один пустой VkPipelineLayout!
		// VkPipelineLayout надо обязательно прописать до создания самого конвейера!
		
		//setup push constants
		//~ VkPushConstantRange push_constant;
			//~ push_constant.offset = 0;									//this push constant range starts at the beginning
			//~ push_constant.size = sizeof(FhGpassPushConstantsInfo);				//this push constant range takes up the size of a MeshPushConstants struct
			//~ push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		//this push constant range is accessible only in the vertex shader
		
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			
			// конвейеру должны быть доступны:
			// 1 - описания точек привязки буферов и изображений
			pipelineLayoutInfo.setLayoutCount = 1;						
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_L; 		// тут может быть массив, но пока только один
			
			// 2 - описания т.н. пуш-констант, которые являются другим способом передачи динамических значений в шейдер
			//~ pipelineLayoutInfo.pushConstantRangeCount = 1;
			//~ pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		
		VkResult result = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout_L);
		if( result != VK_SUCCESS ) 
		{
			checkResult(result);
			throw std::runtime_error("failed to create L pipeline layout!");
		}
	}
	
	
	// 8 - Graphics Pipeline
	{
		// создание графического конвейера: этапы, шейдеры, смешивание + описание точек привязки для передачи данных в конвейер
		// 1 - программируемые этапы конвейера
		// 1.1 - загрузить байт-код
		auto vertShaderCode = readFile( FH_SHADER_PATH  "L_pass_v.spv");		// auto vertShaderCode - это статичный вектор символов-байтов
		auto fragShaderCode = readFile( FH_SHADER_PATH  "L_pass_f.spv");

		// 1.2 - собрать из байт-кода шейдерный модуль ("обернуть" байт-код в Vulkan)
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
		
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
			
		auto bindingDescription = FhFullscreenQuadVertex::getBindingDescription();
		auto attributeDescriptions = FhFullscreenQuadVertex::getAttributeDescriptions();
		
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
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;	// предпочитаем МЕНЬШИЕ значения
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;	 // make sure that the format of the depth/stencil image contains a stencil component.


		// 2.6 - смешивание цветов
		// Когда фрагментный шейдер рассчитал цвет пикселя, надо правильно его совместить с цветом из буфера - смешать или скомбинировать.
		// конфигурация привязанного фреймбуфера:
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE; // не совмещать (новый цвет заменяет старый из буфера)

		// 																				color				bloom
		std::array<VkPipelineColorBlendAttachmentState, 2> colorBlendAttachments = {colorBlendAttachment, colorBlendAttachment,};


		// глобальные настройки совмещения цветов (между буферами?):
		VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = static_cast<uint32_t>( colorBlendAttachments.size() );	//1;
			colorBlending.pAttachments = colorBlendAttachments.data();//&colorBlendAttachment;
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
			pipelineInfo.layout = pipelineLayout_L;
			pipelineInfo.renderPass = renderPass_L;			// createRenderPass() должна быть вызвана ДО этого момента!
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			
			pipelineInfo.pDepthStencilState = &depthStencil;

		VkResult result2 = vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_L);
		if ( result2!= VK_SUCCESS)
		{
			checkResult(result2);
			throw std::runtime_error("failed to create L graphics pipeline!");
		}

		// после создания конвейера отдельные шейдеры (вершинный/фрагментный) уже не нужны
		vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);

	}
	
	
	
	
}
   
   
void Graphics::destroyLpass()
{
	// in reverse order
	
	destroyLpassDynamicLightSourceInfoBuffer();
	
	
	vkDestroyPipeline(logicalDevice, pipeline_L, nullptr);

	vkDestroyPipelineLayout(logicalDevice, pipelineLayout_L, nullptr);  
	
	vkDestroyDescriptorPool(logicalDevice, descriptorPool_L, nullptr);
	
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout_L, nullptr);
	
	if(frameBuffer_L != VK_NULL_HANDLE) vkDestroyFramebuffer(logicalDevice, frameBuffer_L, nullptr);
	
	for(int i=0; i<lPassAttachmentsCount; i++)
	{			
		vkDestroySampler(logicalDevice, 	lPassImageSampler[i], nullptr);
		vkDestroyImageView(logicalDevice, 	lPassImageView[i], 	nullptr);
		vkFreeMemory(logicalDevice, 		lPassImageMemory[i], nullptr);
		vkDestroyImage(logicalDevice, 		lPassImage[i], 		nullptr);
	}
	
	vkDestroyRenderPass(logicalDevice, renderPass_L, nullptr);
}


void Graphics::createLpassDynamicLightSourceInfoBuffer()
{
	// создание в видеопамяти буфера для шейдерных переменных и "прокидывание" прямого доступа к нему
		
	// 1. Получаем лимит устройства
	//~ VkPhysicalDeviceProperties deviceProperties;				// info struct
	//~ vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	
	//~ uint32_t alignment = deviceProperties.limits.minUniformBufferOffsetAlignment; // 64

	//~ // 2. Считаем реальный размер структуры
	//~ const size_t structSize = sizeof(FhDynamicLightSourceInfo); // 452

	//~ // 3. Вычисляем выровненный шаг (stride)
	//~ // Формула: (size + alignment - 1) & ~(alignment - 1)
	//~ const uint32_t stride = (structSize + alignment - 1) & ~(alignment - 1);
	// Для 452 и 64: stride = 464? Нет! 452 -> 448? Нет. Давайте посчитаем:
	// 452 / 64 = 7.0625. Округляем вверх: 8 * 64 = 512.
	// stride = 512 байт.
	
	
	
		
	// 1 - размер буфера загружаемых в шейдер (ВЫРОВНЕННЫХ!) данных
	//const int MAX_LIGHTS = 16;
	//const uint32_t totalSize = stride * MAX_LIGHTS; // 512 * 16 = 8192
	
	VkDeviceSize bufferSize = MAX_DYNAMIC_LIGHT_SOURCES * sizeof(FhDynamicLightSourceInfo);	//	stride	// размер буфера =  байта
				
				
	//~ printMessage("\n\n\n\n\n\n\n\n\n\n\nCreate buffer----------------------------------")
	//~ printVariable(sizeof(FhDynamicLightSourceInfo)) // 464
	//~ //printVariable(alignment) // 64
	//~ //printVariable(stride) // 512
	//~ printVariable(bufferSize) // 8192
	//~ printMessage("\n\n\n\n\n\n\n\n\n\n\n----------------------------------")
	
	
	// 2 - аналогично как делали для буфера вершинных данных, только без vkUnmapMemory!
	// 2.1 - создали host-side буфер и выделили ему ВИДИМУЮ! видеопамять
	createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
									LpassDynamicLightSourceInfoBuffer, 
									LpassDynamicLightSourceInfoBufferMemory );

	// 2.2 - сделали прямую ссылку на эту ВИДИМУЮ память и сохранили ее в вектор uniformBuffersMemory
	vkMapMemory(logicalDevice, LpassDynamicLightSourceInfoBufferMemory, 0, bufferSize, 0, &LpassDynamicLightSourceInfoBufferMapped );

	// XXX буфер остается помеченым до конца жизни программы. Эта техника называется “persistent mapping” и работает на всех имплементациях Vulkan	
}


void Graphics::destroyLpassDynamicLightSourceInfoBuffer()
{	
	vkDestroyBuffer(logicalDevice, LpassDynamicLightSourceInfoBuffer, nullptr);
	vkFreeMemory(logicalDevice,  LpassDynamicLightSourceInfoBufferMemory, nullptr);			
}

void Graphics::updateLpassDynamicLightSourceInfoBuffer()
{
	//~ // 1. Получаем лимит устройства
	//~ VkPhysicalDeviceProperties deviceProperties;				// info struct
	//~ vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	
	//~ uint32_t alignment = deviceProperties.limits.minUniformBufferOffsetAlignment; // 64

	//~ // 2. Считаем реальный размер структуры
	//~ const size_t structSize = sizeof(FhDynamicLightSourceInfo); // 452

	//~ // 3. Вычисляем выровненный шаг (stride)
	//~ // Формула: (size + alignment - 1) & ~(alignment - 1)
	//~ const uint32_t stride = (structSize + alignment - 1) & ~(alignment - 1);
	
	
	std::array<FhDynamicLightSourceInfo, MAX_DYNAMIC_LIGHT_SOURCES> lightSourcesData;	// 16 динамических источников
	
	FhDynamicLightSourceInfo playerTorchInfo;
		playerTorchInfo.position = CHECK_PTR(g_Camera) ? g_Camera->getActorPosition() : glm::vec3(0,0, 1);
		playerTorchInfo.direction = CHECK_PTR(g_Camera) ? g_Camera->getActorForwardVector() : glm::vec3(0.707, 0, -0.707);
		playerTorchInfo.color = glm::vec3(1.0,0.98,0.34);
		playerTorchInfo.type = FH_DYNAMIC_LIGHTSOURCE_SPOT;
		playerTorchInfo.sector = 30.0f;
		playerTorchInfo.radius = 10.0f;
		playerTorchInfo.power = 5.0f;
		playerTorchInfo.blendWidth = 25.0f;
		playerTorchInfo.exists = 1;
			
	//~ // write player torch info: 0
	//~ int i=0;
	//~ memcpy(LpassDynamicLightSourceInfoBufferMapped + i * stride, &playerTorchInfo, sizeof(FhDynamicLightSourceInfo));			
	lightSourcesData[0] = playerTorchInfo;
	
	
	if( CHECK_PTR(g_Lab) )
	{
		// XXX заменить на поиск по тегу, чтобы автоматизировать заполнение списка активных источников
		std::shared_ptr<DynamicLight> spot = dynamic_pointer_cast<DynamicLight>(g_Lab->getActor("LabSpot"));
		if( spot != nullptr )
		{
			lightSourcesData[spot->index] = spot->getLightSourceInfo();
		}
		
		std::shared_ptr<DynamicLight> helispot = dynamic_pointer_cast<DynamicLight>(g_Lab->getActor("HeliSpot"));
		if( helispot != nullptr )
		{
			lightSourcesData[helispot->index] = helispot->getLightSourceInfo();
		}
	}
	
	//~ printMessage("\n\n\n\n\n\n\n\n\n\n\nUpdate buffer----------------------------------")
	//~ printVariable(sizeof(FhDynamicLightSourceInfo)) // 512
	//~ //printVariable(alignment) // 64
	//~ //printVariable(stride) //512
	//~ printVariable(lightSourcesData.size()) // 16
	//~ printVariable(sizeof(playerTorchInfo)) // 512
	//~ //printVariable(sizeof(*lightSourcesData.data())) // 8
	//~ printMessage("\n\n\n\n\n\n\n\n\n\n\n----------------------------------")
	
	//~ system("pause");
	
	
	// Создаёшь VkBuffer размером 16 * 512 = 8192 байта
	//~ const VkDeviceSize bufferSize = lightSourcesData.size() * sizeof(FhDynamicLightSourceInfo);
	
	memcpy( LpassDynamicLightSourceInfoBufferMapped, lightSourcesData.data(), MAX_DYNAMIC_LIGHT_SOURCES*sizeof(FhDynamicLightSourceInfo));
	
	
	// Копируешь весь массив в mapped память буфера
	//~ memcpy(LpassDynamicLightSourceInfoBufferMapped, lightSourcesData.data(), bufferSize);
	
	
	
	

	
	//~ for (int i = 0; i < MAX_DYNAMIC_LIGHT_SOURCES; ++i) 
	//~ {
		//~ // Пишем данные подряд, но оставляем «дыры» (padding) между ними
		//~ memcpy(mappedBuffer + i * stride, &lights[i], structSize );
		//~ // Остальное (512 - 452 = 60 байт) остаётся нулями — это нормально
	//~ }
	
}











void Graphics::createFpass()
{
	// 1 - Renderpass attachments
	{
		VkAttachmentDescription finalAttachment{};
			finalAttachment.format = swapChainImageFormat;
			finalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;						// 1 => без мультисемплинга
			// loadOp и storeOp относятся к цвету и глубине 
			// мы знаем, что координаты модели после всех матричных умножений преобразуются в вектор, у которого X и Y это экранные координаты,
			// а компонента Z - экранная глубина пикселя
			finalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// заливать буфер одним цветом перед каждым кадром (назначать всем значениям некую константу)
			finalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;					// результат отрисовки будет сохранен в память и может быть считан потом
			// пока что нам плевать на буфер трафарета, поэтому DONT_CARE
			finalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			finalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			// текстуры и фреймбуферы  представляются как VkImage с определенным форматом пикселей, однако расположение пикселей в памяти может меняться
			// в зависимости от того, что мы пытаемся сделать с изображением
			finalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// плевать, где текстурное прикрепление было расположено ранее
			// VK_IMAGE_LAYOUT_UNDEFINED не гарантирует, что данные из изображения будут сохранены, но мы все равно собирались делать заливку VK_ATTACHMENT_LOAD_OP_CLEAR
			finalAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// (автоматически) целевое расположение текстурного прикрепления - цепочка показа
			
		// 1.1 - субпасс
		// рендерпасс может состоять из множества субпассов. Субпассы это операции рендеринга, зависящие от содержимого фреймбуфера от предыдущего рендерпасса
		// (к примеру, последовательность пост-процессинговых эффектов, применяющихся друг за другом)
		// Если сгруппировать эти операции рендеринга в едииный рендерпасс, то Vulkan может их реорганизовать и выжать больше производительности .
		// Для тестового треугольника оставим одиночный субпасс
		// каждый субпасс ссылается на 1 или несколько прикреплений, описанных ранее. Эти ссылки и есть структуры VkAttachmentReference
		
		// 1.2 - references			
		VkAttachmentReference finalAttachmentRef {};
			finalAttachmentRef.attachment = 0; 			// 
			finalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 	// We intend to use the attachment to function as a color buffer. Vulkan автоматически перенесет текстурное прикрепление в указанное расположение, когда субпасс стартует.

		// 2.2 - subpass
		VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;	// т.к. Vulkan также может подерживать compute subpasses, надо явно указать, что этот субпасс - графический!
			
			subpass.colorAttachmentCount = 1;								// количество цветовых прикреплений
			subpass.pColorAttachments = &finalAttachmentRef;				// ссылка на текстурное прикрепление цвета
			
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
			renderPassInfo.attachmentCount = 1;				// указание количества прикреплений
			renderPassInfo.pAttachments = &finalAttachment;										// привязка прикреплений к рендерпасу
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass_F);

		if( result != VK_SUCCESS ) 
		{
			checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create F render pass!");
		}
	}
		
	
	// 2 - Framebuffer
	{
		uint32_t swch_imageCount;
		vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swch_imageCount, nullptr);
		
		
		frameBuffer_F.resize(swch_imageCount);
		
		for(int i=0; i<swch_imageCount; i++)
		{
			VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass_F;	// с каким рендерпасом фреймбуфер должен быть совместим
				
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = &swapChainImageViews[i];
				
				framebufferInfo.width = swapChainExtent.width;
				framebufferInfo.height = swapChainExtent.height;
				framebufferInfo.layers = 1;			// number of layers in image arrays. Our swap chain images are single images, so the number of layers is 1.

			if( vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &frameBuffer_F[i]) != VK_SUCCESS )
			{
				throw std::runtime_error("failed to create F framebuffer!");
			}
		}		
	}
	

	// 3 - DescriptorSetLayout
	{
		// схема разъема. описание точек привязки только ресурсов (буферов, изображений) к шейдерам
		/*
			VkDescriptorSetLayout describes shader binding points - which types of resource to bind to which place
			For example: to vertex shader we connect un UBO - this will be "binding 0"
						 to fragment shader we connect texture sampler - this will be "binding 1"
		*/
		
		
		std::array<VkDescriptorSetLayoutBinding, 1> bindings = {};
				bindings[0].binding = 0;												// layout(binding = 0) uniform sampler2D maps[]; из фрагментного шейдера
				bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// тип - изображение
				bindings[0].descriptorCount = lPassAttachmentsCount;				// массив на 2-5 ячеек
				bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;					// обращение к текстурам ожидается из фрагментного шейдера
				bindings[0].pImmutableSamplers = nullptr;
			
			//~ // binding 0 -  project and view matrices - layout(binding = 1) uniform FhGeneralInfo {} commonData
				//~ bindings[0].binding = 0;												// индекс точки привязки: layout(binding = 0) uniform FhGeneralInfo из вершинного шейдера
				//~ bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;			// тип - uniform buffer
				//~ bindings[0].descriptorCount = 1;										// UBO-шек может быть много, но сейчас в массиве - только одна. This could be used to specify a transformation for each of the bones in a skeleton for skeletal animation, for example
				//~ bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;					// определили этап конвейера, на котором будет обращение шейдера к UBO. Сейчас - вершинный шейдер
				//~ bindings[0].pImmutableSamplers = nullptr; 								// Optional. Для изображений (image sampling)
		
					
		// 3 - summarize all binding
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();
			
		// 4 - create VkDescriptorSetLayout
		VkResult result = vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout_F);
		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			throw std::runtime_error("Failed to create F descriptor set layout!");
		}
	}
	
	
	// 4 - Descriptor Pool
	{
		// набор разъемов. создание общего перечня сетов.
		/*
			Descriptor pool — это объект, из которого Vulkan выделяет descriptor set-ы. Ты заранее указываешь, сколько дескрипторов каких типов тебе нужно, и pool использует это как резерв памяти/ресурсов для будущих set-ов.

			VkDescriptorPool представляет собой ресурс, используемый для управления набором дескрипторов. 
			Pool позволяет эффективно управлять созданием и удалением дескрипторных наборов (descriptor sets).
			VkDescriptorPool служит инструментом эффективного распределения и повторного использования ресурсов, позволяя избежать 
			затратных операций выделения памяти при каждом изменении состояния графического конвейера.

		*/
		
				
		// 1 - Сначала нужно описать, какие типы дескрипторов содержатся в сетах дескрипторов, и их количество/
		// То есть какие патроны набивать в магазин, помещаемый в разгрузку (пул)
		// Есть пока всего 2 типа передаваемых в шейдер (конвейер) ресурсов - UBO и Image sampler, создадим массив с описанием обоих типов ресурсов
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * lPassAttachmentsCount; // XXX в каждом шейдере - массив на lPassAttachmentsCount сэмплеров буферов из предыдущего прохода
			
			//~ // 1.0 - в движке будет (MAX_FRAMES_IN_FLIGHT * MAX_ENTITIES_IN_ACTION) UBO-шек
			//~ poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			//~ poolSizes[0].descriptorCount =  MAX_ENTITIES_IN_ACTION;
			
			//~ // 1.1 - в движке будет descriptorCount сэмплеров текстур цвета
			//~ poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//~ poolSizes[1].descriptorCount = MAX_ENTITIES_IN_ACTION * MAX_MATERIALS_PER_ENTITY;	// в каждом шейдере - массив на 16 ячеек текстур
			
			//~ poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//~ poolSizes[2].descriptorCount = MAX_ENTITIES_IN_ACTION * MAX_MATERIALS_PER_ENTITY; // XXX в каждом шейдере - массив на 4 ячейки нормалей
			
			
			//~ poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			//~ poolSizes[4].descriptorCount = MAX_ENTITIES_IN_ACTION * MAX_MATERIALS_PER_ENTITY; // XXX в каждом шейдере - массив на 4 ячейки нормалей
			
			//~ // 1.5 - в движке будет descriptorCount наборов матриц материалов
			//~ poolSizes[5].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			//~ poolSizes[5].descriptorCount = MAX_ENTITIES_IN_ACTION;
						

		// 2 - зная все возможные дескрипторы, описываем общий пул для хранения дескрипторов
		VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			
			int entitiesCount = 1; // всего один полноэкранный квад
			poolInfo.maxSets = entitiesCount * MAX_FRAMES_IN_FLIGHT; // Т.е. на каждый кадр в обработке на лету - свой набор (Set!)
			
			// и в каждом таком наборе несколько пулов (maps, ubo etc), каждый из которых содержит заданное число дескрипторов
			poolInfo.poolSizeCount = static_cast<uint32_t>( poolSizes.size() ); // количество пулов
			poolInfo.pPoolSizes = poolSizes.data();
			
			

		// 3 - и выделяем непрерывный блок памяти (pool memory) достаточного размера
		VkResult result = vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool_F);
		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			throw std::runtime_error("failed to create F descriptor pool!");
		}   
	}
	
	
	// 5 - Descriptor Sets
	{
		descriptorSet_F.resize(MAX_FRAMES_IN_FLIGHT);
		
		for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorSetAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = descriptorPool_F;				// указываем пул			
				allocInfo.descriptorSetCount = 1;										// и число сетов
				allocInfo.pSetLayouts = &descriptorSetLayout_F; 			// указываем схему описания подключения ресурсов к конвейеру
		
			// 2 - вырезали кусок памяти из pool memory для конкретного descriptor set (взяли розетки со склада)
			VkResult result = vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet_F[i] );

			if( result != VK_SUCCESS) 
			{
				checkResult(result);
				DEBUG_INFO
				throw std::runtime_error("Failed to allocate F-pass descriptor set for fullscreen quad!");
			}
		
			// 3 - связываем реальные ресурсы с дескрипторами
			// 3.1 информация о буфере матриц PVM		
			//~ std::array<VkDescriptorBufferInfo, 2> bufferInfos{};
				//~ // binding 0
				//~ bufferInfos[0].buffer = g_GraphicsCore.GpassGeneralInfoBuffer;	// ссылка на конкретный буфер UBO
				//~ bufferInfos[0].offset = 0;
				//~ bufferInfos[0].range = sizeof(FhGeneralInfo);				// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
			
				//~ // binding 5
				//~ bufferInfos[1].buffer = materialInfoBuffer;						// ссылка на конкретный буфер матриц конфигов материалов
				//~ bufferInfos[1].offset = 0;
				//~ bufferInfos[1].range = sizeof(FhGpassMaterialsInfo);			// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
			
				//~ // binding 6
				//~ bufferInfos[2].buffer = g_GraphicsCore.lightsUniformBuffers;	// ссылка на конкретный буфер lights
				//~ bufferInfos[2].offset = 0;
				//~ bufferInfos[2].range = sizeof(FhIlluminationInfo);				// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
				

			// 3.2 - информация о текстурах
			//~ VkDescriptorImageInfo imageInfo{};
				//~ imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				//~ imageInfo.imageView = imageView;//textureImageView;
				//~ imageInfo.sampler = sampler;//textureSampler;
		
			// к шейдеру прикрепляются максимум 2-5 картинки: массив текстур на lPassAttachmentsCount сэмплера с предыдущего прохода (L-pass)
			std::array<VkDescriptorImageInfo, lPassAttachmentsCount> materials_textures {};			
				// color resouce
				materials_textures[L_PASS_COLOR_ATTACHMENT].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				materials_textures[L_PASS_COLOR_ATTACHMENT].imageView = lPassImageView[L_PASS_COLOR_ATTACHMENT];
				materials_textures[L_PASS_COLOR_ATTACHMENT].sampler = lPassImageSampler[L_PASS_COLOR_ATTACHMENT];
				
				materials_textures[L_PASS_BLOOM_ATTACHMENT].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				materials_textures[L_PASS_BLOOM_ATTACHMENT].imageView = lPassImageView[L_PASS_BLOOM_ATTACHMENT];
				materials_textures[L_PASS_BLOOM_ATTACHMENT].sampler = lPassImageSampler[L_PASS_BLOOM_ATTACHMENT];
				
				//~ // работает
				//~ materials_textures[L_PASS_COLOR_ATTACHMENT].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				//~ materials_textures[L_PASS_COLOR_ATTACHMENT].imageView = gPassImageView[G_PASS_COLOR_ATTACHMENT];
				//~ materials_textures[L_PASS_COLOR_ATTACHMENT].sampler = gPassImageSampler[G_PASS_COLOR_ATTACHMENT];
				
				//~ materials_textures[L_PASS_BLOOM_ATTACHMENT] = materials_textures[L_PASS_COLOR_ATTACHMENT];				
				
	
			// 4 - описание дескрипторов
			std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = descriptorSet_F[i];
				descriptorWrites[0].dstBinding = 0;							// в фрагментном (!) шейдере: layout(binding = 0) uniform sampler2D maps[2];
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[0].descriptorCount = lPassAttachmentsCount;	// Количество текстур в массиве maps[]
				descriptorWrites[0].pImageInfo = materials_textures.data();

				//~ // 4.1 PVM buffer: struct FhGeneralInfo
				//~ descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				//~ descriptorWrites[0].dstSet = descriptorSet_F[i];
				//~ descriptorWrites[0].dstBinding = 0;							// в вершинном (!) шейдере: layout(binding = 0) uniform UniformBufferObject
				//~ descriptorWrites[0].dstArrayElement = 0;
				//~ descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				//~ descriptorWrites[0].descriptorCount = 1;
				//~ descriptorWrites[0].pBufferInfo = &bufferInfos[0];	//&bufferInfo;

				// 4.2 Image samplers 0,1,2,3
				//~ for(int b=1; b<5; b++)	// заполнение биндингов материалов 0,1,2,3, каждый по 4 текстуры, все 16 текстур описаны в materials_textures 
				//~ {
					//~ descriptorWrites[b].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					//~ descriptorWrites[b].dstSet = descriptorSet_F[i];
					//~ descriptorWrites[b].dstBinding = b;							// в фрагментном (!) шейдере: layout(binding = 1) uniform sampler2D material0_textures;
					//~ descriptorWrites[b].dstArrayElement = 0;
					//~ descriptorWrites[b].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					//~ descriptorWrites[b].descriptorCount = MAX_MATERIALS_PER_ENTITY;	// второй binding вычисляется как dstBinding + (descriptorCount-1)? Количество текстур в дескрипторе
					//~ descriptorWrites[b].pImageInfo = materials_textures.data() + (b-1)*MAX_MATERIALS_PER_ENTITY;	// из общего массива materials_textures берем начало с соответствующим смещением
				//~ }

				//~ // material infos buffer: struct FhGpassMaterialsInfo
				//~ descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				//~ descriptorWrites[5].dstSet = descriptorSet_F[i];
				//~ descriptorWrites[5].dstBinding = 5;							// в вершинном (!) шейдере: layout(binding = 5) uniform FhGpassMaterialsInfo
				//~ descriptorWrites[5].dstArrayElement = 0;
				//~ descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				//~ descriptorWrites[5].descriptorCount = 1;
				//~ descriptorWrites[5].pBufferInfo = &bufferInfos[1];

				//~ // LIGHTS buffer: struct FhIlluminationInfo
				//~ descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				//~ descriptorWrites[6].dstSet = descriptorSet_F[i];
				//~ descriptorWrites[6].dstBinding = 6;							// в вершинном (!) шейдере: layout(binding = 6) uniform FhIlluminationInfo 
				//~ descriptorWrites[6].dstArrayElement = 0;
				//~ descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				//~ descriptorWrites[6].descriptorCount = 1;
				//~ descriptorWrites[6].pBufferInfo = &bufferInfos[2];
		

			// 5 - обновление содержимого
			vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

			#ifdef ENTITY_DEBUG
				printVarComment("\t Descriptor set of entity updated!", entityName ) 
			#endif
		}
	}
	
	
	// 6 - Graphics Pipeline Layout
	{
		// схема прибора. объединение VkDescriptorSetLayout + Push-Constants: общее описание точек привязки данных (ресурсов и переменных)
		// Даже если не намерены передавать в шейдер данные, все равно обязательно нужно создать один пустой VkPipelineLayout!
		// VkPipelineLayout надо обязательно прописать до создания самого конвейера!
		
		//setup push constants
		//~ VkPushConstantRange push_constant;
			//~ push_constant.offset = 0;									//this push constant range starts at the beginning
			//~ push_constant.size = sizeof(FhGpassPushConstantsInfo);				//this push constant range takes up the size of a MeshPushConstants struct
			//~ push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;		//this push constant range is accessible only in the vertex shader
		
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			
			// конвейеру должны быть доступны:
			// 1 - описания точек привязки буферов и изображений
			pipelineLayoutInfo.setLayoutCount = 1;						
			pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_F; 		// тут может быть массив, но пока только один
			
			// 2 - описания т.н. пуш-констант, которые являются другим способом передачи динамических значений в шейдер
			//~ pipelineLayoutInfo.pushConstantRangeCount = 1;
			//~ pipelineLayoutInfo.pPushConstantRanges = &push_constant;
		
		VkResult result = vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout_F);
		if( result != VK_SUCCESS ) 
		{
			checkResult(result);
			throw std::runtime_error("failed to create F pipeline layout!");
		}
	}
	
	
	// 7 - Graphics Pipeline
	{
		//~ printMessage("\n\nFinal Pipeline\n\n")
		
		// создание графического конвейера: этапы, шейдеры, смешивание + описание точек привязки для передачи данных в конвейер
		// 1 - программируемые этапы конвейера
		// 1.1 - загрузить байт-код
		auto vertShaderCode = readFile( FH_SHADER_PATH  "F_pass_v.spv");		// auto vertShaderCode - это статичный вектор символов-байтов
		auto fragShaderCode = readFile( FH_SHADER_PATH  "F_pass_f.spv");

		// 1.2 - собрать из байт-кода шейдерный модуль ("обернуть" байт-код в Vulkan)
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
		
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
			
		auto bindingDescription = FhFullscreenQuadVertex::getBindingDescription();
		auto attributeDescriptions = FhFullscreenQuadVertex::getAttributeDescriptions();
		
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
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;	// предпочитаем МЕНЬШИЕ значения
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;	 // make sure that the format of the depth/stencil image contains a stencil component.


		// 2.6 - смешивание цветов
		// Когда фрагментный шейдер рассчитал цвет пикселя, надо правильно его совместить с цветом из буфера - смешать или скомбинировать.
		// конфигурация привязанного фреймбуфера:
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE; // не совмещать (новый цвет заменяет старый из буфера)

		// глобальные настройки совмещения цветов (между буферами?):
		VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
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
			pipelineInfo.layout = pipelineLayout_F;
			pipelineInfo.renderPass = renderPass_F;			// createRenderPass() должна быть вызвана ДО этого момента!
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
			
			pipelineInfo.pDepthStencilState = &depthStencil;

		VkResult result2 = vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_F);
		if ( result2!= VK_SUCCESS)
		{
			checkResult(result2);
			throw std::runtime_error("failed to create Final graphics pipeline!");
		}

		// после создания конвейера отдельные шейдеры (вершинный/фрагментный) уже не нужны
		vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);

	}
	
	
	
}
   
   
void Graphics::destroyFpass()
{
	// in reverse order
	
	vkDestroyPipeline(logicalDevice, pipeline_F, nullptr);

	vkDestroyPipelineLayout(logicalDevice, pipelineLayout_F, nullptr);  
	
	vkDestroyDescriptorPool(logicalDevice, descriptorPool_F, nullptr);
	
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout_F, nullptr);
	
	uint32_t swch_imageCount;
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swch_imageCount, nullptr);
	for(int i=0; i<swch_imageCount; i++)
	{
		if(frameBuffer_F[i] != VK_NULL_HANDLE) vkDestroyFramebuffer(logicalDevice, frameBuffer_F[i], nullptr);
	}
		
	vkDestroyRenderPass(logicalDevice, renderPass_F, nullptr);
}



