#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class Graphics;

class VkTexture
{
// обертка текстуры хранит только идентификаторы и ссылку на само приложение
public:
	VkTexture(const std::string& name, Graphics& core);
	~VkTexture();
		
	Graphics& g_GraphicsCore;
	
	
	
	std::string					_filename = "vkTexture";
	
	VkImage 					_textureImage;							// host-side дескриптор изображения, связанный с видеопамятью textureImageMemory
	VkDeviceMemory 				_textureImageMemory;						// зарезервированная под текстуру видеопамять
	VkImageView 				_textureImageView;						// обертка над куском (дескриптором куска?) видеопамяти, позволяет воспринимать этот кусок как изображение. Надо удалять вручную
	VkSampler 					_textureSampler;							// описывает, как именно обрабатывать изображение - как фильтрова
};
