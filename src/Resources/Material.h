#pragma once

#include <string>
//~ #include <GL/glew.h>
//~ #include <GLFW/glfw3.h>

#include <glm/glm.hpp>
//#include <glm/gtc/matrix_integer.hpp> // обязательно для umat4
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <memory>

#include "fhStructs.h" // FhMaterialInfo
// материал это оболочка над текстурами. Он набирает из ресурсов vkTextures для color, normal, roughness etc и на основании файла описания материала хранит в себе конфиги масштаба

class VkTexture;


class Material	//класс оболочки для хранения материала (массива текстур 2Д)
{
public:
	//~ Material();
	Material(std::string materialName, Graphics& core);
	
	~Material();
	
	Graphics& g_GraphicsCore;

	
//~ protected:
	bool status = false;
	
	std::string resourceName = "none";
	
	std::vector<std::shared_ptr<VkTexture>> vpMaterialTextures;			// вектор указателей на текстуры
	
	// пользовательские переменные и методы
	
	FhMaterialInfo getMaterialInfo();
	
private:



	glm::ivec2	textureSize = glm::ivec2(128, 128);

	FhMaterialInfo materialData;
	
	bool loadMaterial(std::string& name);

	std::string getMaterialPath(std::string& filename);
	
	std::string getTexturePath(std::string& filename);
};
