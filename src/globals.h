#pragma once

#include <memory>
#include <string>
#include "resourceManager.h"
// This file contains all global variables used in project

extern bool 					g_Key[350];			// massive of key states

//~ class Scena;
//~ extern std::shared_ptr<Scena> 	g_SceneMicromir;	// actor 1

class Camera;
extern std::shared_ptr<Camera> 	g_Camera;			// actor 2

//~ class Graphics;
//~ extern Graphics 				g_GraphicsCore;		// graphics engine


extern uint32_t 				WIDTH;// = 1366;
extern uint32_t 				HEIGHT;// = 768;

//~ extern std::string 				MODEL_PATH;// = "datas\\meshes\\";
//~ extern std::string 				SHADER_PATH;// = "datas\\shaders\\";


#ifdef _WIN32
	#define FH_MODEL_PATH "datas\\meshes\\"
#endif
#ifdef __linux__
	#define FH_MODEL_PATH "./datas/meshes/"
#endif


#ifdef _WIN32
	#define FH_SHADER_PATH "datas\\shaders\\"
#endif
#ifdef __linux__
	#define FH_SHADER_PATH "./datas/shaders/"
#endif



#ifdef _WIN32
	#define FH_TEXTURE_PATH "datas\\textures\\"
#endif
#ifdef __linux__
	#define FH_TEXTURE_PATH "./datas/textures/"
#endif


constexpr int 					MAX_FRAMES_IN_FLIGHT = 3;				// MAX_FRAMES_IN_FLIGHT — это сколько кадров CPU/GPU будет обрабатывать 
																		// одновременно, а количество изображений в swapchain — это сколько 
																		// color images вообще есть у цепочки показа

constexpr int					MAX_MATERIALS_PER_ENTITY = 4;			// максимум материалов, назначаемых одному объекту
constexpr int					MAX_SAMPLERS_PER_MATERIAL = 16;			// максимум текстурных сэмплеров в 1 материале


constexpr int					MAX_ENTITIES_IN_ACTION = 500;			// максимум объектов на сцене

constexpr int					MAX_DYNAMIC_LIGHT_SOURCES = 16;			// максимум динамических источников света

constexpr int					SHADOWMAP_WIDTH = 1024;
constexpr int					SHADOWMAP_HEIGHT = 1024;



class Laboratory;
extern std::shared_ptr<Laboratory>	g_Lab;

//~ class Mesh;
//~ class VkTexture;
//~ class Material;

//~ extern resManAsset<Mesh>		Meshes;
//~ extern resManAsset<VkTexture>	VkTextures;
//~ extern resManAsset<Material>	Materials;

