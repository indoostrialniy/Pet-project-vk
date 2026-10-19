#include "fhRender.h"

#include "macros.h"

#include "Scena.h" // MicroWorld, Camera
#include "Camera.h"

#include "Entity.h"
#include "vkTexture.h"
#include "globals.h"

#include <set>
#include <limits>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cstring>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

//~ #define STB_IMAGE_IMPLEMENTATION		// для тел функций
#include <stb_image.h>	// One code file needs to include the header with the STB_IMAGE_IMPLEMENTATION definition to include the function bodies, otherwise we’ll get linking errors.


//~ #define TINYOBJLOADER_IMPLEMENTATION	// для тел функций
#include <tiny_obj_loader.h>


    
// абстрактная функция для удобного создания host-side буфера и связывания его с выделенной видеопамятью
void Graphics::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
{
	// 1 - создание объекта буфера на стороне хоста (CPU)
	VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;										// флаги
		bufferInfo.usage = usage;									// флаги
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// определение способа владения буфером очередями

	VkResult bufferResult = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer);
	if ( bufferResult != VK_SUCCESS) 
	{
		checkResult(bufferResult);
		throw std::runtime_error("failed to create buffer!");
	}
	
	// 2 - определение требуемого типа памяти для буфера
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);
	
	// 3.1 - выделение памяти на GPU...
	VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

	VkResult allocResult = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory);
	if( allocResult != VK_SUCCESS) 
	{
		checkResult(allocResult);
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	// 3.2 - соединение выделенной видеопамяти с host-side дескриптором буфера....
		// The first three parameters are self-explanatory and the fourth parameter is the offset within the region of memory. 
		// Since this memory is allocated specifically for this the vertex buffer, the offset is simply 0. 
		// If the offset is non-zero, then it is required to be divisible by memRequirements.alignment.
	vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0); // 0 - это смещение внутри области памяти
	// антипод - vkFreeMemory

}



void movingEntity()
{
	//~ while(1)//for(int i=0; i<5; i++)
	//~ {
		//~ if( CHECK_PTR(g_g_SceneMicromir) && g_g_SceneMicromir->components.contains("SR3") )
		//~ {
			//~ printMessage("Moving SR3...")

			//~ g_g_SceneMicromir->components["SR3"]->position += glm::vec3(0,0, 0.1);
			//~ //updateUniformBuffer(currentFrame, "SR3"); // обязательно до начала обработки кадра!

			
		//~ }
		//~ else
		//~ {
			//~ printMessage("No entity SR3")
		//~ }
	//~ }	
}





//~ void Graphics::createDescriptorSets( VkImageView& imageView, VkSampler& sampler, std::string entityName)// = "test") int index,  // удаление не нужно, ибо происходит автоматически
//~ {
	//~ if(g_SceneMicromir && g_SceneMicromir->components.contains(entityName)) 
	//~ {
		//~ g_SceneMicromir->components[entityName]->descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	
		//~ // 1 - для каждого кадра создаем свой набор дескрипторов ресурсов
		//~ for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
		//~ {
			
			//~ VkDescriptorSetAllocateInfo allocInfo{};
				//~ allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				//~ allocInfo.descriptorPool = descriptorPool;				// указываем пул
				
				//~ // allocInfo.descriptorSetCount = 1;					// и число сетов
				//~ allocInfo.descriptorSetCount = 1;						// и число сетов

				//~ allocInfo.pSetLayouts = &descriptorSetLayout; 			// указываем схему описания подключения ресурсов к конвейеру
			
			
			//~ // 2 - вырезали кусок памяти из pool memory для конкретного descriptor set (взяли розетки со склада)
			//~ // VkResult result = vkAllocateDescriptorSets(logicalDevice, &allocInfo, &_frames[i].descriptorSet );
			//~ VkResult result = vkAllocateDescriptorSets(logicalDevice, &allocInfo, &g_SceneMicromir->components[entityName]->descriptorSets[i] );

			//~ if( result != VK_SUCCESS) 
			//~ {
				//~ checkResult(result);
				//~ throw std::runtime_error("failed to allocate descriptor set!");
			//~ }
				
			
			//~ // 3 - связываем реальные ресурсы с дескрипторами
			//~ // 3.1 информация о буфере матриц PVM
			//~ VkDescriptorBufferInfo bufferInfo{};
				//~ bufferInfo.buffer = g_SceneMicromir->components[entityName]->uniformBuffers[i];		// ссылка на буфер UBO
				//~ bufferInfo.offset = 0;
				//~ bufferInfo.range = sizeof(UniformBufferObject);	// если переписывать буфер целиком, можно тут указать VK_WHOLE_SIZE
			
			//~ // 3.2 - информация о текстуре
			//~ VkDescriptorImageInfo imageInfo{};
				//~ imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				//~ imageInfo.imageView = imageView;//textureImageView;
				//~ imageInfo.sampler = sampler;//textureSampler;

			//~ // 4 - описание дескрипторов
			//~ std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

				//~ descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				//~ // descriptorWrites[0].dstSet = _frames[i].descriptorSet;
				//~ descriptorWrites[0].dstSet = g_SceneMicromir->components[entityName]->descriptorSets[i];

				//~ descriptorWrites[0].dstBinding = 0;
				//~ descriptorWrites[0].dstArrayElement = 0;
				//~ descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				//~ descriptorWrites[0].descriptorCount = 1;
				//~ descriptorWrites[0].pBufferInfo = &bufferInfo;

				//~ descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				//~ // descriptorWrites[1].dstSet = _frames[i].descriptorSet;
				//~ descriptorWrites[1].dstSet = g_SceneMicromir->components[entityName]->descriptorSets[i];

				//~ descriptorWrites[1].dstBinding = 1;
				//~ descriptorWrites[1].dstArrayElement = 0;
				//~ descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				//~ descriptorWrites[1].descriptorCount = 1;
				//~ descriptorWrites[1].pImageInfo = &imageInfo;

			//~ // 5 - обновление содержимого
			//~ vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		//~ } 
	//~ }	  
//~ }









void Graphics::createCommandPool() 		// создание склада заготовок
{
	// Command pools manage the memory that is used to store the buffers and command buffers are allocated from them. 
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;	// Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(); // потому что будем писать команды для отрисовки

	//~ Command buffers are executed by submitting them on one of the device queues,
	//~ like the graphics and presentation queues we retrieved. Each command pool
	//~ can only allocate command buffers that are submitted on a single type of queue.
	//~ We’re going to record commands for drawing, which is why we’ve chosen the
	//~ graphics queue family.

	for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
	{		
		// создаем объект пула для управления памятью
		if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &_frames[i].commandPool) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}
}
void Graphics::destroyCommandPool()
{
	// уничтожение менеджера памяти команд
	for(int i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
	{
		if(_frames[i].commandPool != VK_NULL_HANDLE) vkDestroyCommandPool(logicalDevice, _frames[i].commandPool, nullptr);		
	}
}







// создание в видеопамяти буфера с вершинными данными треугольника и сохранение дексриптора на этот буфер
void Graphics::createVertexBuffer(const std::vector<Vertex2>& datas, VkBuffer& vbo, VkDeviceMemory& vbomem) 
{
	//~ printVarComment("\n\n\tSIZE: ", datas.size() )
	
	// 1 - размер области памяти для хранения вершинных данных = размер поля для 1 вершины * число вершин
	//~ VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size(); // глобальная vertices хранит все вершинные данные треугольника
	VkDeviceSize bufferSize = sizeof(datas[0]) * datas.size(); // глобальная vertices хранит все вершинные данные треугольника
	
	//~ printVarComment("\n\n\tbufferSize: ", bufferSize )

	
	// 2 - создаем промежуточный буфер, видимый хосту
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	// 2.1 - буфер источник (SRC_BIT)
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	
	// 2.2 - заносим данные в промежуточный буфер
	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);		// делаем кусок видеопамяти для промежуточного буфера видимым для хоста
		memcpy(data, datas.data(), (size_t) bufferSize);							// записываем в него вершинные данные
	vkUnmapMemory(logicalDevice, stagingBufferMemory);								// "запечатываем" кусок обратно
	
	
	// 3 - создали целевой буфер (DST_BIT) на оптимальной памяти видеокарты, но не видимый хосту!
	// назначение буфера - хранить вершины (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vbo, vbomem);
	
	
	// 4 - перемещаем данные о вершинах из промежуточного в целевой
	copyBuffer(stagingBuffer, vbo, bufferSize);
	
	//printMessage("dsdsvssdvsdvdslv dslv slvsl v")
	// 5 - удаляем промежуточный буфер и очищаем видеопамять
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
			
}
void Graphics::destroyVertexBuffer(VkBuffer& vbo, VkDeviceMemory& vbomem)
{
	// удаляем буфер с вершинными данными треугольника и очищаем видеопамять
	vkDestroyBuffer(logicalDevice, vbo, nullptr);
	vkFreeMemory(logicalDevice, vbomem, nullptr);
}



//~ void Graphics::destroyUniformBuffers(std::string entityName)// = "test") int objIndex, 
//~ {
	
	//~ if(g_SceneMicromir && g_SceneMicromir->components.contains(entityName))
	//~ {
		//~ for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		//~ {
			//~ if( CHECK_PTR(g_SceneMicromir->components[entityName]) )	vkDestroyBuffer(logicalDevice, g_SceneMicromir->components[entityName]->uniformBuffers[i], nullptr);
			//~ if( CHECK_PTR(g_SceneMicromir->components[entityName]) )	vkFreeMemory(logicalDevice,  g_SceneMicromir->components[entityName]->uniformBuffersMemory[i], nullptr);
			
			//~ // vkDestroyBuffer(logicalDevice, _frames[i].uniformBuffer, nullptr);
			//~ // vkFreeMemory(logicalDevice,  _frames[i].uniformBufferMemory, nullptr);
			
		//~ }
	//~ }
//~ }




void Graphics::createCommandBuffer() 
{
	for(int i=0;i<MAX_FRAMES_IN_FLIGHT;i++)
	{
		VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = _frames[i].commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// буфер может быть отправлен в очередь на исполнение, но не может быть вызван из другого command buffer.
			allocInfo.commandBufferCount = 1;		// всего один буфер

		if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &_frames[i].commandBuffer) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
}



void Graphics::createSyncObjects() 
{
	VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;		// выставляется сразу в СИГНАЛЬНОМ состоянии, чтобы не было блокировки drawFrame()
											// в самом первом кадре, когда хост может ждать флага, которого физически не может быть выставлено

	for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &_frames[i].imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &_frames[i].renderFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(logicalDevice, &fenceInfo, nullptr, &_frames[i].inFlightFence) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

}
void Graphics::destroySyncObjects()
{
	for(int i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
	{
		if(_frames[i].renderFinishedSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(logicalDevice, _frames[i].renderFinishedSemaphore, nullptr);
		if(_frames[i].imageAvailableSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(logicalDevice, _frames[i].imageAvailableSemaphore, nullptr);
		if(_frames[i].inFlightFence != VK_NULL_HANDLE) vkDestroyFence(logicalDevice, _frames[i].inFlightFence, nullptr);
	}
}




	
// 5 функций для создания изображения как области в видеопамяти с привязанным host-side дескриптором
void Graphics::createTextureImage(std::string filename, VkImage& image, VkDeviceMemory& imageMemory)
{
	// 1 - загрузка изображения
	//printMessage("\ncreateTextureImage()\n")
	
	// 1.1 - загружаем пиксельные данные
	int texWidth, texHeight, texChannels;
	//~ stbi_uc* pixels = stbi_load("textures/bricks-mortar-albedo.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	//~ stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	if (!pixels) {throw std::runtime_error("failed to load texture image!");	}
	
	// 1.2 - создаем промежуточный буфер
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	// в ВИДИМОЙ хосту видеопамяти!
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	
	// 1.3 - загружаем пиксельные данные в видимую видеопамять
	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);		// распечатали
	memcpy(data, pixels, static_cast<size_t>(imageSize));							// записали
	vkUnmapMemory(logicalDevice, stagingBufferMemory);								// запечатали
	
	// 1.4 - пиксельные данные больше не нужны
	stbi_image_free(pixels);
	
	
	// 2 - создание объекта изображения (буфера) в НЕВИДИМОЙ хосту памяти видеокарты
	//~ createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
	createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);


	// 3 - перемещение между слоями
	// 3.1 - переместили в расположение для записи
	transitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	// 3.2 - записали данные в НЕВИДИМУЮ хосту область видеопамяти, отведенной под изображение
	copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	// 3.3 - переместили в расположение для демонстрации
	transitionImageLayout(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// 4 - очистка промежуточного буфера
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
		
}
	

//		функция для переноса пиксельных данных из видимого промежуточного буфера в нужную область видеопамяти
void Graphics::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) 
{
	// копирование данных из ВИДИМОГО хосту буфера в НЕВИДИМУЮ хосту видеопамять происходит через отдельный буфер команд
	
	// 1 - начало буфера
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// 2 - указание параметров копирования пиксельных данных
	VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = { width, height, 1 };

	// 3 - перенос данных
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// 4 - конец буфера и отправка его в графическую очередь на исполнение
	endSingleTimeCommands(commandBuffer);
}

//		функция для изменения расположения текстуры
void Graphics::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) 
{
	// 1 - начало одноразового буфера команд переноса (частая функция)
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// 2 - определение целей
	VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} 
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} 
	else 
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	// 3 - запись команды конвейерного барьера
	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier );

	// 4 - окончание одноразового буфера команд (частая функция)
	endSingleTimeCommands(commandBuffer);
}

void Graphics::destroyTextureImage(VkImage& image, VkDeviceMemory& imageMemory)
{
	vkDestroyImage(logicalDevice, image, nullptr);
	vkFreeMemory(logicalDevice, imageMemory, nullptr);
}



// 3 функции для создания обертки над загруженной текстурой
void Graphics::createTextureImageView(VkImage& image, VkImageView& imageView )
{
	//textureImageView 
	imageView = createImageView(image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}



void Graphics::destroyTextureImageView( VkImageView& imageView )
{
	vkDestroyImageView(logicalDevice, imageView, nullptr);
}

   
//		создание текстурного сэмплера
void Graphics::createTextureSampler( VkSampler& sampler )
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE; // true
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // 1.0f; //
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	VkResult result = vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler);
	if( result != VK_SUCCESS) 
	{
		DEBUG_INFO
		throw std::runtime_error("failed to create texture sampler!");
	}

}

void Graphics::destroyTextureSampler( VkSampler& sampler )
{
	vkDestroySampler(logicalDevice, sampler, nullptr);
}






	




bool Graphics::hasStencilComponent(VkFormat format) 
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}






bool Graphics::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	//return false;
	
	
	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) 
		{
			if (strcmp(layerName, layerProperties.layerName) == 0) 
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound) 
		{
			return false;
		}
	}
	return true;		
}







// -----------------------------------------------------------------------

std::vector<char> Graphics::readFile(const std::string& filename) 
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);	
	// std::ios — это перечисление флагов, которые определяют режимы работы с файлами в C++
	// std::ios::ate - это флаг в языке программирования C++, который после открытия файла перемещает указатель в конец файла. Если этот флаг не установлен, то начальная позиция — начало файла. 
	// std::ios::binary — это флаг, используемый с файловыми потоками (std::ifstream, std::ofstream, std::fstream) для открытия файла в двоичном режиме. Когда файл открыт в двоичном режиме, данные считываются или записываются без специальной обработки символов, а данные рассматриваются как последовательность байтов, а не как текст.
	
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	
	size_t fileSize = (size_t) file.tellg();	// std::fstream::tellg() — функция в C++, которая возвращает текущую позицию указателя файла в потоке файла
	// т.е. открываем файл, с ios::ate сразу ставим указатель в конец и через его позицию (tellg) определяем размер файла в байтах
	
	std::vector<char> buffer(fileSize);			// выделяем вектор под нужное количество байт 

	file.seekg(0);								// Seekg() — метод класса fstream в C++. Он устанавливает текущую позицию в нужную, указываемую числом. В данном случае - в начало
	file.read(buffer.data(), fileSize);			// и читаем в наш вектор данные из файла

	file.close();

	return buffer;
}





void Graphics::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 		// копирование данных между буферами
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
		copyRegion.size = size;
	
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
	
	// Так как есть вспомогательные функции beginSingleTimeCommands и endSingleTimeCommands, то copyBuffer значительно сокращается
	
	//~ // 1 - создали локальный буфер команд
	//~ VkCommandBufferAllocateInfo allocInfo{};
		//~ allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		//~ allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		//~ allocInfo.commandPool = _frames[currentFrame].commandPool;
		//~ allocInfo.commandBufferCount = 1;

	//~ VkCommandBuffer commandBuffer;
	//~ vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

	//~ // 2 - и сразу начали его записывать
	//~ VkCommandBufferBeginInfo beginInfo{};
		//~ beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//~ beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // пометка, что буфер - одноразовый

	//~ vkBeginCommandBuffer(commandBuffer, &beginInfo);	// начало записи команд

		//~ VkBufferCopy copyRegion{};
			//~ copyRegion.srcOffset = 0; // Optional
			//~ copyRegion.dstOffset = 0; // Optional
			//~ copyRegion.size = size;
		//~ vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion); // команда для буфера команд, какой (вершинный) буфер в какой скопировать

	//~ vkEndCommandBuffer(commandBuffer);					// конец записи команд


	//~ // 3 - после записи можно отправить одноразовый буфер команд на исполнение в очередь
	//~ VkSubmitInfo submitInfo{};
		//~ submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		//~ submitInfo.commandBufferCount = 1;
		//~ submitInfo.pCommandBuffers = &commandBuffer;
	//~ // графическая очередь ТОЖЕ ПОДДЕРЖИВАЕТ перемещение
	//~ vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	
	//~ vkQueueWaitIdle(graphicsQueue); // подождать 

	//~ vkFreeCommandBuffers(logicalDevice, _frames[currentFrame].commandPool, 1, &commandBuffer);	// очистка буфера команд
}




//~ void Graphics::updateUniformBuffer(uint32_t currentImage, std::string entityName)// = "test")	//int objIndex , bool rotate = false,
//~ {
	//~ static auto startTime = std::chrono::high_resolution_clock::now();

	//~ auto currentTime = std::chrono::high_resolution_clock::now();
	//~ float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	
	//~ UniformBufferObject ubo{};
		
		//~ // вращаем вокруг оси Z
		//~ //ubo.model = rotate ? glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) : glm::mat4(1);	//
		
		//~ //ubo.view = glm::lookAt(glm::vec3(-0.5f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));		
		
		//~ glm::vec3 POV = CHECK_PTR(g_Camera) ? g_Camera->getActorPosition() : glm::vec3(-1.0f, 0.0f, 0.1f);
		//~ glm::vec3 DOV = POV + (CHECK_PTR(g_Camera) ? g_Camera->getActorForwardVector() : glm::vec3(1.0f, 0.0f, 0.0f));//*Camera::rotation;	//glm::vec3(0.0f, 0.0f, 0.0f);
		//~ glm::vec3 UP = glm::vec3(0.0f, 0.0f, 1.0f);
			
		//~ ubo.view = glm::lookAt(POV, DOV, UP);
		
		//~ //printVariable( (float) swapChainExtent.width / (float) swapChainExtent.height)
		
		//~ if(CHECK_PTR(g_Camera)) ubo.proj = glm::perspective(glm::radians(g_Camera->fCameraFOV), (float) swapChainExtent.width / (float) swapChainExtent.height, g_Camera->fCameraViewNear, g_Camera->fCameraViewDepth);
		//~ ubo.proj[1][1] *= -1; // GLM was originally designed for OpenGL, where the Y coordinate of the clip coordinates is inverted. 
		
	//~ if( g_SceneMicromir && g_SceneMicromir->components.contains(entityName) )	//CHECK_PTR(gameObjects[objIndex]) ) 
	//~ { 	
		//~ memcpy( g_SceneMicromir->components[entityName]->uniformBuffersMapped[currentImage], &ubo, sizeof(ubo)); // memcpy)dest, src, count
	//~ }
	//~ else {printVarComment("Error with entity by name: ", entityName) DEBUG_INFO}
		
//~ }


VkCommandBuffer Graphics::beginSingleTimeCommands() 
{
	// 1 - создание буфера команд из пула от кадра
	VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = _frames[currentFrame].commandPool;			// используется глобальная переменная currentFrame
		allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

	// 2 - начало буфера
	VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Graphics::endSingleTimeCommands(VkCommandBuffer commandBuffer) 
{
	// 1 - окончание буфера
	vkEndCommandBuffer(commandBuffer);

	// 2 - отправка буфера в очередь на исполнение
	VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);	// глобальная переменная graphicsQueue
	
	// 3 - ожидание отработки?
	vkQueueWaitIdle(graphicsQueue);

	// 4 - освобождение 
	vkFreeCommandBuffers(logicalDevice, _frames[currentFrame].commandPool, 1, &commandBuffer);
}
    





//~ void Graphics::createLightsUniformBuffers()
//~ {
	//~ // создание в видеопамяти буфера для шейдерных переменных LIGHTS и "прокидывание" прямого доступа к нему

	//~ // VkBuffer					lightsUniformBuffers;							// host-side хэндлер буфера для переменных (матриц)		
	//~ // VkDeviceMemory				lightsUniformBuffersMemory;					// кусок видеопамяти для буфера переменных
	//~ // void*						lightsUniformBuffersMapped;					// адрес видеопамяти для прямого доступа из функции updateEntityUniformBuffer() и т.п.
			
	//~ // 1 - размер буфера загружаемых в шейдер (ВЫРОВНЕННЫХ!) данных
	//~ VkDeviceSize bufferSize = sizeof(FhIlluminationInfo);			// размер буфера = 16*(4*4)*4 = 1024 байта
			
	
	//~ // 2 - создали host-side буфер и выделили ему ВИДИМУЮ! видеопамять
	//~ createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
									//~ lightsUniformBuffers, 
									//~ lightsUniformBuffersMemory );

	//~ // 3 - сделали прямую ссылку на эту ВИДИМУЮ память и сохранили ее в вектор uniformBuffersMemory
	//~ vkMapMemory(logicalDevice, lightsUniformBuffersMemory, 0, bufferSize, 0, &lightsUniformBuffersMapped );
	// XXX буфер остается помеченым до конца жизни программы. Эта техника называется “persistent mapping” и работает на всех имплементациях Vulkan
//~ }

//~ void Graphics::updateLightsUniformBuffer()
//~ {
	//~ // 1 - grab necessary data
	//~ glm::vec3 cameraPos = CHECK_PTR(g_Camera) ? g_Camera->getActorPosition() : glm::vec3(5,11,44);
	//~ glm::vec3 cameraDir = CHECK_PTR(g_Camera) ? g_Camera->getActorForwardVector() : glm::vec3(0.0, 0.0, -1.0);
	
	//~ // 2 - create & fill instance of FhIlluminationInfo struct
	//~ FhIlluminationInfo Illumination{};	
		
		//~ // player torch
		//~ Illumination.LIGHTS[0][0] = glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 0); 				// pos
		//~ Illumination.LIGHTS[0][1] = glm::vec4(cameraDir.x, cameraDir.y, cameraDir.z, 0.0);			// dir
		//~ Illumination.LIGHTS[0][2] = glm::vec4(1.0, 1.0, 1.0, 0.0);			// color
		//~ Illumination.LIGHTS[0][3] = glm::vec4(1.0, 90.0, 30.0, 15.0);		// type sector radius power
		
		//~ // tunnel lamp
		//~ Illumination.LIGHTS[1][0] = glm::vec4(-13, 4.7, 3.7, 0); 			// pos
		//~ Illumination.LIGHTS[1][1] = glm::vec4(1, 0, 0, 0);					// dir
		//~ Illumination.LIGHTS[1][2] = glm::vec4(0.0, 1.0, 0.0, 0.0);			// color
		//~ Illumination.LIGHTS[1][3] = glm::vec4(0.0, 90.0, 20.0, 10.0);		// type sector radius power
		
		//~ // солнце
		//~ Illumination.LIGHTS[2][0] = glm::vec4(5, 11, 44, 0); 				// pos
		//~ Illumination.LIGHTS[2][1] = glm::vec4(0, 0, -1, 0);					// dir
		//~ Illumination.LIGHTS[2][2] = glm::vec4(0.98, 0.94, 0.7, 0.0);		//glm::vec4(1.0, 0.81, 0.28, 0.0);		// color
		//~ Illumination.LIGHTS[2][3] = glm::vec4(2.0, 170.0, 300.0, 10.0);		// type sector radius power

	//~ // 3 - copy the data to pointer "lightsUniformBuffersMapped" associated with "lightsUniformBuffers"
	//~ memcpy( lightsUniformBuffersMapped, &Illumination, sizeof(Illumination));
//~ }

//~ void Graphics::destroyLightsUniformBuffers()
//~ {	
	//~ vkDestroyBuffer(logicalDevice, lightsUniformBuffers, nullptr);
	//~ vkFreeMemory(logicalDevice,  lightsUniformBuffersMemory, nullptr);		
	// lightsUniformBuffersMapped is just a pointer, nothing to clear	
//~ }


