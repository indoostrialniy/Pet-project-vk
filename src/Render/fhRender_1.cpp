#include "fhRender.h"

#include "macros.h"

#include <set>
#include <algorithm>


#define RENDER_1_DEBUG


/*
 * 
 * name: Graphics::createInstance
 * @param: none
 * @return: none
 * Just create a Vulkan instance
 */
void Graphics::createInstance() 
{
	if( enableValidationLayers && !checkValidationLayerSupport() ) 	// если нужны слои, но они недоступны, выбросим исключение
	{
		DEBUG_INFO
		throw std::runtime_error("Error: validation layers requested, but not available!");
	}
	
	// 1.1
	// Необходимые на данной платформе расширения экземпляра GLFW определяет сам
	std::vector<const char*> glfwRequiredExtensions = getRequiredExtensions();						// получили вектор строк с именами требуемых экземпляру расширений
		// glfwRequiredExtensions:	VK_KHR_surface
        // 							VK_KHR_win32_surface
        // 							VK_EXT_debug_utils
	
	
	// 1.2
	// Доступные на данной платформе расширения экземпляра получим вручную
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);						// get extensions count			
	
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);							// vector of available extensions
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());	// get available Vulkan extensions names
		
	std::vector<const char*> availableExtensionsNames(extensionCount);								// vector of just names of available extensions
	for(int i=0; i<extensionCount; i++)	{															// get only available Vulkan extensions names
		availableExtensionsNames[i] = availableExtensions[i].extensionName;		}					// iterative copying names

	#ifdef RENDER_1_DEBUG
		std::cout << "\nAvailable instance extensions:\n";											// optional console output
		for (const auto& extension : availableExtensions)	{ 										// перебор доступных
			std::cout << '\t' << extension.extensionName << '\n';		}		
	#endif
	
	// 1.3
	// Проверяем, что необходимые - доступны
	if( !hasCommonStrings(glfwRequiredExtensions, availableExtensionsNames) )						// if there are required extensions
	{
		DEBUG_INFO
		throw std::runtime_error("Error: required extensions are not available! Failed to create VkInstance!");
	}
	
	// 2
	// Раз все в порядке, заполняем формы и создаем экземпляр
	VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "DemoLevel";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "fhEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 2, 5);
		appInfo.apiVersion = VK_API_VERSION_1_3;
		
	VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;		
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwRequiredExtensions.size());	// занесли в структуру количество расширений на основании размера вектора
		createInfo.ppEnabledExtensionNames = glfwRequiredExtensions.data();							// занесли в структуру имена самих расширений
				
	// depending on debug mode, specify validation layers count
	if (enableValidationLayers) 	{
		createInfo.enabledLayerCount =	static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();		}	// "VK_LAYER_KHRONOS_validation"
	else 	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;		}
	
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if ( result != VK_SUCCESS) 
	{
		checkResult(result);
		DEBUG_INFO
		throw std::runtime_error("Error: failed to create VkInstance!");
	}
	
}

void Graphics::destroyInstance()
{
	vkDestroyInstance(instance, nullptr);
	instance = VK_NULL_HANDLE;
}



/*
 * 
 * name: Graphics::createSurface
 * @param: none
 * @return: none
 * Create a connection unit between Vulkan-app and native windowing system, called "Surface", which allows to present images on the screen
 */
void Graphics::createSurface() 
{	
	/*
		Surface is a object which connect Vulkan to native Windows or Linux windowing system (Windows, X11/Wayland created with GLFW/SDL etc) 
		Surface is like an adapter between Vulkan and screen system, because Vulkan doesn`t current OS context.
		Surface helps Vulkan to figure out: is it possible to draw in this window
											which formats are supported
											which present modes are available
											which is the window size etc.
		
		Surface must exist to create later the Swapchain (set of pictures presenting on screen)!
		But Vulkan can also work without Surface (offscreen rendering etc)
		
		Surface is like a canvas, on which Vulkan draw?
		Surface allows Vulkan to present images on screen.
		
		Each platform needs a different function call to create a VkSurfaceKHR:
			Win32 : vkCreateWin32SurfaceKHR( )
			Android : vkCreateAndroidSurfaceKHR( )
			Linux : vkCreateXcbSurfaceKHR( )
		But here we create it with GLFW!
	*/	
	
	if( HANDLE_NULL(surface) )
	{
		VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
		
		if ( result != VK_SUCCESS) 
		{
			checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("Error: failed to create glfw window surface!");
		}
	}
}

void Graphics::destroySurface() 
{
	vkDestroySurfaceKHR(instance, surface, nullptr);
	surface = VK_NULL_HANDLE;
}





/*
 * 
 * name: Graphics::pickPhysicalDevice
 * @param: none
 * @return: none
 * Picks suitable device from availables. 
 * This part no require manual destroy of VkPhysicalDevice!!!!
 */
void Graphics::pickPhysicalDevice() 
{
	// 1
	// figure out the number of all available physical devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);				

	if (deviceCount == 0) 
	{
		DEBUG_INFO
		throw std::runtime_error("Error: failed to find GPUs with Vulkan support!");
	}


	// 2
	// save info about all detected physical devices
	std::vector<VkPhysicalDevice> devices(deviceCount);					// make vector of enough capacity
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());	

	#ifdef RENDER_1_DEBUG
		printMessage("\nPhysical devices and info: ")      				// optional console output 
		for( auto dev : devices)
		{	
			// VkPhysicalDeviceType 1->VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, 2->VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, 4->VK_PHYSICAL_DEVICE_TYPE_CPU
					
			VkPhysicalDeviceProperties deviceProperties;				// info struct
			vkGetPhysicalDeviceProperties(dev, &deviceProperties);

			std::cout << "\n\tDevice Name: " << deviceProperties.deviceName << std::endl;
			std::cout << "\tDevice Type: ";
			switch(deviceProperties.deviceType)
			{
				case VK_PHYSICAL_DEVICE_TYPE_OTHER: std::cout << "unknown other" << std::endl; break;			//0
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: std::cout << "integrated" << std::endl; break;		//1
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: std::cout << "discrete" << std::endl; break;			//2
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: std::cout << "virtual" << std::endl; break;			//3
				case VK_PHYSICAL_DEVICE_TYPE_CPU: std::cout << "CPU" << std::endl; break;						//4
			}		
			std::cout << "\tDriver Version: " << deviceProperties.driverVersion << std::endl;
			
			std::cout << "\tAPI Version: " 	<< VK_VERSION_MAJOR(deviceProperties.apiVersion) 
										<< "." << VK_VERSION_MINOR(deviceProperties.apiVersion) 
										<< "." << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
			
			printVarComment("\tOffsetALignment: ", deviceProperties.limits.minUniformBufferOffsetAlignment)
										
			// see GPU features
			// VkPhysicalDeviceFeatures deviceFeatures;
			// vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		}
	#endif



	// 3
	// now select the most suitable device. We can decide on simply by name/type or (if there are no choices) estimate GPU "score points" and pick the "winner"
	for (const auto& device : devices) 
	{
		if( isDeviceSuitable(device) ) 									// очереди, расширения, свапчейн и фичи в порядке
		{
			physicalDevice = device;									// as soon as found suitable device, save its descriptor and break loop
			break;													
		}
	}
	
	if( !HANDLE_VALID(physicalDevice) ) 									// if there are no suitable devices, we have no chance(
	{
		DEBUG_INFO
		throw std::runtime_error("Error: failed to find a suitable GPU!");	// 
	}
	
	
	// 4 
	// query properties of selected GPU
	vkGetPhysicalDeviceProperties(physicalDevice, &mGpuProperties); // again like in p.2 ↑↑↑
	
	// 5 - optional console output about selected physical device
	#ifdef RENDER_1_DEBUG
		std::cout << "\n\nSelected Physical Device Name: " << mGpuProperties.deviceName << std::endl;
		std::cout << "\tDevice Type: ";
		switch(mGpuProperties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_OTHER: std::cout << "unknown other" << std::endl; break;	//0
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: std::cout << "integrated" << std::endl; break;		//1
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: std::cout << "discrete" << std::endl; break;			//2
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: std::cout << "virtual" << std::endl; break;			//3
			case VK_PHYSICAL_DEVICE_TYPE_CPU: std::cout << "CPU" << std::endl; break;			//3
		}		
		std::cout << "\tDriver Version: " << mGpuProperties.driverVersion << std::endl;
		std::cout << "\tAPI Version: " 	<< VK_VERSION_MAJOR(mGpuProperties.apiVersion) 
										<< "." << VK_VERSION_MINOR(mGpuProperties.apiVersion) 
										<< "." << VK_VERSION_PATCH(mGpuProperties.apiVersion) << std::endl;
	#endif
	
	
	//system("pause");
}





/*
 * 
 * name: Graphics::createLogicalDevice
 * @param: none
 * @return: none
 * After picking physical device create a Logical device - main Vulkan object
 */
void Graphics::createLogicalDevice() 
{
	
	// 1 - indexing required queue families of previously already picked physical device
	// findQueueFamilies() is a function for searching and indexing queue families (XXX at the moment - hardcoded graphic & present). 
	// Already called previously in isDeviceSuitable() for any of PhysicalDevice
	
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);	// get struct with family indexes
	
	// 1.1 - convert struct with <optional> to set of uint32_t
	if( !indices.graphicsFamily.has_value() || !indices.presentFamily.has_value() )
	{
		DEBUG_INFO
		throw std::runtime_error("Cannot create logical device: Not all families found!");
	}
	
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() }; // set of queue family indexes

	
	// 2 - Создание логического устройства похоже на создание VkInstance и включает в себя описание возможностей, которые мы будем использовать. 
	// Нам нужно указать, какие очереди из имеющихся доступных семейств необходимо зарезервировать. (т.е. уже сейчас надо знать их индексы) 
	// При необходимости из одного физического устройства можно создать несколько логических устройств.
	//~ struct VkDeviceQueueCreateInfo {
		//~ VkStructureType             sType;
		//~ const void*                 pNext;
		//~ VkDeviceQueueCreateFlags    flags;
		//~ uint32_t                    queueFamilyIndex;
		//~ uint32_t                    queueCount;
		//~ const float*                pQueuePriorities;
	//~ }
	
	
	
	// Каждая очередь в семействе имеет приоритет (число с плавающей точкой от 0.0 до 1.0).
	// Приоритет необходимо указать даже в случае, если мы используем всего одну очередь!
	// Чем ближе значение к 1.0f, тем выше приоритет соответствующей очереди.
	// Значение влияет на распределение вычислительных ресурсов между разными очередями внутри одной семьи.
	// Важно понимать, что сами команды внутри отдельной очереди выполняются последовательно, независимо от значения приоритета. 
	// Однако приоритеты определяют порядок распределения ресурсов (таких как пропускная способность процессора, 
	// доступ к памяти и другим ресурсам) между различными очередями одной семьи.

	// Например, если у нас две очереди в одном семействе:
	// Первая очередь с приоритетом 0.8f,
	// Вторая очередь с приоритетом 0.2f.
	// Это значит, что первая очередь получит больше ресурсов и сможет быстрее исполнять команды, чем вторая.
	
	
	// 3 - configure required queue families: priorities, count
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;			// создали массив для хранения структур информации о ВСЕХ резервируемых очередях

		
	float queuePriority = 1.0f; // такой финт ушами потому, что очередей в семействе может быть несколько и спецификация Vulkan требует указывать значение как массив
	
	for (uint32_t queueFamily : uniqueQueueFamilies) // iterate family indexes
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;		// 2
			queueCreateInfo.queueFamilyIndex = queueFamily;			// индекс семейства
			queueCreateInfo.queueCount = 1;							// количество очередей в семействе (конвейеров исполнения) - их может быть несколько
			queueCreateInfo.pQueuePriorities = &queuePriority;		// массив float-ов, определяющих относительные приоритеты очередей одной и той же семьи.

		queueCreateInfos.push_back(queueCreateInfo);				// store struct with info about reserved queue
	}
	
	// 3.1 - now we have vector with info about all necessary queues!
	
	
	// 4 - фичи видеокарты

	
   
	// 5 - прописываем информацию о создаваемом логическом устройстве
	VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		// 5.1 - резервируемые очереди
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());		// количество резервируемых очередей
		createInfo.pQueueCreateInfos = queueCreateInfos.data();									// занесли инфу об очередях

		// 5.2 - требуемые расширения
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());		// количество
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();							// инфа
	
		// 5.3 - Сначала описываем, какие фичи нам нужны
				/*
				//~ VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
					//~ indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
					//~ indexingFeatures.pNext = nullptr;
					//~ // ВАЖНО: включаем динамическую индексацию массивов сэмплеров
					//~ indexingFeatures.shaderSampledImageArrayNonUniformIndexing  = VK_TRUE;	//shaderSampledImageArrayDynamicIndexing = VK_TRUE;
					// (Опционально) если планируешь менять MIP-уровень динамически
					//indexingFeatures.shaderResourceMinLod = VK_TRUE; 

				//----------
				//~ printMessage("Flag")
				//~ VkPhysicalDeviceFeatures2 pFeatures;
				//~ pFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

				//~ vkGetPhysicalDeviceFeatures2( physicalDevice,    &pFeatures);
				//~ VkPhysicalDeviceFeatures tmp = pFeatures.features;
				
				//~ const VkBool32* tmpp  = reinterpret_cast<const VkBool32*>(&tmp);
	
				//~ #ifdef RENDER_1_DEBUG
					//~ printMessage("\nRequiered tmpp device features:")
					//~ for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++i) {
						//~ if( tmpp[i] ) {		std::cout << "\t" << i << std::endl;}		}
				//~ #endif

				//~ system("pause");
				//----------
				*/
				
			// Оборачиваем все в VkPhysicalDeviceFeatures2, чтобы передать в vkCreateDevice
			VkPhysicalDeviceFeatures2 features2{};
				features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
				features2.pNext = &indexingFeatures; // фичи 1_1+ проверены на этапе выбора физического устройства
				features2.features = mGpuFeatures; // фичи 1_0 проверены на этапе выбора физического устройства

		// И передаём в createInfo
		createInfo.pEnabledFeatures = nullptr; // &mGpuFeatures. Если используем VkPhysicalDeviceFeatures2, старое поле оставляем nullptr
		createInfo.pNext = &features2;          // <-- это ключ: фичи идут через pNext


		// 5.4 - слои валидации
		// Device Layers have never worked since Vulkan 1.0 and only Instance Layers should be used instead: https://docs.vulkan.org/spec/latest/appendices/legacy.html#legacy-devicelayers.
		

	// 6 - теперь непосредственно создаем главное логическое устройство, а очереди зарезервируются автоматически
	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
	if (result != VK_SUCCESS) 
	{
		checkResult(result);
		DEBUG_INFO
		throw std::runtime_error("Error: failed to create logical VkDevice!");
	}
	
	// 7 - т.к. очереди резервируются автоматически, нужно явным образом сохранить их дескрипторы для дальнейшего взаимодействия
	// 				logicDevice,		   queueFamilyIndex,     queueIndex,  descriptor
	// т.к. ранее queueCreateInfo.queueCount = 1, то в каждом семестве всего по 1 очереди, соответственно, их индексы - 0
	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
	
}

void Graphics::destroyLogicalDevice()	// уничтожение логического устройства
{
	vkDestroyDevice(logicalDevice, nullptr); 
	logicalDevice = VK_NULL_HANDLE;
	
	// прямые дескрипторы очередей удаляются автоматически - graphicsQueue и presentQueue
}







/*
 * 
 * name: Graphics::getRequiredExtensions
 * @param: none
 * @return: vector of 
 * Auxiliary C++ func to figure out if there are required Vulkan extensions
 */
std::vector<const char*> Graphics::getRequiredExtensions()					// запрос требуемых расширений instance`а
{
	/*
	Расширения instance-уровня, нужные для создания окна-поверхности, стандартизированы заранее:

	VK_KHR_surface — универсальное, всегда (это «передняя часть» WSI, абстракция поверхности);
		платформенно-зависимое второе: 	VK_KHR_win32_surface на Windows, 
										VK_KHR_xcb_surface/VK_KHR_xlib_surface на Linux, 
										VK_KHR_wayland_surface под Wayland, 
										VK_KHR_metal_surface на macOS (через MoltenVK), 
										VK_KHR_android_surface на Android.
	*/
	
	uint32_t greExtensionsCount = 0;
	const char** greExtensions;																// массив строк с именами расширений
	
	greExtensions = glfwGetRequiredInstanceExtensions(&greExtensionsCount);					// записали в массив строки с именами требуемых расширений, заодно по ссылке записав их количество

	std::vector<const char*> extensions(greExtensions, greExtensions + greExtensionsCount);	// конструктор принимает 2 итератора (начало и конец) и т.о. копирует в вектор содержимое массива строк имен

	if (enableValidationLayers) 																// если слои валидации включены, добавим еще и расширение "VK_EXT_debug_utils"
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);								// "VK_EXT_debug_utils"
	}

	#ifdef RENDER_1_DEBUG
		printMessage("\nRequired by GLFW instance extensions:")
		for(auto requiredExtension : extensions)
		{
			std::cout << "\t" << requiredExtension << "\n";
		}
	#endif
	
	return extensions;																			// вернули вектор требуемых расширений
}




/*
 * 
 * name: Graphics::hasCommonStrings
 * @param: 2 vectors of strings (as char*)
 * @return: bool success
 * Auxiliary C++ func to figure out if there are common "strings" in both vectors
 */
bool Graphics::hasCommonStrings(const std::vector<const char*> &v1, const std::vector<const char*> &v2)
{
	if( v1.empty() || v2.empty() ) return false;

	// Конвертируем указатели на строки в string_view
	std::vector<std::string_view> sv1(v1.size()), sv2(v2.size());
	
	transform( v1.begin(), v1.end(), sv1.begin(), [](auto str){ return std::string_view(str); } );
	transform( v2.begin(), v2.end(), sv2.begin(), [](auto str){ return std::string_view(str); } );

	sort(sv1.begin(), sv1.end());
	sort(sv2.begin(), sv2.end());

	std::vector<std::string_view> intersection;
	
	set_intersection(sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), back_inserter(intersection));

	#ifdef RENDER_1_DEBUG
		printMessage("Available required instance extenstions: ")
		for(auto Common : intersection)
		{
			std::cout << "\t" << Common << "\n";
		}
	#endif

	return !intersection.empty();
}






// -----------------------------------------------------------------------
// 4 functions below - for checking VkPhysicalDevice suitability
// -----------------------------------------------------------------------


/*
 * 
 * name: Graphics::isDeviceSuitable
 * @param: VkPhysicalDevice
 * @return: bool if selected device is suitable or not
 * Checks if: 	1 - VkPhysicalDevice supports requiered queue families 
 * 				2 - VkPhysicalDevice supports required extensions
 * 				3 - Swapchain will be adequate (VkSurfaceKHR supports required formats etc)
 */
bool Graphics::isDeviceSuitable(VkPhysicalDevice ids_phDevice)
{	
	// 1 - check if device supports queue families. 
	// Both Graphics and Present Queue Families are hardcoded in QueueFamilyIndices struct.
	// findQueueFamilies() returns a QueueFamilyIndices struct which contains indexes of detected queue families
	QueueFamilyIndices indices = findQueueFamilies(ids_phDevice);			//


	// 2 - check extensions support, enumerated in "deviceExtensions"
	bool extensionsSupported = checkDeviceExtensionSupport(ids_phDevice);
	
	
	// 3 - check swapchain, because we need to present images on screen
	// Surface is also an Extension! So we need previous bool flag ↑↑↑
	bool swapChainAdequate = false;									
	if( extensionsSupported )
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(ids_phDevice);	// query swapchain info: capabilities, formats, presentModes
		
		swapChainAdequate = 	!swapChainSupport.formats.empty() 
							&& 	!swapChainSupport.presentModes.empty(); // where are capabilities?
	}
	
	// 4 - features available
	bool featuresAvailable = checkDeviceFeaturesSupport(ids_phDevice);
	// for anisotrophics
	// VkPhysicalDeviceFeatures supportedFeatures;
	// vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);


	// 5 formats for deferred rendering
	
	if(HANDLE_VALID(surface))
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(ids_phDevice, surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> 	formats;
		
		//~ vkGetPhysicalDeviceFormatProperties
		
		/*
		// грубый перебор известных форматов - для таблицы в консоль
		for (uint32_t f = VK_FORMAT_BEGIN_RANGE; f <= VK_FORMAT_END_RANGE; ++f) {
			VkFormat format = static_cast<VkFormat>(f);
			VkFormatProperties p;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &p);
			if (p.optimalTilingFeatures == 0) continue; // полностью бесполезный

			std::printf("%4d: sampled=%d attach=%d blend=%d linear=%d storage=%d\n",
				format,
				bool(p.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT),
				bool(p.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT),
				bool(p.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT),
				bool(p.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT),
				bool(p.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT));
		}
		*/
		
		if( formatCount != 0 ) 
		{
			formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(ids_phDevice, surface, &formatCount, formats.data());
		}
		
		printMessage("\nAvailable surface formats")
		for(auto f : formats)
		{
			printVariable(f.format)
		}
	}
	

	// 4 - if all OK, return true (queues supported AND extensions supported AND swapchain OK
	return indices.isComplete() && extensionsSupported && swapChainAdequate && featuresAvailable;// && supportedFeatures.samplerAnisotropy;
}



/*
 * 
 * name: Graphics::findQueueFamilies
 * @param: VkPhysicalDevice
 * @return: QueueFamilyIndices struct instance
 * Returns QueueFamilyIndices struct instance with indexes of required queue families (hardcoded in this struct)
 */
QueueFamilyIndices Graphics::findQueueFamilies(VkPhysicalDevice fqf_physDevice)
{
	// 1 - get properties of supported queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(fqf_physDevice, &queueFamilyCount, nullptr);				// check how many queue families are supported
	
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);							// create vector of enough capacity for queue properties
	vkGetPhysicalDeviceQueueFamilyProperties(fqf_physDevice, &queueFamilyCount, queueFamilies.data());	// save queue properties data
	
	
	// 2 - create empty QueueFamilyIndices struct instance with uninitialized optional variables (graphicsFamily и presentFamily)
	QueueFamilyIndices indices;	
	
	
	// 3 -  VkQueueFamilyProperties struct contains info about queue family, 
	// including types of supported operations and number of free available queues "to book". 
	// We need to find at least one queue family, which supports VK_QUEUE_GRAPHICS_BIT 
	// and one family more, which supports sending images to screen
	int i = 0;
	for (const auto& queueFamily : queueFamilies)					// iterate all queues
	{		
		// 3.1 - detect GRAPHICS family
		if( queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
		{
			// this IF make bitwise AND to check if the lowest bit (with index 0) in queueFamily.queueFlags is set to 1 (VK_QUEUE_GRAPHICS_BIT = 0x00000001)
			// if true, then this family is for graphics and then we save its index (i) as index of graphics family
			indices.graphicsFamily = i;
			
			//~ printVariable( queueFamily.queueFlags )
			//~ printVariable( queueFamily.queueCount )
			//~ printVariable( queueFamily.minImageTransferGranularity.width )
			//~ printVariable( queueFamily.minImageTransferGranularity.height )
			//~ printVariable( queueFamily.minImageTransferGranularity.depth )
		}

		// 3.2 - detect PRESENT family
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(fqf_physDevice, i, surface, &presentSupport);	// check if physical device supports presentation via created Surface
		if( presentSupport )
		{
			// if true, then this family (and also GPU) is for presenting and then we save its index (i) as index of graphics family
			indices.presentFamily = i;
		}

		// 3.3 - end
		if( indices.isComplete() ) // when both families already detected (isComplete() returns True), break loop, there is no more to search
		{
			break;
		}

		i++;
	}
	
	// Однако в дальнейшем мы будем искать и другие очереди, поэтому лучше поместить индексы в структуру
	
	//~ struct VkQueueFamilyProperties {
		//~ VkQueueFlags    queueFlags;								// uint32_t
		//~ uint32_t        queueCount;
		//~ uint32_t        timestampValidBits;
		//~ VkExtent3D      minImageTransferGranularity;			// detalization
	//~ }
	//~ struct VkExtent3D {
		//~ uint32_t    width;
		//~ uint32_t    height;
		//~ uint32_t    depth;
	//~ }

	
	// 4 - return filled QueueFamilyIndices struct instance
	return indices;
}



/*
 * 
 * name: Graphics::checkDeviceExtensionSupport
 * @param: VkPhysicalDevice
 * @return: bool
 * Returns True if device supports required exensions (stored in global deviceExtensions vector in fhRender.h)
 */
bool Graphics::checkDeviceExtensionSupport(VkPhysicalDevice cdes_phDevice)
{
	// 1
	// Необходимые расширения конкретного устройства прописаны в fhRender.h
	std::set<std::string> cdesRequiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );	//convert global required extensions vector to a Set

	#ifdef RENDER_1_DEBUG
		std::cout << "\nRequired device extensions:\n";		// optional console output
		for(auto re : cdesRequiredExtensions)	{
			std::cout << "\t" << re << std::endl;	}
	#endif
	
	
	// 2
	// Доступные расширения устройства получим вручную
	uint32_t cdesExtensionCount;
	vkEnumerateDeviceExtensionProperties(cdes_phDevice, nullptr, &cdesExtensionCount, nullptr);					// get the count of available extensions

	std::vector<VkExtensionProperties> cdesAvailableExtensions(cdesExtensionCount);								// // create vector of enough capacity for extensions properties
	vkEnumerateDeviceExtensionProperties(cdes_phDevice, nullptr, &cdesExtensionCount, cdesAvailableExtensions.data());	// save extensions data
	
	#ifdef RENDER_1_DEBUG
		std::cout << "\nAvailable device extensions:\n";	// optional console output
		for(auto ae : cdesAvailableExtensions)	{
			std::cout << "\t" << ae.extensionName << std::endl;	}
	#endif
		

	// 3
	// simply check if element of first vector also is member in second vector
	for (const auto& extension : cdesAvailableExtensions) 
	{
		//std::cout << '\t' << extension.extensionName << '\n';
		// erase() tries to delete current extension from list of cdesRequiredExtensions
		cdesRequiredExtensions.erase(extension.extensionName); // XXX возможно, стоит указать конец диапазона удаления, чтобы не очищалось вплоть до конца вектора?
	}
	
	// 4
	//if vector of required extensions is empty (empty()==true) - then all required extensions were detected in availables and excluded from there and device is ready
	// otherwise device is not suitable
	
	
	if( cdesRequiredExtensions.empty() )
	{
		#ifdef RENDER_1_DEBUG
			printMessage("All required device extensions available!")
		#endif
		return true;
	}
	else
	{
		return false;
	}
		 
}




bool Graphics::checkDeviceFeaturesSupport(VkPhysicalDevice cdfs_phDevice)
{
	// 1
	// Необходимые проекту фичи устройства версии 1_0 прописаны в mGpuFeatures
	const VkBool32* requiredFeatures10  = reinterpret_cast<const VkBool32*>(&mGpuFeatures);
	
	#ifdef RENDER_1_DEBUG
		printMessage("\nRequiered device features: (in order of their enumeration in VkPhysicalDeviceFeatures)")
		for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++i) {
			if( requiredFeatures10[i] ) {		std::cout << "\t" << i << std::endl;}		}
	#endif
	
	// 2
	// Поддерживаемые конкретной видеокартой cdfs_phDevice фичи получим вручную	
	VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures11{};
		indexingFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

	VkPhysicalDeviceFeatures2 features2{};
		features2.sType  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.pNext  = &indexingFeatures11;          // базовая + навесная в одной цепочке
		//features2.features = ...						// структура со всеми поддерживаемыми устройством фичами версии 1_0
	vkGetPhysicalDeviceFeatures2(cdfs_phDevice, &features2);	// запросим все доступные фичи
	
	// 2.1 поддерживаемые фичи версии 1_0
	const VkBool32* supportedFeatures10  = reinterpret_cast<const VkBool32*>(&features2.features);
	
	#ifdef RENDER_1_DEBUG
		printMessage("\nSupported advanced device features: ")
		for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++i) {
			if( supportedFeatures10[i] ) {		std::cout << "\t" << i << std::endl;}		}
	#endif
	
	// 2.2 поддерживаемых более новых фич VkPhysicalDeviceDescriptorIndexingFeatures всего 20 штук, можно сравнивать вручную с indexingFeatures
	

	// 3
	// Проверка поддержки требуемых фич
	bool featuresSupported = true;
	
	#ifdef RENDER_1_DEBUG
		printMessage("Supported required features:")
	#endif
	
	// 10 check
	for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); ++i) 
	{
		if( requiredFeatures10[i] ) 
		{		
			if( supportedFeatures10[i] ) 
			{
				#ifdef RENDER_1_DEBUG
					std::cout << "\t" << i << ": OK!" << std::endl;
				#endif
			}
			else
			{
				featuresSupported = false;
				printVarComment("At least not supported 1_0 feature: ", i)
				break;
			}
		}		
	}
	
	// 11+ check
	if( indexingFeatures11.shaderSampledImageArrayNonUniformIndexing != indexingFeatures.shaderSampledImageArrayNonUniformIndexing)
	{
		printVarComment("At least not supported 1_1+ feature: ", indexingFeatures11.shaderSampledImageArrayNonUniformIndexing)
		featuresSupported == false;
	}
	else
	{
		#ifdef RENDER_1_DEBUG
			std::cout << "\t .shaderSampledImageArrayNonUniformIndexing OK!" << std::endl;
		#endif
	}
	
	//system("pause");
		
	return featuresSupported;
}




/*
 * 
 * name: Graphics::querySwapChainSupport
 * @param: VkPhysicalDevice
 * @return: SwapChainSupportDetails struct
 * Returns struct with VkSurfaceCapabilitiesKHR, vector of VkSurfaceFormatKHR and vector of VkPresentModeKHR
 */
SwapChainSupportDetails Graphics::querySwapChainSupport(VkPhysicalDevice qscs_phDevice) 	// функция запрашивает информацию о цепочке показа, вспомогательная для создания Swapchain:
{
	if( !HANDLE_VALID(qscs_phDevice) )
	{
		DEBUG_INFO
		throw std::runtime_error("Error: cannot query swapchain support because of invalid VkPhysicalDevice");
	}
	
	// 1 - create empty struct
	SwapChainSupportDetails details;	
	// now query:		 
	
	// 2 - surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(qscs_phDevice, surface, &details.capabilities);

	// 3 - surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(qscs_phDevice, surface, &formatCount, nullptr);
	if( formatCount != 0 ) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(qscs_phDevice, surface, &formatCount, details.formats.data());
	}

	// 4 - presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(qscs_phDevice, surface, &presentModeCount, nullptr);
	if( presentModeCount != 0 ) 
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(qscs_phDevice, surface, &presentModeCount, details.presentModes.data());
	}

	// 5 - return filled struct for creating Swapchain
	return details;
}





