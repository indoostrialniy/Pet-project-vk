#include "auxiliary.h"
#include "Camera.h"
#include "Scena.h"

#include <iostream>
#include <nameof.hpp>
#include <cstring>
#include <sstream>

//~ #include "fhEngine.h"

//~ bool _key[350];	// массив для состояний клавиш

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physDevice) 
{
	//~ struct VkPhysicalDeviceMemoryProperties {
		//~ uint32_t        memoryTypeCount;
		//~ VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
		//~ uint32_t        memoryHeapCount;
		//~ VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
	//~ }
	
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
	{
		// checking if the corresponding bit is set to 1 			и флаг типа памяти равен заданному через аргумент properties
		if(     (typeFilter & (1 << i)) 					&& (memProperties.memoryTypes[i].propertyFlags & properties) 		== properties     ) 
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}




Stopwatch::Stopwatch()
{
	timeOfInstanceCreation = glfwGetTime();
	timeOfLastFrame = glfwGetTime();
}

Stopwatch::Stopwatch(const std::string& className)
{
	Stopwatch();
	name = className;
}

Stopwatch::~Stopwatch()
{
	timeOfInstanceDestruction = glfwGetTime() - timeOfInstanceCreation;
	std::cout << "Lifetime of " << name << ": " << timeOfInstanceDestruction << std::endl;
}

void Stopwatch::updateTick()	// обязана вызываться каждый цикл в Core::mainLoop()!
{
	timeSinceLastFrame = glfwGetTime() - timeOfLastFrame;
	//std::cout << timeSinceLastFrame << std::endl;
	timeOfLastFrame = glfwGetTime();
}

float Stopwatch::getTick()
{
	return timeSinceLastFrame;
}
	
double Stopwatch::timeSinceLastFrame = 0; // первичная инициализация СТАТИЧЕСКОЙ переменной счетчика времени между кадрами


std::string getValue(const std::string& strSource, int num) //возврат слова с порядковым номером num из строки strSource
{
	//cout << "Incoming line: " << *strSource << endl;
	std::istringstream potokIzStroki(strSource); //создали входной поток из строки StrVar
	std::string str = "none";
	for (int i = 0; i <= num; i++, potokIzStroki >> str) //num раз пропарсим строку
		str = i == num ? str : "none";    //когда дойдем до заданного слова, сохраним в переменной само слово, иначе - "0"
		
	//potokIzStroki.close();
	return str;
}

std::string getValueOrNone(std::string& strSource, int num) //возврат слова с порядковым номером num из строки strSource. используется для загрузки скелета
{
	int currWordIndex = 0;
	std::string word;

	for (char c : strSource)
	{
		if (c == ' ')
		{
			currWordIndex++;
			if (currWordIndex > num) { break; }
		}
		else if (currWordIndex == num) { word += c; }
	}
	if (currWordIndex < num) { return "none"; }
	return word;
}

char* StringToChar(std::string StringValue) 				// XXX переделать с аргументом по ссылке
{
	char* CharValue = new char[StringValue.length() + 1];
	strcpy(CharValue, StringValue.c_str());	//преобразуем string в char
	return CharValue;
	delete[] CharValue;
}


//~ std::shared_ptr<Scena> g_SceneMicromir;
//~ std::shared_ptr<Camera> CAMERA;


//~ Graphics app;


//~ template <typename P>
//~ bool CHECK_PTR(P pointer) 
//~ {	
	//~ if(&pointer != nullptr)	{ return true; }	
	//~ else 
	//~ {	
		//~ std::cout << "------------------------------------------------------------------------------------\nError by access ptr [" 
		//~ << NAMEOF(pointer) << "] by type [" << NAMEOF_TYPE(decltype(pointer)) 
		//~ << "]. It`s nullptr! File: " << __FILE__<< ", Line: " << __LINE__<< ", Function: " << __FUNCTION__ 
		//~ << "\n------------------------------------------------------------------------------------" << std::endl;	
		//~ return false;	
	//~ }	
//~ }

//~ template <typename P, typename N>
//~ bool CHECK_PTR(P<N> pointer)
//~ {	
	//~ if(&pointer != nullptr)	{ return true; }	
	//~ else 
	//~ {	
		//~ std::cout << "------------------------------------------------------------------------------------\nError by access ptr [" 
		//~ << NAMEOF(pointer) << "] by type [" << NAMEOF_TYPE(decltype(pointer)) 
		//~ << "]. It`s nullptr! File: " << __FILE__<< ", Line: " << __LINE__<< ", Function: " << __FUNCTION__ 
		//~ << "\n------------------------------------------------------------------------------------" << std::endl;	
		//~ return false;	
	//~ }	
//~ }
