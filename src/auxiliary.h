#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <vector>
#include <array>
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>



//~ extern bool _key[350];	// массив для состояний клавиш



//~ class MicroWorld;
//~ class Camera;
//~ class Scena;

//~ class Graphics;

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physDevice);


// в этой структуре хранятся индексы всех требуемых программе семейств очередей: графических и т.д.



	
	
struct QueueFamilyIndices 
{
	/*
		This struct must contain indexes of both needed (FOR THIS 3D-ENGINE) queue families: 
			Graphics Family - for drawing, rasterization, illumination, shaders etc
			Present Family - for sending complete images to display
		
		But there can be NO required Queue Familys (for example, bad/wrecked/wrong GPU).
		So, to avoid magical Zero-values, use std::optional and add isComplete() member to flag Success if there are both required Queue Families.
	*/
	
    std::optional<uint32_t> graphicsFamily; // Основное семейство очередей для обработки графики, включая рисование вершин, растеризацию, освещение и обработку шейдеров.
    std::optional<uint32_t> presentFamily; // Специальное семейство очередей для отправки готовых кадров на поверхность дисплея (монитор).

    bool isComplete() 
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// для создания Swapchain нужны 3 набора дополнительных данных
struct SwapChainSupportDetails 
{
    // 1-й
    VkSurfaceCapabilitiesKHR 			capabilities;		
   
	//~ struct VkSurfaceCapabilitiesKHR {
		//~ uint32_t                         minImageCount;
		//~ uint32_t                         maxImageCount;
		//~ VkExtent2D                       currentExtent;
		//~ VkExtent2D                       minImageExtent;
		//~ VkExtent2D                       maxImageExtent;
		//~ uint32_t                         maxImageArrayLayers;
		//~ VkSurfaceTransformFlagsKHR       supportedTransforms;
		//~ VkSurfaceTransformFlagBitsKHR    currentTransform;
		//~ VkCompositeAlphaFlagsKHR         supportedCompositeAlpha;
		//~ VkImageUsageFlags                supportedUsageFlags;
	//~ } VkSurfaceCapabilitiesKHR;


	// 2-й
    std::vector<VkSurfaceFormatKHR> 	formats;	// формат цвета, например, R8G8B8A8 non-linear sRGB 
    //~ struct VkSurfaceFormatKHR {
		//~ VkFormat           format;
		//~ VkColorSpaceKHR    colorSpace;
	//~ } VkSurfaceFormatKHR;
    
    
    // 3-й
    std::vector<VkPresentModeKHR> 		presentModes;	// режим передачи изображений на отображение
    
	//~ enum VkPresentModeKHR {
		//~ VK_PRESENT_MODE_IMMEDIATE_KHR = 0,
		//~ VK_PRESENT_MODE_MAILBOX_KHR = 1,
		//~ VK_PRESENT_MODE_FIFO_KHR = 2,
		//~ VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,						// это вроде как самый предпочтительный на начальных этапах
		//~ VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR = 1000111000,
		//~ VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR = 1000111001,
		//~ VK_PRESENT_MODE_FIFO_LATEST_READY_EXT = 1000361000,
		//~ VK_PRESENT_MODE_MAX_ENUM_KHR = 0x7FFFFFFF
	//~ } VkPresentModeKHR;
    	
};


// Описание формата хранения данных для одной вершины геометрической фигуры
struct Vertex 
{
	// структура хранит данные для 1 вершины. В памяти эти данные расположены подряд, т.е. 20 байт, из которых 8 это для двух координат и 12 для трех цветов
	// методы не занимают места, т.к. не являются непосредственно частью объекта, т.к. это инструкции, а не поля данных

	glm::vec3 pos;			// первый атрибут
	glm::vec3 color;		// второй атрибут
	glm::vec2 texCoord;
	
	// Binding - всего один, т.к. привязывается один буфер
	static VkVertexInputBindingDescription getBindingDescription() 
	{
		// A vertex binding describes at which rate to load data from memory throughout the vertices. 
		// It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance 
	
		// разбиение буфера вершинных данных по блокам (вершинам):
		VkVertexInputBindingDescription bindingDescription{};
		//All of our per-vertex data is packed together in one array, so we’re only going to have one binding, поэтому индекс будет 0
			bindingDescription.binding = 0;								// The binding parameter specifies the index of the binding in the array of bindings
			bindingDescription.stride = sizeof(Vertex);					// смещение между блоками (вершинами) в байтах
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	// Move to the next data entry after each vertex
		
		return bindingDescription;	
	}
	
	// А вот атрибутов может быть много
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() 
	{
		// разбиение блока одной конкретной вершины из буфера вершинных данных на составляющие его атрибуты и описание, как их вычленять:
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{}; // пустой массив строго фиксированного размера
					
			// описание первого атрибута - position attribute (0)
			attributeDescriptions[0].binding = 0;							// из какого биндинга идут данные о вершине
			attributeDescriptions[0].location = 0;							// индекс входной location-директивы из вершинного шейдера. (layout(location = 0) in vec2 inPosition;)
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// тип данных - vec3 XYZ
			attributeDescriptions[0].offset = offsetof(Vertex, pos);		// макрос для автоматического вычисления смещения атрибута в массиве

			// описание второго атрибута - color attribute (1)
			attributeDescriptions[1].binding = 0;							// из какого биндинга идут данные о вершине
			attributeDescriptions[1].location = 1;							// индекс входной location-директивы из вершинного шейдера. (layout(location = 1) in vec3 inColor;)
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;	// тип данных - vec3 RGB
			attributeDescriptions[1].offset = offsetof(Vertex, color);		// макрос для автоматического вычисления смещения атрибута в массиве

			// описание третьего атрибута - texCoords (2)
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		// XXX индекс в массиве и location могут или обязаны совпадать?
        return attributeDescriptions;
    }

};


class Stopwatch			// класс для счетчика времени. Обрабатывает переменную timeSinceLastFrame - время в мс между кадрами
{
public:
	Stopwatch();

	Stopwatch(const std::string& className);

	~Stopwatch();

	void updateTick();	// обязана вызываться каждый цикл в Core::mainLoop()!
	
	float getTick();
	
	static double timeSinceLastFrame;	// для статической не нужно указывать extern double timeSinceLastFrame = 0; Можно сразу определение в fhEngine.cpp

private:
	double timeOfLastFrame = 0;

	double timeOfInstanceCreation = 0;
	double timeOfInstanceDestruction = 0;

	std::string name;
};



std::string getValue(const std::string& strSource, int num); //возврат слова с порядковым номером num из строки strSource

std::string getValueOrNone(std::string& strSource, int num); //возврат слова с порядковым номером num из строки strSource. используется для загрузки скелета

char* 		StringToChar(std::string StringValue);	// XXX переделать с аргументом по ссылке


//extern double Stopwatch::timeSinceLastFrame = 0; // первичная инициализация СТАТИЧЕСКОЙ переменной счетчика времени между кадрами


//~ extern std::shared_ptr<Scena> g_SceneMicromir;
//~ extern std::shared_ptr<Camera> CAMERA;

//~ extern Graphics app;

//~ class MicroWorld;
//~ class Entity;


//~ template <typename P>
//~ bool CHECK_PTR(P pointer);

//~ template <typename P, typename N>
//~ bool CHECK_PTR(P<N> pointer);
