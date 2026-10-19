#pragma once
#include <string>
#include <optional>

#include "auxiliary.h" // Vertex struct

#include "fhStructs.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Graphics;


class Mesh // класс оболочки для хранения и обработки геометрической модели (mesh)
{
public:
	//~ Mesh();
	Mesh(std::string meshName, Graphics& core);
	~Mesh();
	
	Graphics& g_GraphicsCore;
	
	int							getVerticesCount();
	
	VkBuffer&					getMeshVertexBuffer();
	
	bool 						getStatus();
	std::string 				getResourceName();
	
	std::optional<std::string> 	meshSkeletonName;// By default the model is non-skeletal. Skeleton is optional, so if it exist, its name will be written in this optional-variable. 
												// Skeleton itself will be loaded and configured during Entity creation via Entity::configureEntity()
protected:
	bool 						status = false;
	
	std::string 				resourceName = "none";
	
	// Model does`t change, so it is just one. Almost always is for Reading
	std::vector<Vertex> 		meshVertexDataVector;				// vector of Vertex-data-structs
	std::vector<Vertex2> 		meshVertexDataVector2;				// vector of Vertex-data-structs

	VkBuffer 					meshVertexBuffer; 					// host-side vertex buffer data handler
    VkDeviceMemory 				meshVertexBufferMemory;				// GPU memory reserved for buffer
	
	int 						meshFacesCount = 0;
	int							meshVerticesCount = 0;				// will need this from Draw()?
	int							meshAttributesCount = 9; 			// 9 if no bones, 17 if skeletal (because 4 bones add 8 attributes: bone index - bone weight)
	
	bool loadModelObj( std::vector<Vertex>& verts, std::string filename ) ;
	
	bool meshLoadBinary(std::string name);
	
	std::string meshGetPathBinary(std::string filename);
};

