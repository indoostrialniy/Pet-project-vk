#pragma once

//~ #define GLFW_INCLUDE_VULKAN
//~ #include <GLFW/glfw3.h>

//~ #include "auxiliary.h" // Stopwatch

class GLFWwindow;
class Stopwatch;

class GLFW	// класс для обработки окна и ввода
{
public:
	GLFW(); // конструктор инициализирует окно
	
	~GLFW();	// деструктор очищает ресурсы окна
	
	GLFWwindow* 	window;									// окно
	
	bool 			framebufferResized = false;    			// буль изменения размера окна
	
	Stopwatch 		stopwatch;	// экземпляр счетчика времени между кадрами. Его метод updateTick() обязан вызываться в mainLoop() для корректной работы
	
	static void 	framebufferResizeCallback(GLFWwindow* Window, int width, int height);
	
	void 			updateInputs();	// вызывать в каждом цикле эту функцию выгоднее, чем делать постоянные запросы через framework

};

