#include "globals.h"

#include "fhRender.h"
#include "Camera.h"
#include "Scena.h"
#include "Material.h"
#include "Laboratory.h"
//~ #include "Mesh.h"

bool 							g_Key[350];


//~ std::shared_ptr<Scena> 			g_SceneMicromir;


std::shared_ptr<Camera> 		g_Camera;


//~ Graphics 						g_GraphicsCore;


uint32_t 						WIDTH = 1366;
uint32_t 						HEIGHT = 768;

//~ std::string 					MODEL_PATH = "datas\\meshes\\";
//~ std::string 					SHADER_PATH = "datas\\shaders\\";

//~ int 							MAX_FRAMES_IN_FLIGHT = 2;


std::shared_ptr<Laboratory>		g_Lab;


//~ resManAsset<Mesh>		Meshes(core);

//~ resManAsset<VkTexture>	VkTextures(core);

//~ resManAsset<Material>	Materials(core);
