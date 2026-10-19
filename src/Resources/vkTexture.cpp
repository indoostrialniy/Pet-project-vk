#include "vkTexture.h"

#include "macros.h"		// prints
#include "globals.h"	// g_GraphicsCore

#include "fhRender.h"	// here is Graphics class definition

//~ #define TEXTURE_DEBUG

VkTexture::VkTexture(const std::string& name, Graphics& core) : g_GraphicsCore(core)
{
	#ifdef TEXTURE_DEBUG
		printVarComment("Create texture ", name)
	#endif
	
	_filename = name;
	
	g_GraphicsCore.createTextureImage(name, _textureImage, _textureImageMemory );				// загрузили пиксели текстуры в видеопамять
	g_GraphicsCore.createTextureImageView( _textureImage, _textureImageView );			// обернули пиксели в абстракцию для восприятия их как изображения
	g_GraphicsCore.createTextureSampler( _textureSampler );				// настроили сэмплер под текстуру: фильтрация и т.п. 
}

VkTexture::~VkTexture()
{
	//~ printMessage("destroy texture")
	
	#ifdef TEXTURE_DEBUG
		std::cout << "Destructor of [" << NAMEOF_TYPE(decltype(this)) << "] [" <<  _filename << "]" << std::endl;
	#endif
	
	g_GraphicsCore.destroyTextureSampler( _textureSampler );
	g_GraphicsCore.destroyTextureImageView( _textureImageView );
	g_GraphicsCore.destroyTextureImage( _textureImage, _textureImageMemory );	
}

