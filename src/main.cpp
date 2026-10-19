#include "globals.h"			// g_GraphicsCore
#include <iostream>

#include "fhRender.h"
#include "macros.h"

#define STB_IMAGE_IMPLEMENTATION		// для тел функций
#include <stb_image.h>	// One code file needs to include the header with the STB_IMAGE_IMPLEMENTATION definition to include the function bodies, otherwise we’ll get linking errors.


#define TINYOBJLOADER_IMPLEMENTATION	// для тел функций
#include <tiny_obj_loader.h>



//~ #define SOL_ALL_SAFETIES_ON 1
//~ #include "sol.hpp"
//~ extern "C" {
    //~ #include <lua.h>
    //~ #include <lualib.h>
    //~ #include <lauxlib.h>
//~ }
//~ #include <glm/glm.hpp>



	//~ 
	
	//~ resManAsset<Mesh>		Meshes(ENGINE);

	//~ resManAsset<VkTexture>	VkTextures(ENGINE);

	//~ resManAsset<Material>	Materials(ENGINE);
	
//~ resManAsset<Mesh>		Meshes(ENGINE);

//~ resManAsset<VkTexture>	VkTextures(ENGINE);

//~ resManAsset<Material>	Materials(ENGINE);




//~ int foo(lua_State* L) 
//~ {
	//~ // 0-й аргумент - это сам экземпляр LUA
    //~ double a = lua_tonumber(L, 1);	// взяли первый аргумент
    //~ double b = lua_tonumber(L, 2);	// второй аргумент
    
    //~ lua_pushnumber(L, a + b);      // кладём результат
    
    //~ return 1;                      // указываем, что возвращается один аргумент (результат)
//~ }


//~ class Renderer 
//~ {
//~ public:
    //~ void setViewport(int w, int h) 
    //~ { 
		//~ /* ... */ 
		//~ printMessage("Function called from LUA!")
	//~ }
//~ };

//~ Renderer* g_renderer = nullptr;

//~ int lua_renderer_setViewport(lua_State* L) 
//~ {
    //~ if (!g_renderer) return 0;
    
    //~ int w = lua_tonumber(L, 1);
    //~ int h = lua_tonumber(L, 2);
    
    //~ g_renderer->setViewport(w, h);
    
    //~ return 0; // нет возвращаемых значений
//~ }


struct A
{
	float a;		// 4
	float b;		// 8
	float c;		// 12
	glm::vec3 D;	// 24
	glm::vec4 E;	// 40
	
};

struct B
{
				float a;		// 4
				float b;		// 8
	alignas(16) float c;		// 3
	alignas(16) glm::vec3 D;
				glm::vec4 E;
};


struct FhDynamicLightSourceInfo1 // 464 								
{
public:
				glm::vec3 position 	= glm::vec3(0.0, 0.0, 0.0);			// 0 1 2 3 4 5 6 7 8 9 10 11
	alignas(16) glm::vec3 direction = glm::vec3(1.0, 0.0, 0.0);			// 12 13 14 15 - байты смещения. Вектор: 16 17 18 19   20 21 22 23   24 25 26 27
	alignas(16) glm::vec3 color 	= glm::vec3(1.0, 1.0, 1.0);			// 28 29 30 31 - байты смещения. Вектор: 32 33 34 35   36 37 38 39   40 41 42 43
																		// 
	uint32_t type		= FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;		// 44 45 46 47
	float	sector		= 30.0f;										// 48 49 50 51
	float	radius		= 10.0f;										// 52 53 54 55
	float	power		= 10.0f;										// 56 57 58 59
	
	float	blendWidth	= 10.0f;										// 60 61 62 63
	uint32_t exists		= 0;											// 64 65 66 67
				
	alignas(16) glm::mat4 PV[6]		{
										glm::mat4(1.0f),				// 80 - 143
										glm::mat4(1.0f),				// 144 - 207
										glm::mat4(1.0f),				// 208 - 271
										glm::mat4(1.0f),				// 272 - 335
										glm::mat4(1.0f),				// 336 - 399
										glm::mat4(1.0f)					// 400 - 463
									}; 
	// самый строгий alignas - 16, соответственно компилятор добъет до 464, но тут этого и не нужно, структура сама такого же объема
};


struct FhDynamicLightSourceInfo2 // 444										// 16 таких структур передаются в буфер к шейдеру - MAX_DYNAMIC_LIGHT_SOURCES
{
public:
	glm::vec3 position 	= glm::vec3(0.0, 0.0, 0.0);						// 3
	glm::vec3 direction = glm::vec3(1.0, 0.0, 0.0);						// 3
	glm::vec3 color 	= glm::vec3(1.0, 1.0, 1.0);						// 3
		
	uint32_t type		= FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;		// 1
	float	sector		= 30.0f;										// 1
	float	radius		= 10.0f;										// 1
	float	power		= 10.0f;										// 1
	float	blendWidth	= 10.0f;										// 1
	uint32_t exists		= 0;											// 1
	
	// уже 15 флоатов
				
	glm::mat4 PV[6]	{glm::mat4(1.0f),glm::mat4(1.0f),glm::mat4(1.0f),glm::mat4(1.0f),glm::mat4(1.0f),glm::mat4(1.0f)}; // 16*6 = 96 флоатов
}; 


struct FhDynamicLightSourceInfo3 //512
{
    glm::mat4 PV[6];   				// 384 байта, выровнено по 16

    glm::vec3 position;            // 12
    glm::vec3 direction;           // 12
    glm::vec3 color;               // 12

    float radius;                  // 4
    float power;                   // 4
    float sector;                  // 4
    float blendWidth;              // 4
    uint32_t type;                 // 4
    uint32_t exists;               // 4

    // Жёсткий паддинг до 512 байт
    float padding[17]; // 512 - 444 = 68 байт. 512 - (384 + 36 + 24)
};



struct FhDynamicLightSourceInfo4 // 80
{
    alignas(16) glm::vec3 position;            // 0 1 2 3   4 5 6 7   8 9 10 11
    alignas(16) glm::vec3 direction;           // 12 13 14 15 - байты смещения. 16 17 18 19   20 21 22 23   24 25 26 27
    alignas(16) glm::vec3 color;               // 28 29 30 31 - байты смещения. 32 33 34 35   36 37 38 39   40 41 42 43

    alignas(16) float radius;                  // 44 45 46 47 - байты смещения 48 49 50 51
				float power;                   // 52 53 54 55
				float sector;                  // 56 57 58 59
				float blendWidth;              // 60 61 62 63
				uint32_t type;                 // 64 65 66 67
				uint32_t exists;               // 68 69 70 71
	// самый строгий alignas - 16, соответственно компилятор добъет до 80
												// 72 73 74 75 76 77 78 79
};

struct FhDynamicLightSourceInfo5 // 144
{
    alignas(16) glm::vec3 position;            // Вектор: 0 1 2 3   4 5 6 7   8 9 10 11
    alignas(16) glm::vec3 direction;           // 12 13 14 15 - байты смещения. Вектор: 16 17 18 19   20 21 22 23   24 25 26 27
    alignas(16) glm::vec3 color;               // 28 29 30 31 - байты смещения. Вектор: 32 33 34 35   36 37 38 39   40 41 42 43

    alignas(16) float radius;                  // 44 45 46 47 - байты смещения. Число: 48 49 50 51
	alignas(16) float power;                   // 52 53 54 55   56 57 58 59   60 61 62 63 - байты смещения. Число: 64 65 66 67
	alignas(16) float sector;                  // 68 69 70 71   72 73 74 75   76 77 78 79 - байты смещения. Число: 80 81 82 83
	alignas(16) float blendWidth;              // 84 85 86 87   88 89 90 91   92 93 94 95 - байты смещения. Число: 96 97 98 99
	alignas(16) uint32_t type;                 // 100 101 102 103   104 105 106 107   108 109 110 111 - байты смещения. Число: 112 113 114 115
	alignas(16) uint32_t exists;               // 116 117 118 119   120 121 122 123   124 125 126 127 - байты смещения. Число: 128 129 130 131
		// самый строгий alignas - 16, соответственно компилятор добъет до 144
												// 132 133 134 135 136 137 138 139 140 141 142 143
};

struct FhDynamicLightSourceInfo6 // 
{
				glm::vec3 position;             // Вектор: 0 1 2 3   4 5 6 7   8 9 10 11
    alignas(16) glm::vec3 direction;            // 12 13 14 15 - байты смещения. Вектор: 16 17 18 19   20 21 22 23   24 25 26 27
    alignas(16) glm::vec3 color;                // 28 29 30 31 - байты смещения. Вектор: 32 33 34 35   36 37 38 39   40 41 42 43

    alignas(16) float radius;                   // 44 45 46 47 - байты смещения. Число: 48 49 50 51
				float power;                    // 								 Число: 52 53 54 55
				float sector;					//								 Число: 56 57 58 59
				float blendWidth;				//								 Число: 60 61 62 63
				uint32_t type;					//								 Число: 64 65 66 67
				uint32_t exists; 				//								 Число: 68 69 70 71
				                  
	alignas(16) glm::mat4 PV[6]; 				// 72 73 74 75  76 77 78 79	 - байты смещения. Числа: 80 - 463
			// уже 464 байта, надо добить до 512, добавим 48 байт
			float reserve[12];
};


struct FhDynamicLightSourceInfo7 //  656 needed
{
				glm::vec3 position;             // Вектор: 0 1 2 3   4 5 6 7   8 9 10 11
    alignas(16) glm::vec3 direction;            // 12 13 14 15 - байты смещения. Вектор: 16 17 18 19   20 21 22 23   24 25 26 27
    alignas(16) glm::vec3 color;                // 28 29 30 31 - байты смещения. Вектор: 32 33 34 35   36 37 38 39   40 41 42 43

    alignas(16) float radius;                   // 44 45 46 47 - байты смещения. Число: 48 49 50 51
	alignas(16)	float power;                    // 								 Число: 64 65 66 67
	alignas(16)	float sector;					//								 Число: 80 81 82 83
	alignas(16)	float blendWidth;				//								 Число: 96 97 98 99
	alignas(16)	uint32_t type;					//								 Число: 112 113 114 115
	alignas(16)	uint32_t exists; 				//								 Число: 128 129 130 131
				                  
	alignas(16) glm::mat4 PV[6]; 				// 								Числа: 144 - 527
			// уже 464 байта, надо добить до 512, добавим 48 байт
	alignas(16)  float reserve[12];				//								Числа 544 - 
};

//_____//_____//_____//____//
	//~ a	  b
	//~ c
	//~ D.x	  D.y    D.z	  
	//~ E.x   E.y    E.z	E.w


int main() 
{	
	//~ printVariable(sizeof(A)) //40
	//~ printVariable(sizeof(B)) //64
	
	//~ printVariable(sizeof(FhDynamicLightSourceInfo1)) //464
	//~ printVariable(sizeof(FhDynamicLightSourceInfo2)) //444
	//~ printVariable(sizeof(FhDynamicLightSourceInfo3)) //512
	//~ printVariable(sizeof(FhDynamicLightSourceInfo4)) //528
	//~ printVariable(sizeof(FhDynamicLightSourceInfo5)) //528
	//~ printVariable(sizeof(FhDynamicLightSourceInfo6)) //528
	//~ printVariable(sizeof(FhDynamicLightSourceInfo7)) //528


	
	//~ system("pause");
	
	//~ g_renderer = new Renderer;
	
	
	//~ // begin lua
	//~ lua_State* L = luaL_newstate();
	//~ luaL_openlibs(L);                 // подключает стандартные модули (math, io и т.д.)
	
	//~ // регистрация:
	//~ lua_pushcfunction(L, foo);
	//~ lua_setglobal(L, "foo");          // теперь в Lua доступно foo(a,b)
	
	//~ lua_pushcfunction(L, lua_renderer_setViewport);
	//~ lua_setglobal(L, "renderer_setViewport");
	
	//~ luaL_dofile(L, "datas\\scripts\\oasis.lua");    // загружает и выполняет скрипт
	
	//~ // end lua
	//~ lua_close(L); // закрываем состояние
	//~ delete g_renderer;
	
	
	//~ sol::state lua;
	//~ lua.open_libraries();
	
	
	//~ // Регистрируем vec3
	//~ lua.new_usertype<glm::vec3>("vec3",
		//~ sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
		//~ "x", &glm::vec3::x,
		//~ "y", &glm::vec3::y,
		//~ "z", &glm::vec3::z,
		//~ sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
		//~ sol::meta_function::subtraction, [](const glm::vec3& a, const glm::vec3::b) { return a - b; }
	//~ );

	//~ // Регистрируем quat
	//~ lua.new_usertype<glm::quat>("quat",
		//~ sol::constructors<glm::quat(), glm::quat(float, float, float, float)>(),
		//~ "x", &glm::quat::x,
		//~ "y", &glm::quat::y,
		//~ "z", &glm::quat::z,
		//~ "w", &glm::quat::w
	//~ );
	
	//~ glm::mat4 M = glm::mat4(0.0f);
	//~ printMat4("Matrix 0 ", M)
	
	//~ glm::mat4 N = glm::mat4(1.0f);
	//~ printMat4("Matrix 1 ", N)
	
	//~ glm::mat4 P = glm::mat4(glm::vec4(1.0f),glm::vec4(2.0f),glm::vec4(3.0f),glm::vec4(4.0f));
	//~ printMat44("Matrix 1 ", P)
	//~ system("pause");
	
	{
		Graphics ENGINE;
		
		try {
			ENGINE.initVulkan(); // run()
			
			ENGINE.spawns();
			
			ENGINE.createDeferredPipeline();
			
			ENGINE.mainLoop();
					
			ENGINE.cleanup(); // XXX cleanup перенести, чтобы при обработке исключений все подчищалось
			
		} 
		catch (const std::exception& e) 
		{
			std::cerr << e.what() << std::endl;
			//return EXIT_FAILURE;
			
			system("pause");
		}
	}
	
	
    return EXIT_SUCCESS;
}




