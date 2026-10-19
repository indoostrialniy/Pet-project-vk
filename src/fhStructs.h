#pragma once

#include <string>
#include <memory>
#include <vector>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "fhEnumerations.h"

class Graphics;



struct FhEntityInitializationInfo
{
public:
	std::string initSysname; 											// системное имя, уникальное в пределах актора
	
	std::string initMeshName; 											// model filename
	
	glm::vec3 	initPos			= glm::vec3(0.0f, 0.0f, 0.0f); 			// локальные координаты сущности внутри актора
	
	glm::quat 	initRot			= glm::quat(1.0f, 0.0f, 0.0f, 0.0f);	// локальное вращение сущности внутри актора
	
	glm::vec3 	initScale		= glm::vec3(1.0f, 1.0f, 1.0f);
	
	uint32_t 	initType 		= FH_ENTITY_TYPE_RENDER_OPAQUE;
	
	std::vector <std::string> 	initMeshMaterialsVector;				// вектор текстовых имен назначенных материалов
};



struct FhDynamicLightSourceInfo //  6 версия
{
				glm::vec3 position	= glm::vec3(0,0,0);             			// Вектор: 0 1 2 3   4 5 6 7   8 9 10 11
    alignas(16) glm::vec3 direction	= glm::vec3(1,0,0);            				// 12 13 14 15 - байты смещения. Вектор: 16 17 18 19   20 21 22 23   24 25 26 27
    alignas(16) glm::vec3 color		= glm::vec3(0.5,0.98,0.34);             	// 28 29 30 31 - байты смещения. Вектор: 32 33 34 35   36 37 38 39   40 41 42 43

				float radius		= 10.0f;                   					// 	Адреса: 44 45 46 47	
				float power			= 10.0f;                    				// 	Адреса: 48 49 50 51
				float sector		= 90.0f;									//	Адреса: 52 53 54 55
				float blendWidth	= 10.0f;									//	Адреса: 56 57 58 59
				uint32_t type		= FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;	//  Адреса: 60 61 62 63
				uint32_t exists		= 0; 										//	Адреса: 64 65 66 67
				                  
	alignas(16) glm::mat4 PV[6]; 												// 	Адреса: 80 - 463
								// ↑↑↑ уже 464 байт, надо добить до 512, чтоб было кратно 64 (лимит Вулкана), добавим 12*4=48 байт
			float reserve[12];	// но в GLSL, согласно std140, если float идет массивом, то на одно значение выделяется 16 байт (vec4), 
								// поэтому в шейдере хвост надо делать в 4 раза меньше: float reserve[3]; !!!!
};
static_assert(sizeof(FhDynamicLightSourceInfo) == 512, "FhDynamicLightSourceInfo must be exactly 512 bytes");



struct FhMicroWorldInitializationInfo : public FhDynamicLightSourceInfo	// для инициализации обычного актора, но также и источника света
{
public:
	FhMicroWorldInitializationInfo(Graphics& core) : vulkanCore(core) {}
	
	std::string name;
	
	glm::vec3 	initPos			= glm::vec3(0.0f, 0.0f, 0.0f); 			// мировые координаты актора
	
	glm::quat 	initRot			= glm::quat(1.0f, 0.0f, 0.0f, 0.0f);	// мировое вращение актора
	
	uint32_t	index			= 0;
	
	Graphics& vulkanCore;												// ссылка на действующий класс vulkan-ядра
};



// The data in the matrices is binary compatible with the way the shader expects it, 
// so we can later just use memcpy-command to copy a this UniformBufferObject-struct data to a VkBuffer
struct FhGeneralInfo	// common data for G-pass. (former UniformBufferObject)
{
	// обращать внимание на выравнивание!
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 cameraPos;
	float Time;				// game time in seconds
	//alignas(16) 
};



struct FhGpassPushConstantsInfo // per-entity personal data for G-pass
{
	glm::mat4 model;				// entity model matrix
	uint32_t type;
	uint32_t reserve1;
	float reserve2;
	float reserve3;
};



struct Vertex2 	// описание одной вершины геометрической модели
{
	// структура хранит данные для 1 вершины. В памяти эти данные расположены подряд, т.е. 36 байт, из которых 12 это для трех координат, 12 для нормалей и 12 для текстурных координат и индекса материала
	// методы не занимают места, т.к. не являются непосредственно частью объекта, т.к. это инструкции, а не поля данных

	glm::vec3 	pos;			// первый атрибут - position
	glm::vec3 	normal;		// второй атрибут - normal
	glm::vec2 	texCoord; 	// UV +  material index
	uint32_t 	materialIndex;	// material index
	glm::vec3	tangent;		
	
	
	// Binding - всего один, т.к. привязывается один буфер
	static VkVertexInputBindingDescription getBindingDescription() 
	{
		// A vertex binding describes at which rate to load data from memory throughout the vertices. 
		// It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance 
	
		// разбиение буфера вершинных данных по блокам (вершинам):
		VkVertexInputBindingDescription bindingDescription{};
		//All of our per-vertex data is packed together in one array, so we’re only going to have one binding, поэтому индекс будет 0
			bindingDescription.binding = 0;								// The binding parameter specifies the index of the binding in the array of bindings
			bindingDescription.stride = sizeof(Vertex2);					// смещение между блоками (вершинами) в байтах
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	// Move to the next data entry after each vertex
		
		return bindingDescription;	
	}
	
	// А вот атрибутов может быть много
	static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() 
	{
		// разбиение блока одной конкретной вершины из буфера вершинных данных на составляющие его атрибуты и описание, как их вычленять:
        std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{}; // пустой массив строго фиксированного размера
					
			// везде д.б. VK_FORMAT_R32G32B32_SFLOAT для выравнивания по 4 байта
			
			// описание первого атрибута - position attribute (0)
			attributeDescriptions[0].binding = 0;							// из какого биндинга идут данные о вершине
			attributeDescriptions[0].location = 0;							// индекс входной location-директивы из вершинного шейдера. (layout(location = 0) in vec2 inPosition;)
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// тип данных - vec3 XYZ
			attributeDescriptions[0].offset = offsetof(Vertex2, pos);		// макрос для автоматического вычисления смещения атрибута в массиве

			// описание второго атрибута - normal attribute (1)
			attributeDescriptions[1].binding = 0;							// из какого биндинга идут данные о вершине
			attributeDescriptions[1].location = 1;							// индекс входной location-директивы из вершинного шейдера. (layout(location = 1) in vec3 inColor;)
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;//VK_FORMAT_R8G8B8_SNORM;		// тип данных - vec3 RGB
			attributeDescriptions[1].offset = offsetof(Vertex2, normal);		// макрос для автоматического вычисления смещения атрибута в массиве

			// описание третьего атрибута - texCoords (2)
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;	//VK_FORMAT_R8G8_UNORM;
			attributeDescriptions[2].offset = offsetof(Vertex2, texCoord);
			
			// описание четвертого атрибута - индекса материала (3)
			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32_UINT;
			attributeDescriptions[3].offset = offsetof(Vertex2, materialIndex);
			
			// описание пятого атрибута - вектора тангенса (4)
			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(Vertex2, tangent);

        return attributeDescriptions;
    }

};



struct FhFullscreenQuadVertex 	// описание одной вершины геометрической модели
{
	// структура хранит данные для 1 вершины.

	glm::vec2 	pos;			// первый атрибут - position
	glm::vec2 	texCoord; 		// UV
	
	
	// Binding - всего один, т.к. привязывается один буфер
	static VkVertexInputBindingDescription getBindingDescription() 
	{
		// A vertex binding describes at which rate to load data from memory throughout the vertices. 
		// It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance 
	
		// разбиение буфера вершинных данных по блокам (вершинам):
		VkVertexInputBindingDescription bindingDescription{};
		//All of our per-vertex data is packed together in one array, so we’re only going to have one binding, поэтому индекс будет 0
			bindingDescription.binding = 0;								// The binding parameter specifies the index of the binding in the array of bindings
			bindingDescription.stride = sizeof(FhFullscreenQuadVertex);					// смещение между блоками (вершинами) в байтах
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	// Move to the next data entry after each vertex
		
		return bindingDescription;	
	}
	
	// А вот атрибутов может быть много
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
	{
		// разбиение блока одной конкретной вершины из буфера вершинных данных на составляющие его атрибуты и описание, как их вычленять:
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{}; // пустой массив строго фиксированного размера
								
			// описание первого атрибута - position attribute (0)
			attributeDescriptions[0].binding = 0;							// из какого биндинга идут данные о вершине
			attributeDescriptions[0].location = 0;							// индекс входной location-директивы из вершинного шейдера. (layout(location = 0) in vec2 inPosition;)
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;	// тип данных - vec3 XYZ
			attributeDescriptions[0].offset = offsetof(FhFullscreenQuadVertex, pos);		// макрос для автоматического вычисления смещения атрибута в массиве

			// описание второго атрибута - UV attribute (1)
			attributeDescriptions[1].binding = 0;							// из какого биндинга идут данные о вершине
			attributeDescriptions[1].location = 1;							// индекс входной location-директивы из вершинного шейдера. (layout(location = 1) in vec3 inColor;)
			attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;		//VK_FORMAT_R8G8B8_SNORM;		// тип данных - vec3 RGB
			attributeDescriptions[1].offset = offsetof(FhFullscreenQuadVertex, texCoord);		// макрос для автоматического вычисления смещения атрибута в массиве

        return attributeDescriptions;
    }

};



struct FhMaterialInfo // таких структур в буфер сущности кладется 4 штуки
{
	// материал может вмещать в себя до 16 текстур, уточняющих значения тех или иных параметров для конкретного пикселя.
	// при парсинге материала индекс всех карт инкрементируется, а в шейдере наоборот - вычитается. 
	// Т.о. если индекс карты colorAddress== 0, то это значит, что текстура не назначена и надо брать значение по-умолчанию colorDefault
	
	uint32_t colorAddress = 0;
	uint32_t normalAddress = 0;
	uint32_t roughnessAddress = 0;
	uint32_t metallicAddress = 0;
	
	uint32_t specularAddress = 0;
	uint32_t lightmapAddress = 0;
	uint32_t emissionAddress = 0;
	uint32_t displacementAddress = 0;
	
	float colorScale = 1.0f;
	float normalScale = 1.0f;
	float roughnessScale = 1.0f;
	float metallicScale = 1.0f;
	
	float specularScale = 1.0f;
	float lightmapScale = 1.0f;
	float emissionScale = 1.0f;
	float displacementScale = 1.0f;
	
	float alphaDefault = 1.0f;
	float roughnessDefault = 0.0f;
	float metallicDefault = 0.0f;
	float specularDefault = 0.0f;
	
	alignas(16) glm::vec3 colorDefault 		= glm::vec3(1,1,0);	// yellow by default
	alignas(16) glm::vec3 emissionDefault 	= glm::vec3(0,0,1);	// blue
	
};



struct FhShadowpassPushConstantsInfo // per-entity personal data for Shadow-pass
{
	glm::mat4 model;				// entity model matrix
	glm::mat4 PV;
};

struct FhShadowpassCubeEdgesInfo	// UBO
{
	glm::mat4 PV[6];
};



struct FrameData 
{    
	VkSemaphore 	imageAvailableSemaphore;				// Семафоры используются для синхронизации (упорядочивания) выполнения работ в одной очереди или между разными очередями
	VkSemaphore 	renderFinishedSemaphore;				// Есть два типа семафоров - binary and timeline. В данном примере используются только  binary. 
	
	VkFence 		inFlightFence;							// заборы используются для синхронизации GPU с CPU

	VkCommandPool 	commandPool;							// объект, который управляет памятью для хранения командных буферов (Command Buffers)
	VkCommandBuffer commandBuffer;							// удаляется автоматически при уничтожении Command pool, вручную не надо
		
};



struct FhMovementScenarioInfo 												// структура описывает сценарий одного конкретного корневого двигающегося/вращающегося элемента
{
	std::string name = "none";
	
	glm::vec3	defaultPose = glm::vec3(0,0,0);
	glm::vec3	targetPose = glm::vec3(0,0,0);
	float		fTimeForMoving = 1.0f;
	
	glm::quat	defaultQuat = glm::quat(1,0,0,0);
	glm::quat	targetQuat = glm::quat(1,0,0,0);
	float		fTimeForRotation = 1.0f;
	
	bool		defaultState	= false;
	bool		targetState		= true;
	float		fTimeForChangeState = 1.0f;
	
	int 		transformationType = 0; 							// см. MAIN_MOVING_PARTS_TYPE
	
	float		powValue = 1;										// по умолчанию степень коэффициента смешивания = 1, что соответствует линейному закону
	
	//int			keyCode = GLFW_KEY_E;								// код назначенной клавиши (GLFW_KEY_A etc)
	bool 		mouse = false;										// флаг переключения внимания на мышь
	bool 		keyPressed = false;									// флаг нажатия назначенной клавиши
	
	bool 		status = false;
	float		fTick = 0.0f;	
};

