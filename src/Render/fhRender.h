#pragma once

#include "auxiliary.h"

#include "globals.h"
#include "fhStructs.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Window_API_GLFW.h" // GLFW-class window, input

#include <array>

class Mesh;
class VkTexture;
class Material;


const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


// список необходимых расширений устройства
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif



class Graphics : GLFW
{
public:

	resManAsset<Mesh>		Meshes;

	resManAsset<VkTexture>	VkTextures;

	resManAsset<Material>	Materials;

	// 1 - Vulkan Core:
	VkInstance 					instance 		{ VK_NULL_HANDLE };
    
    VkPhysicalDevice 			physicalDevice 	{ VK_NULL_HANDLE };		// не требует уничтожения, так как это лишь указатель. Уничтожается вместе с instance
    VkPhysicalDeviceProperties 	mGpuProperties 	= {};
    VkPhysicalDeviceFeatures	mGpuFeatures 	= {};
    
    VkDevice 					logicalDevice 	{ VK_NULL_HANDLE };		// логическое устройство - основная переменная. 
    VkQueue 					graphicsQueue 	{ VK_NULL_HANDLE };							// дескриптор зарезервированной графической очереди. Уничтожается автоматически вместе с логическим устройством
    VkQueue 					presentQueue 	{ VK_NULL_HANDLE };							// дескриптор зарезервированной презентационной очереди
		
    
    
    
    // 2 - Visual output: Для возможности выводить результат работы на экран, надо дополнительно настроить:
    //		-связь Vulkan с существующими системами отображения - Windows, Linux и т.д.
    VkSurfaceKHR 				surface			 { VK_NULL_HANDLE };	// мультиплатформенная оболочка окна. Подключаемое расширение для возможности показывать на экране

    //		-механизм чередования кадров, отображаемых на экране
    VkSwapchainKHR 				swapChain		 { VK_NULL_HANDLE };	// infrastructure for owning and sequence images (rendertargets)
    VkFormat 					swapChainImageFormat;					// VK_FORMAT_B8G8R8A8_SRGB
    VkExtent2D 					swapChainExtent;						// sizes: width, height
    std::vector<VkImage> 		swapChainImages;						// host-side handles of swapchain images. Created and destroyed simultaneously with swapchain
    std::vector<VkImageView> 	swapChainImageViews;					// shells, specify image format, mip levels, layers etc. Make swapchain images available to be used with renderpass. Created and destroyed manually!
    std::vector<VkFramebuffer> 	swapChainFramebuffers;					// objects for binding VkImageViews to specified renderpass attachments (render target)
    
    //		-механизмы рендеринга
    
    
    
    // deferred G-pass
    VkRenderPass				renderPass_G	 		{ VK_NULL_HANDLE };
    VkFramebuffer				frameBuffer_G			{ VK_NULL_HANDLE };
    VkDescriptorSetLayout 		descriptorSetLayout_G 	{ VK_NULL_HANDLE }; 		// описание "портов" шейдера - т.е. точек подключения/привязки неких ресурсов. Для возможности подключать к шейдеру ресурсы (буферы/семплеры) обязательно нужен VkDescriptorSetLayout, даже если де-факто ресурсов не предвидится
    VkDescriptorPool 			descriptorPool_G 		{ VK_NULL_HANDLE };			// объект, выделяющий и владеющий местом для descriptor sets? пул дескрипторов для шейдерных переменных - один. Надо удалять вручную
    VkPipelineLayout 			pipelineLayout_G 		{ VK_NULL_HANDLE };			// обобщенное описание (доступа конвейера к) ЛЮБЫХ передаваемых в шейдеры переменных - ресурсов (descriptorSetLayout) и чисел (push-констант), которые могут быть обновлены во время отрисовки. pipeline layout используется при "жизни" программы, поэтому надо вручную удалить в конце
    VkPipeline 					pipeline_G  			{ VK_NULL_HANDLE };			// может ссылаться на descriptorSetLayout. Это "рецепт" генерации кадра: какие этапы + каким рендерпасом + какие могут быть точки подключения
    
    static constexpr int gPassAttachmentsCount = 6;									// G_PASS_ATTACHMENTS количество выходных прикреплений к G-пассу
    
    std::array<VkImage, 		gPassAttachmentsCount> gPassImage;
    std::array<VkDeviceMemory , gPassAttachmentsCount> gPassImageMemory;
    std::array<VkImageView, 	gPassAttachmentsCount> gPassImageView;
    std::array<VkSampler, 		gPassAttachmentsCount> gPassImageSampler;
        
    void createGpass();
    void destroyGpass();
    
    //FhGeneralInfo buffer creation start
		VkBuffer 		 	GpassGeneralInfoBuffer			 { VK_NULL_HANDLE };		// host-side хэндлер буфера для переменных (матриц)		
		VkDeviceMemory  	GpassGeneralInfoBufferMemory	 { VK_NULL_HANDLE };		// кусок видеопамяти для буфера переменных
		void* 			 	GpassGeneralInfoBufferMapped;								// адрес видеопамяти для прямого доступа из функции updateEntityUniformBuffer() и т.п.
		
		void createGpassGeneralInfoBuffer(); // создание в видеопамяти буфера для шейдерных переменных и "прокидывание" прямого доступа к нему
		void destroyGpassGeneralInfoBuffer();
		void updateGpassGeneralInfoBuffer();
    //FhGeneralInfo buffer creation end
    // deferred G-pass end
    
    
    // Fullscreen quad start
		std::vector<FhFullscreenQuadVertex> 	fullscreenQuadVertexDataVector;	
		VkBuffer 								fullscreenQuadVertexBuffer		 { VK_NULL_HANDLE }; 	// host-side vertex buffer data handler
		VkDeviceMemory 							fullscreenQuadVertexBufferMemory { VK_NULL_HANDLE };	// GPU memory reserved for buffer
		void createFullscreenQuad();
		void destroyFullscreenQuad();
    // Fullscreen quad end
    
      
    
    // deferred L-pass start
    VkRenderPass				renderPass_L				{ VK_NULL_HANDLE };
    VkFramebuffer				frameBuffer_L				{ VK_NULL_HANDLE };
    VkDescriptorSetLayout 		descriptorSetLayout_L		{ VK_NULL_HANDLE }; 	// описание "портов" шейдера - т.е. точек подключения/привязки неких ресурсов. Для возможности подключать к шейдеру ресурсы (буферы/семплеры) обязательно нужен VkDescriptorSetLayout, даже если де-факто ресурсов не предвидится
    VkDescriptorPool 			descriptorPool_L			{ VK_NULL_HANDLE };		// объект, выделяющий и владеющий местом для descriptor sets? пул дескрипторов для шейдерных переменных - один. Надо удалять вручную
    VkPipelineLayout 			pipelineLayout_L			{ VK_NULL_HANDLE };		// обобщенное описание (доступа конвейера к) ЛЮБЫХ передаваемых в шейдеры переменных - ресурсов (descriptorSetLayout) и чисел (push-констант), которые могут быть обновлены во время отрисовки. pipeline layout используется при "жизни" программы, поэтому надо вручную удалить в конце
    VkPipeline 					pipeline_L					{ VK_NULL_HANDLE };
    VkDescriptorSet				descriptorSet_L				{ VK_NULL_HANDLE };
    
    static constexpr int lPassAttachmentsCount = 2;									// L_PASS_ATTACHMENTS количество выходных прикреплений к L-пассу
    
    std::array<VkImage, 		lPassAttachmentsCount> lPassImage;
    std::array<VkDeviceMemory , lPassAttachmentsCount> lPassImageMemory;
    std::array<VkImageView, 	lPassAttachmentsCount> lPassImageView;
    std::array<VkSampler, 		lPassAttachmentsCount> lPassImageSampler;
    
    void createLpass();
    void destroyLpass();
    
    //FhDynamicLightSourceInfo buffer creation start
		VkBuffer 		 	LpassDynamicLightSourceInfoBuffer			 { VK_NULL_HANDLE };		// host-side хэндлер буфера для переменных (матриц)		
		VkDeviceMemory  	LpassDynamicLightSourceInfoBufferMemory		 { VK_NULL_HANDLE };		// кусок видеопамяти для буфера переменных
		void* 			 	LpassDynamicLightSourceInfoBufferMapped;					// адрес видеопамяти для прямого доступа из функции updateEntityUniformBuffer() и т.п.
		
		void createLpassDynamicLightSourceInfoBuffer(); // создание в видеопамяти буфера для шейдерных переменных и "прокидывание" прямого доступа к нему
		void destroyLpassDynamicLightSourceInfoBuffer();
		void updateLpassDynamicLightSourceInfoBuffer();	// call in every frame or when illumination data has changed
    //FhDynamicLightSourceInfo buffer creation end
    // deferred L-pass end
    
    
    
    
    // Final-pass start. Because of double buferization need MAX_FRAMES_IN_FLIGHT pieces of something
    VkRenderPass				renderPass_F					{ VK_NULL_HANDLE };
    std::vector<VkFramebuffer>	frameBuffer_F					{ VK_NULL_HANDLE };
    VkDescriptorSetLayout 		descriptorSetLayout_F			{ VK_NULL_HANDLE }; 					// описание "портов" шейдера - т.е. точек подключения/привязки неких ресурсов. Для возможности подключать к шейдеру ресурсы (буферы/семплеры) обязательно нужен VkDescriptorSetLayout, даже если де-факто ресурсов не предвидится
    VkDescriptorPool 			descriptorPool_F				{ VK_NULL_HANDLE };							// объект, выделяющий и владеющий местом для descriptor sets? пул дескрипторов для шейдерных переменных - один. Надо удалять вручную
    VkPipelineLayout 			pipelineLayout_F				{ VK_NULL_HANDLE };							// обобщенное описание (доступа конвейера к) ЛЮБЫХ передаваемых в шейдеры переменных - ресурсов (descriptorSetLayout) и чисел (push-констант), которые могут быть обновлены во время отрисовки. pipeline layout используется при "жизни" программы, поэтому надо вручную удалить в конце
    VkPipeline 					pipeline_F						{ VK_NULL_HANDLE };			// может ссылаться на descriptorSetLayout. Это "рецепт" генерации кадра: какие этапы + каким рендерпасом + какие могут быть точки подключения
    std::vector<VkDescriptorSet> descriptorSet_F;
    
    void createFpass();
    void destroyFpass();
    
    // Final-pass end
    
    
    
    
    
    // Самая важная связь такая: render pass не “рисует в swapchain” напрямую, он рисует в framebuffer, 
    // а framebuffer уже указывает на image view swapchain image. Поэтому обычно ты создаёшь отдельный framebuffer на каждый swapchain image.
    
      

	//~ Pipeline layout нужен для создания Pipeline:
	// Через pipeline layout осуществляется доступ к descriptor sets из graphicsPipeline.
	// Ноль+ descriptor set layouts и ноль+ push constant ranges скомбинированы в единый pipeline layout, описывающий полный набор ресурсов, доступных конвейеру.
	// Pipeline layout представляет последовательность descriptor set, каждый из которых на своем месте.
	// Эта последовательность используется для определения интерфейса между шейдерами и ресурсами.
	
    //		-механизмы для синхронизации и обработки команд рендеринга (семафоры и заборы) занесены в структуру FrameData
    uint32_t 					currentFrame = 0;						// индекс обрабатываемого кадра
	FrameData 					_frames[MAX_FRAMES_IN_FLIGHT];			// массив структур с дескрипторами семафоров, заборов, командных пулов и командных буферов для каждого кадра


    
    // depth. We only need a single depth image, because only one draw operation is running at once. 
/*
Почему depth image может быть один
Depth buffer нужен не для презентации, а для теста глубины при рендере.
Если у тебя в каждый момент времени реально выполняется только один render pass на GPU в рамках одного depth attachment, 
то одного depth image достаточно, и его можно использовать во всех framebuffer-ах.

*/

    
    //~ VkImage 					depthImage;
	//~ VkDeviceMemory 				depthImageMemory;
	//~ VkImageView 				depthImageView;
	
	//~ VkBuffer					lightsUniformBuffers;							// host-side хэндлер буфера для переменных (матриц)		
	//~ VkDeviceMemory				lightsUniformBuffersMemory;					// кусок видеопамяти для буфера переменных
	//~ void*						lightsUniformBuffersMapped;					// адрес видеопамяти для прямого доступа из функции updateEntityUniformBuffer() и т.п.
	
	//~ void createLightsUniformBuffers();
    //~ void updateLightsUniformBuffer();

    //~ void destroyLightsUniformBuffers();
	
    
    // Вспомогательное
    bool flagE = true;
    
    
    // абстрактная функция для удобного создания host-side буфера и связывания его с выделенной видеопамятью
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	
    void checkResult( VkResult result );
    
    
    
    




    void run();
    
    //---------------------------------Методы---------------------------
    void initVulkan();
    
    void createDeferredPipeline();

    
    void mainLoop();
    void cleanup();
    
    void spawns();

    
    // fhRender_2.cpp
    void createInstance();
	void destroyInstance();
	void createSurface();
	void destroySurface();		// уничтожение экранной поверхности 
    void pickPhysicalDevice();
    void createLogicalDevice(); // После выбора физического устройства необходимо создать логическое устройство. 
	void destroyLogicalDevice();	// уничтожение логического устройства
	
	// fhRender_3.cpp
	void createSwapChain();
	void destroySwapChain();
    void getSwapchainImages();
    void createSwapchainImageViews(); 
    void destroySwapchainImageViews();
    void cleanupSwapChain();
    void recreateSwapChain();
    
    
    
	
	VkShaderModule createShaderModule(const std::vector<char>& code);	// создать из присланного байт-кода шейдерный модуль
    
    
    
    
        
    void createCommandPool(); 		// создание "склада" заготовок
    
    void destroyCommandPool();
	
	// создание в видеопамяти буфера с вершинными данными треугольника и сохранение дексриптора на этот буфер
	void createVertexBuffer(const std::vector<Vertex2>& datas, VkBuffer& vbo, VkDeviceMemory& vbomem);

    void destroyVertexBuffer(VkBuffer& vbo, VkDeviceMemory& vbomem);
    	
	//~ void destroyUniformBuffers(std::string entityName); 
	

    void createCommandBuffer();
    
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex); 
     
    void createSyncObjects();
    
    void destroySyncObjects();
	
	
	// 		абстрактная функция для удобного многократного создания изображений
	void createImage(	uint32_t width, 
						uint32_t height, 
						VkFormat format, 
						VkImageTiling tiling, 
						VkImageUsageFlags usage, 
						VkMemoryPropertyFlags properties, 
						VkImage& image, 
						VkDeviceMemory& imageMemory);
	
		
	// 5 функций для создания изображения как области в видеопамяти с привязанным host-side дескриптором
	void createTextureImage(std::string filename, VkImage& image, VkDeviceMemory& imageMemory);
		
	
	//		функция для переноса пиксельных данных из видимого промежуточного буфера в нужную область видеопамяти
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	//		функция для изменения расположения текстуры
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	
	void destroyTextureImage(VkImage& image, VkDeviceMemory& imageMemory);
	
    
    
    // 3 функции для создания обертки над загруженной текстурой
    void createTextureImageView(VkImage& image, VkImageView& imageView );
    
	//		абстрактная функция для удобного создания оберток для кусков видеопамяти.
	//		используется при создании изображений текстур и изображений из swapchain
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlag);
    
    void destroyTextureImageView( VkImageView& imageView );
	
       
    //		создание текстурного сэмплера
    void createTextureSampler( VkSampler& sampler );
    
	void destroyTextureSampler( VkSampler& sampler );
	
    
    
    
        
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    
    VkFormat findDepthFormat();
    
    bool hasStencilComponent(VkFormat format);
    
    
    bool UPD = true;
    
    
    void drawFrame();
    


	std::vector<const char*> getRequiredExtensions();					// запрос требуемых расширений instance`а
    
	bool checkValidationLayerSupport(); 
	
	
	
	// -----------------------------------------------------------------------
	// Четыре функции ниже - для проверки применимости VkPhysicalDevice
	bool isDeviceSuitable(VkPhysicalDevice device); 	// эта функция проверяет, подходит ли VkPhysicalDevice под заданные разработчиком критерии
    
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physDevice);		// функция индексирует требуемые семейств очередей
    
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);				// функция проверяет, поддерживает ли VkPhysicalDevice нужные расширения
    
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device); 	// функция запрашивает информацию о цепочке показа, вспомогательная для создания Swapchain:
    
	
	// -----------------------------------------------------------------------
	// Три функции ниже - для настройка Swapchain
	// определение формата
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    
	// определение режима отображения (синхронизации с обновлением экрана?)
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    
	// определение размера изображения
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    

	
	// -----------------------------------------------------------------------
	// вспомогательные C++ функции
	bool hasCommonStrings(const std::vector<const char*> &v1, const std::vector<const char*> &v2);
	
	static std::vector<char> readFile(const std::string& filename);
    
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);	// копирование данных между буферами
	
	void updateUniformBuffer(uint32_t currentImage, std::string entityName);// = "test")	//int objIndex , bool rotate = false,
	
	VkCommandBuffer beginSingleTimeCommands();
    
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    
	//
};


//~ class Game: Graphics
//~ {
//~ public:

	

	//~ Game();
//~ };


