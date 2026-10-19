#include "fhRender.h"

#include "macros.h"

#include <set>
#include <algorithm>


//~ #define RENDER_1_DEBUG


/*
 * 
 * name: Graphics::createInstance
 * @param: none
 * @return: none
 * Just create a Vulkan instance
 */
void Graphics::createInstance() 
{
	
	
	if( enableValidationLayers && !checkValidationLayerSupport() ) 
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}
	
	VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "DemoLevel";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "fhEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 5);
		appInfo.apiVersion = VK_API_VERSION_1_0;
		
	VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;		
	
	std::vector<const char*> glfwExtensions = getRequiredExtensions();						// получили вектор строк с именами требуемых экземпляру расширений
		
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());	// занесли в структуру количество расширений на основании размера вектора
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();							// занесли в структуру имена самих расширений
				
	// depending on debug mode, specify validation layers count
	if (enableValidationLayers) 
	{
		createInfo.enabledLayerCount =	static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
	}
	
	// get extensions count	
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,nullptr);				
	
	// get these available extensions
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);							// make vector of enough capacity
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());	// get Vulkan extensions names
	
	// make an independent vector of just available extension names
	std::vector<const char*> availableExtensionsNames(extensionCount);
	
	for(int i=0; i<extensionCount; i++)
	{
		availableExtensionsNames[i] = availableExtensions[i].extensionName;							// iterative copying names
	}

	// optional console output
	#ifdef RENDER_1_DEBUG
		std::cout << "\nAvailable instance extensions:\n";
		for (const auto& extension : availableExtensions) // перебор доступных
		{
			std::cout << '\t' << extension.extensionName << '\n';			
		}	
	#endif
	
	if( hasCommonStrings(glfwExtensions, availableExtensionsNames) )	// if there are required extensions
	{
		// create Vulkan API instance with required extensions and simultaneously check return code
		checkResult( vkCreateInstance(&createInfo, nullptr, &instance) );
	}
	else
	{
		DEBUG_INFO
		throw std::runtime_error("There are no required extensions! Failed to create Vulkan instance! Try to recompile with #define RENDER_1_DEBUG");
	}
	
}

void Graphics::destroyInstance()
{
	vkDestroyInstance(instance, nullptr);
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
	
	if(surface == VK_NULL_HANDLE)
	{
		VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
		
		if ( result != VK_SUCCESS) 
		{
			checkResult(result);
			DEBUG_INFO
			throw std::runtime_error("Failed to create window surface!");
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
	// 1 - figure out the number of all available physical devices
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);				

	if (deviceCount == 0) 
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}


	// 2 - save info about all detected physical devices
	std::vector<VkPhysicalDevice> devices(deviceCount);					// make vector of enough capacity
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());	

	// optional console output
	#ifdef RENDER_1_DEBUG
		printMessage("\nPhysical devices and info: ")       
		for( auto dev : devices)
		{	
			// VkPhysicalDeviceType 1->VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, 2->VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, 4->VK_PHYSICAL_DEVICE_TYPE_CPU
					
			VkPhysicalDeviceProperties deviceProperties;				// info struct
			vkGetPhysicalDeviceProperties(dev, &deviceProperties);

			std::cout << "\n\tDevice Name: " << deviceProperties.deviceName << std::endl;
			std::cout << "\tDevice Type: ";
			switch(deviceProperties.deviceType)
			{
				case VK_PHYSICAL_DEVICE_TYPE_OTHER: printMessage("unknown other") break;			//0
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: printMessage("integrated") break;		//1
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: printMessage("discrete") break;			//2
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: printMessage("virtual") break;			//3
				case VK_PHYSICAL_DEVICE_TYPE_CPU: printMessage("CPU") break;						//4
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


	// 3 - now select the most suitable device. We can decide on simply by name/type or (if there are no choices) estimate GPU "score points" and pick the "winner"
	for (const auto& device : devices) 
	{
		if( isDeviceSuitable(device) ) 
		{
			physicalDevice = device;								// as soon as found suitable device, save its descriptor and break loop
			break;													
		}
	}
	
	if (physicalDevice == VK_NULL_HANDLE) 							// if there are no suitable devices, we have no chance(
	{
		DEBUG_INFO
		throw std::runtime_error("Failed to find a suitable GPU!");	// 
	}
	
	
	// 4 - query properties and features of selected GPU
	vkGetPhysicalDeviceProperties(physicalDevice, &mGpuProperties); // again like in p.2 ↑↑↑
	vkGetPhysicalDeviceFeatures(physicalDevice, &mGpuFeatures);		// 
	
		//~ mGpuFeatures.geometryShader = VK_TRUE;
	
	// 5 - optional console output about selected physical device
	#ifdef RENDER_1_DEBUG
		std::cout << "\n\nSelected Physical Device Name: " << mGpuProperties.deviceName << std::endl;
		std::cout << "\tDevice Type: ";
		switch(mGpuProperties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_OTHER: printMessage("unknown other") break;	//0
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: printMessage("integrated") break;		//1
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: printMessage("discrete") break;			//2
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: printMessage("virtual") break;			//3
			case VK_PHYSICAL_DEVICE_TYPE_CPU: printMessage("CPU") break;			//3
		}		
		std::cout << "\tDriver Version: " << mGpuProperties.driverVersion << std::endl;
		std::cout << "\tAPI Version: " 	<< VK_VERSION_MAJOR(mGpuProperties.apiVersion) 
										<< "." << VK_VERSION_MINOR(mGpuProperties.apiVersion) 
										<< "." << VK_VERSION_PATCH(mGpuProperties.apiVersion) << std::endl;
	#endif
	
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
	VkPhysicalDeviceFeatures deviceFeatures{};						// пока пусто?
		deviceFeatures.geometryShader = VK_TRUE;

	// 5 - прописываем информацию о создаваемом логическом устройстве
	VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		// 5.1 - резервируемые очереди
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());		// количество резервируемых очередей
		createInfo.pQueueCreateInfos = queueCreateInfos.data();									// занесли инфу об очередях

		// 5.2 - фичи
		createInfo.pEnabledFeatures = &deviceFeatures;											

		// 5.3 - требуемые расширения
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());		// количество
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();							// инфа
		
		//~ // 5.4 - слои валидации
		//~ if (enableValidationLayers) 
		//~ {
			//~ createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			//~ createInfo.ppEnabledLayerNames = validationLayers.data();
		//~ } 
		//~ else 
		//~ {
			//~ createInfo.enabledLayerCount = 0;
		//~ }


	// 6 - теперь непосредственно создаем главное логическое устройство, а очереди зарезервируются автоматически
	VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice);
	
	if (result != VK_SUCCESS) 
	{
		checkResult(result);
		throw std::runtime_error("failed to create logical logicalDevice!");
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
	
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;																// массив строк с именами расширений
	
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);					// записали в массив строки с именами требуемых расширений, заодно по ссылке записав их количество

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);	// конструктор принимает 2 итератора (начало и конец) и т.о. копирует в вектор содержимое массива строк имен

	#ifdef RENDER_1_DEBUG
		printMessage("\nRequired instance extensions:")
		for(auto requiredExtension : extensions)
		{
			std::cout << "\t" << requiredExtension << "\n";
		}
	#endif

	if (enableValidationLayers) 																// если слои валидации включены, добавим еще и расширение "VK_EXT_debug_utils"
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);								// "VK_EXT_debug_utils"
	}

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
		printMessage("\nCommon strings: ")
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

	// for anisotrophics
	// VkPhysicalDeviceFeatures supportedFeatures;
	// vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);


	// 4 - if all OK, return true (queues supported AND extensions supported AND swapchain OK
	return indices.isComplete() && extensionsSupported && swapChainAdequate;// && supportedFeatures.samplerAnisotropy;
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
	// 1 - get properties of supported extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(cdes_phDevice, nullptr, &extensionCount, nullptr);					// get the count of available extensions

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);								// // create vector of enough capacity for extensions properties
	vkEnumerateDeviceExtensionProperties(cdes_phDevice, nullptr, &extensionCount, availableExtensions.data());	// save extensions data
	
	// optional console output
	#ifdef RENDER_1_DEBUG
		std::cout << "\nAvailable device extensions:\n";
		for(auto ae : availableExtensions)
		{
			printVarComment("\t", ae.extensionName)
		}
	#endif
	
	// 2 - convert global required extensions vector to a Set
	std::set<std::string> requiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );

	// optional console output
	#ifdef RENDER_1_DEBUG
		std::cout << "\nRequired device extensions:\n";
		for(auto re : requiredExtensions)
		{
			printVarComment("\t", re)
		}
	#endif

	// 3 - simply check if element of first vector also is member in second vector
	for (const auto& extension : availableExtensions) 
	{
		//std::cout << '\t' << extension.extensionName << '\n';
		// erase() tries to delete current extension from list of requiredExtensions
		requiredExtensions.erase(extension.extensionName); // XXX возможно, стоит указать конец диапазона удаления, чтобы не очищалось впоть до конца вектора?
	}
	
	// 4 - if vector of required extensions is empty (empty()==true) - then all required extensions were detected in availables and excluded from there and device is ready
	// otherwise device is not suitable
	return requiredExtensions.empty();
	 
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





