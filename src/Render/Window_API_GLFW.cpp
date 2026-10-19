#include "auxiliary.h" // Stopwatch
#include "Window_API_GLFW.h"

#include "macros.h"	// prints

#include "globals.h"	// g_Key


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>




GLFW::GLFW() // конструктор инициализирует окно
{
	glfwInit();
	
	printMessage("\n\nCREATE NEW WINDOW\n\n")

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);		// GLFW создавался под OpenGL, поэтому сейчас этого API не надо
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);			// измение размера окна влечет пересоздание swapchain, поэтому пока заблокировано

	window = glfwCreateWindow(WIDTH, HEIGHT, "fhEngineVK v0.2.1", nullptr, nullptr);
	
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);	// callback для отслежиавния изменения размера окна. Нужен для пересоздания swapchain
};


GLFW::~GLFW()	// деструктор очищает ресурсы окна
{
	glfwDestroyWindow(window);
	glfwTerminate();
}


void GLFW::framebufferResizeCallback(GLFWwindow* Window, int width, int height) 
{
	auto app = reinterpret_cast<GLFW*>(glfwGetWindowUserPointer(Window));
	app->framebufferResized = true;
}


void GLFW::updateInputs()	// вызывать в каждом цикле эту функцию выгоднее, чем делать постоянные запросы через framework?
{
	//printMessage("Update inputs")
	
	stopwatch.updateTick();
	
	// Printable keys 32-162
	for(int k=32; k< 163; k++)
	{
		if( glfwGetKey(window, k) ) { g_Key[k] = true; }
		if( !glfwGetKey(window, k) ) { g_Key[k] = false; }
	}
	
	// Function keys 256-348
	for(int k=256; k< 349; k++)
	{
		if( glfwGetKey(window, k) ) { g_Key[k] = true; }
		if( !glfwGetKey(window, k) ) { g_Key[k] = false; }
	}
}

