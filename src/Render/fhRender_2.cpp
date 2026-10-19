#include "fhRender.h"

#include "macros.h"

#include <limits>
#include <algorithm>



//~ #define RENDER_2_DEBUG


/*
 * 
 * name: Graphics::createSwapChain
 * @param: none
 * @return: none
 * Specifies parameters of Swapchain and creates it
 */
void Graphics::createSwapChain() 
{
	
	// Vulkan does not have the concept of a “default framebuffer”, hence it requires
	// an infrastructure that will own the buffers we will render to before we visualize
	// them on the screen. 
	// This infrastructure is known as the swap chain and must
	// be created explicitly in Vulkan. The swap chain is essentially a queue of images
	// that are waiting to be presented to the screen. Our application will acquire
	// such an image to draw to it, and then return it to the queue. 
	
	// 1 - detect all posible ways to present image
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice); 		// query struct with 3 types of data: capabilities, formats, presentModes
	// In all swapChainSupport struct will be placed variants of color coding


	// 2 - specify parameters after extraction:
	VkSurfaceFormatKHR 	surfaceFormat 	= chooseSwapSurfaceFormat(swapChainSupport.formats);		// format is hardcoded in func
	VkPresentModeKHR 	presentMode 	= chooseSwapPresentMode(swapChainSupport.presentModes);		// present mode is hardcoded in func
	VkExtent2D 			extent 			= chooseSwapExtent(swapChainSupport.capabilities);			// figure outresolution: limited or like a window size 

	//
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
	

	// optional console output
	#ifdef RENDER_2_DEBUG
		printMessage("\ncreateSwapchain")
		printVarComment("width ", extent.width)
		printVarComment("height ", extent.height)
	#endif
	
	// 3 - specify count of images in swapchain
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;	// == two
	
	//printVarComment("\n\n\n\nSwapchain images: ", imageCount)
	
	// 3.1 if there is upper-limit of count of images and out ↑↑imageCount↑↑ is greater...
	if (swapChainSupport.capabilities.maxImageCount > 0 				// 0 is a special value that means that there is no maximum
		&& imageCount > swapChainSupport.capabilities.maxImageCount)	// если мы превысили число максимально допустимых изображений
	{
		imageCount = swapChainSupport.capabilities.maxImageCount; // ... bound imageCount to maximum
	}
	
	
	//printVarComment("\n\n\n\nSwapchain images: ", imageCount)
	
	// 4 - convert family indexes to set
	// 4.1 - get required family indexes
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice); // fhRender_2.cpp
	
	// 4.2 - convert struct with <optional> to set of uint32_t
	if( !indices.graphicsFamily.has_value() || !indices.presentFamily.has_value() )
	{
		DEBUG_INFO
		throw std::runtime_error("Cannot create Swapchain: Not all families found!");
	}
	
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };


	// 5 - fill Swapchain info
	VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;								// target Surface descriptor
		createInfo.minImageCount = imageCount;						// images count
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;							// count of image layers. (1 - normal, 2 - stereoscopic)
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if( indices.graphicsFamily != indices.presentFamily ) 			// if this are DIFFERENT families
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	//  Images can be used across multiple queue families without explicit ownership transfers.
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} 
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	// An image is owned by one queue family
																	// at a time and ownership must be explicitly transferred before using it in
																	// another queue family. This option offers the best performance
	}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // игнорировать смешивание по альфе с другими окнами на экране
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE; 					// связано с пересозданием Swapchain. (т.к. его надо пересоздавать при изменении размера окна)


	// 6 - create swapchain 
	VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain);
	if( result != VK_SUCCESS) 
	{
		checkResult(result);
		throw std::runtime_error("failed to create swapchain!");
	}
	
	

	
}


void Graphics::destroySwapChain()
{
	if(swapChain != VK_NULL_HANDLE) 
	{
		vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
	}
}

// -----------------------------------------------------------------------
// Три функции ниже - для настройка Swapchain
// определение формата
VkSurfaceFormatKHR Graphics::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
{
	// из списка форматов поверхностей, поддерживаемых устройством, выберем отвечающий заданным параметрам - 
	// VK_FORMAT_B8G8R8A8_SRGB и VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 				// размер чисел для хранения информации о цветах компонентов пикселя
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 	// способ кодирования цвета
		{
			return availableFormat;
		}
	}
	return availableFormats[0]; // если цикл все-таки дошел до конца, то выберем первый попавшийся формат
}

// определение режима отображения (синхронизации с обновлением экрана?)
VkPresentModeKHR Graphics::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
{
	//~ for (const auto& availablePresentMode : availablePresentModes) 
	//~ {
		//~ if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		//~ {
			//~ return availablePresentMode;
		//~ }
	//~ }

	printMessage("Hardcoded VK_PRESENT_MODE_FIFO_KHR!")
	DEBUG_INFO
	
	return VK_PRESENT_MODE_FIFO_KHR; 		// режим (отображения?) по-умолчанию
}

// определение размера изображения
VkExtent2D Graphics::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
{
	if( capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() ) 
	{
		return capabilities.currentExtent;
	} 
	else 
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}




void Graphics::cleanupSwapChain()
{
	
	//~ vkDestroyImageView(logicalDevice, depthImageView, nullptr);
	//~ vkDestroyImage(logicalDevice, depthImage, nullptr);
	//~ vkFreeMemory(logicalDevice, depthImageMemory, nullptr);

	//destroyFramebuffers();
	destroySwapchainImageViews();
	destroySwapChain();
}


void Graphics::recreateSwapChain() 
{
	// overvoorde version
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(logicalDevice);

	cleanupSwapChain();

	createSwapChain();
	createSwapchainImageViews();
	//createDepthResources();
	//createFramebuffers();
		
	//~ // createRenderPass(); 		// для создания пайплайна рендерпасс уже должен быть создан, поэтому функция вызывается до! А ему самому нужен готовый свапчейн
	//~ // createGraphicsPipeline();
	
}






/*
 * 
 * name: Graphics::getSwapchainImages
 * @param: none
 * @return: none
 * Gets the handles of Swapchain VkImages and stores them in vector "swapChainImages"
 */
void Graphics::getSwapchainImages()
{
	
	// we need to specify how to handle swap chain images that will be used across multiple queue families. 
	// That will be the case in our application if the graphics queue family is different from the presentation queue. 
	// We’ll be drawing on the images in the swap chain from the graphics queue and then submitting them on the presentation queue. 
	
	// 1 - Swapchain is already created, now we extract image handlers.
	// Vulkan implementation allows to create Swapchains with multiple images, so query Swapchain size first
	// get count of images in swapchain. Actually it was specified by us in createSwapchain()
	uint32_t swch_imageCount;
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swch_imageCount, nullptr);
	
	// 2 - then resize the container...
	swapChainImages.resize(swch_imageCount);
	
	// 3 - get handles to VkImages of the Swapchain and put them in vector "swapChainImages"
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swch_imageCount, swapChainImages.data());

	#ifdef RENDER_2_DEBUG
		printVarComment("Images in swapchain: ", swch_imageCount)
	#endif
}



/*
 * 
 * name: Graphics::createSwapchainImageViews
 * @param: none
 * @return: none
 * Abstract function for creating VkImageView shell for every VkImage in Swapchain in cycle
 */
void Graphics::createSwapchainImageViews() 
{
	
	// VkImageView это обертка над объектом изображения в памяти. 
	// Она описывает, как обращаться к изображению и к какой его части. 
	// К примеру, изображение может быть обработано как 2D depth texture без мипмаппинга.

	// Данная функция создает базовый VkImageView для каждого изображения в цепочке, чтобы их можно было использовать как render target
	// 1 - Имплементация Vulkan допускает создание цепочки с множеством изображений, поэтому сначала надо запросить размер цепочки...
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) 
	{
		// т.к. есть вспомогательная функция createImageView, то можно сократить код до:
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
	
}

void Graphics::destroySwapchainImageViews()
{
	//~ Unlike images, the image views were explicitly created by us, so we need to add a loop to destroy them 
	// уничтожение звеньев цепочки
	for (auto imageView : swapChainImageViews) 
	{
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}
}



/*
 * 
 * name: Graphics::createImageView
 * @param: VkImage handler, format, AspectFlag
 * @return: VkImageView
 * Creates VkImageView for selected VkImage depending on arguments
 */
VkImageView Graphics::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlag) 
{
	//		абстрактная функция для удобного создания оберток для кусков видеопамяти.
	//		используется при создании изображений текстур и изображений из swapchain
	
	// 1 - указание данных об изображении
	VkImageViewCreateInfo viewInfo{};									// структура для настройка обертки изображения
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;	//VkImage
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;	//VK_FORMAT_R8G8B8A8_SRGB
		viewInfo.subresourceRange.aspectMask = aspectFlag;//VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

	// 2 - создание обертки
	VkImageView imageView;	
	
	VkResult result = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView);
	if( result != VK_SUCCESS) 
	{
		checkResult(result);
		DEBUG_INFO
		throw std::runtime_error("failed to create image view!");
	}

	return imageView;
}

























VkFormat Graphics::findDepthFormat() 
{
	VkFormat fdf_DepthFormat;
	
	fdf_DepthFormat = findSupportedFormat(	{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
										VK_IMAGE_TILING_OPTIMAL,
										VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT // обратить внимание: Make sure to use the VK_FORMAT_FEATURE_ flag instead of VK_IMAGE_USAGE_ 
									);
	
	return fdf_DepthFormat;
}


VkFormat Graphics::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
{
	for (VkFormat format : candidates) 
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
		{
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}


// 		абстрактная функция для удобного многократного создания изображений
void Graphics::createImage(	uint32_t width, 
					uint32_t height, 
					VkFormat format, 
					VkImageTiling tiling, 
					VkImageUsageFlags usage, 
					VkMemoryPropertyFlags properties, 
					VkImage& image, 
					VkDeviceMemory& imageMemory) 
{
	// 1 - создание host-side хэндлера изображения
	VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// изображением сиюмоментно владеет только 1 очередь

	VkResult result = vkCreateImage(logicalDevice, &imageInfo, nullptr, &image);
	if( result != VK_SUCCESS) 
	{
		checkResult(result);
		throw std::runtime_error("failed to create image!");
	}

	// 2 - определение требований к памяти
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

	// 3 - выделение видеопамяти под изображение
	VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

	VkResult allocResult = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory);
	if( allocResult != VK_SUCCESS) 
	{
		checkResult(allocResult);
		throw std::runtime_error("failed to allocate image memory!");
	}

	// 4 - связывание выделенной видеопамяти с host-side хэндлером
	vkBindImageMemory(logicalDevice, image, imageMemory, 0); // 0 - это offset
	
}




bool createImage2( VkImage& image, 
							VkDeviceMemory& imageMemory, 
							VkImageView& imageView,
							VkSampler& imageSampler,
							uint32_t width,
							uint32_t height,
							uint32_t depth,
							uint32_t arrayLayersCount,
							uint32_t mipLevels,
							VkImageType type,
							VkFormat format,
							VkImageTiling tiling,
							VkImageUsageFlags usage,
							VkImageViewType viewType,
							VkImageAspectFlags aspectFlag,
							uint32_t baseMipLevel,
							uint32_t levelCount,
							uint32_t baseArrayLayer,
							uint32_t layerCount
							)
{
	
	/*
	// 1 - create image
	{
		// 1 - создание host-side хэндлера изображения
		VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = width;
			imageInfo.extent.height = height;
			imageInfo.extent.depth = depth;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = arrayLayersCount;
			imageInfo.format = format;
			imageInfo.tiling = tiling;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = usage;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// изображением сиюмоментно владеет только 1 очередь

		VkResult result = vkCreateImage(logicalDevice, &imageInfo, nullptr, &image);
		
		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create image!");
		}

		// 2 - определение требований к памяти
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

		// 3 - выделение видеопамяти под изображение
		VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

		VkResult allocResult = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory);
		if( allocResult != VK_SUCCESS) 
		{
			checkResult(allocResult);
			DEBUG_INFO
			throw std::runtime_error("failed to allocate image memory!");
		}

		// 4 - связывание выделенной видеопамяти с host-side хэндлером
		vkBindImageMemory(logicalDevice, image, imageMemory, 0); // 0 - это offset
	}
	
	
	// 2 - create image view
	{
		//		абстрактная функция для удобного создания оберток для кусков видеопамяти.
		//		используется при создании изображений текстур и изображений из swapchain
		
		// 1 - указание данных об изображении
		VkImageViewCreateInfo viewInfo{};									// структура для настройка обертки изображения
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = image;	//VkImage
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;	//VK_FORMAT_R8G8B8A8_SRGB
			viewInfo.subresourceRange.aspectMask = aspectFlag;//VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

		// 2 - создание обертки
		VkImageView imageView;	
		
		VkResult result = vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView);
		if( result != VK_SUCCESS) 
		{
			checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("failed to create image view!");
		}

		return imageView;
	}
	
	// 3 - create image sampler
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

		VkResult result = vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &imageSampler);
		if( result != VK_SUCCESS) 
		{
			DEBUG_INFO
			throw std::runtime_error("failed to create texture sampler!");
		}	
	}
	
		
			g_GraphicsCore.createTextureSampler( dynamicLightShadowImageSampler[i] );
	*/
	
	return false;
}
