#include "Material.h"

#include <iostream>
#include <nameof.hpp>
#include "macros.h"
#include "globals.h"
#include <fstream>  //ifstream

//~ #ifndef STB_IMAGE_IMPLEMENTATION
	//~ #define STB_IMAGE_IMPLEMENTATION
	//~ #include <stb_image.h>
//~ #endif

//~ #ifndef STB_IMAGE_WRITE_IMPLEMENTATION
	//~ #define STB_IMAGE_WRITE_IMPLEMENTATION
	//~ #include <stb_write.h>
//~ #endif

#include "auxiliary.h"
#include "vkTexture.h"

#include "fhRender.h"

//~ #define MATERIAL_DEBUG


//~ Material::Material()
//~ {

//~ };


Material::Material(std::string materialName, Graphics& core) : g_GraphicsCore(core)
{
	//~ LogDuration ld("Material constructor");
	
	resourceName = materialName;
	
	//status = true;
	//materialData.colorDefault = glm::vec3(1,0,0);
	status = loadMaterial(materialName);
	
	#ifdef MATERIAL_DEBUG
		std::cout << "Constructor of [" << NAMEOF_TYPE(decltype(this)) << "] [" <<  materialName << "], status: " << status << std::endl;
	#endif
}



Material::~Material()
{
	#ifdef MATERIAL_DEBUG
		std::cout << "Destructor of [" << NAMEOF_TYPE(decltype(this)) << "] [" <<  resourceName << "]" << std::endl;
	#endif
	
	//~ glDeleteTextures(1, &texture2DArray);
	
	//~ textureSubMapsNames.clear();
}


//~ GLuint& Material::getTexture2DArray() 
//~ {	
	//~ return texture2DArray; 
//~ }	//возможно здесь надо возвращать ссылочную переменную

FhMaterialInfo Material::getMaterialInfo()
{ 
	return materialData; 
}



bool  Material::loadMaterial(std::string& name)
{
	//при создании класса в конструктор приходит только строка с названием файла материала, например, "datas\\materials\\BRICK.ltx"
	//загружаться и настраиваться материал будет тут в классе
	//если пришли сюда, значит, дубликатов нет, значит надо прочитать файл и загрузить все текстуры и настроить матрицу конфигурации

	
	//Name = getPathCorrectedBySystem(Name);
	std::string materialFile = getMaterialPath(name);

	std::ifstream materialStream(materialFile);
	std::string StrVar;
	
	#ifdef MATERIAL_DEBUG
		std::cout << "\nLoading and configurating material: " << name << std::endl;
		printVariable(materialFile)
	#endif
	
	bool sizesEqual = true; //вспомогательная переменная для проверки равенства размеров текстур в карте

	int material_index = 1; //начинаем с нуля, чтобы в шейдере идентифицировать отсутствие карт
	if (materialStream.is_open())
	{
		//getline(materialStream, StrVar);
		while (getline(materialStream, StrVar))
		{
			
			if( getValue(StrVar, 0) == "map" )
			{
				//~ getline(materialStream, StrVar);
				std::string n;
				enum TYPE {BASE_COLOR, NORMAL, ROUGHNESS, METALLIC, DISPLACEMENT, EMISSION, LIGHTMAP, SPECULAR};
				
				int type = -1;
				int index = 0;
				std::string mapName;
				float scale = 1.0f;
				
				while (getValue(StrVar, 0) != "END")
				{
					getline(materialStream, StrVar);
					
					if( getValue(StrVar, 0) == "BaseColor") 	{ type = BASE_COLOR;}
					if( getValue(StrVar, 0) == "Normal") 		{ type = NORMAL;	}
					if( getValue(StrVar, 0) == "Roughness") 	{ type = ROUGHNESS;	}
					if( getValue(StrVar, 0) == "Metallic") 		{ type = METALLIC;	}
					if( getValue(StrVar, 0) == "Displacement") 	{ type = DISPLACEMENT;	}
					if( getValue(StrVar, 0) == "Emission")		{ type = EMISSION; }									
					if( getValue(StrVar, 0) == "Lightmap")		{ type = LIGHTMAP; }
					if( getValue(StrVar, 0) == "Specular")		{ type = SPECULAR; }
									
					if( getValue(StrVar, 0) == "BaseColor" || getValue(StrVar, 0) == "Normal" || getValue(StrVar, 0) == "Roughness" || getValue(StrVar, 0) == "Metallic" 
						|| getValue(StrVar, 0) == "Displacement" || getValue(StrVar, 0) == "Emission" || getValue(StrVar, 0) == "Lightmap" || getValue(StrVar, 0) == "Specular" )
					{			
						index = material_index;
						mapName = getValue(StrVar, 1);
						
						#ifdef MATERIAL_DEBUG
							std::cout << "catch map: " <<  mapName << std::endl;
						#endif
						
					}
									
					if( getValue(StrVar, 0) == "scale" ){ scale = stof(getValue(StrVar, 1)); }
				}
				 				
				if( type == BASE_COLOR)
				{
					materialData.colorAddress = index; 
					materialData.colorScale = scale; 
					n = FH_TEXTURE_PATH + mapName;	
					
					//printVarComment("Color", n)
				
					vpMaterialTextures.push_back( g_GraphicsCore.VkTextures.getAsset( n ) ) ;
					
					material_index++;
				}
				
				if( type == NORMAL)
				{
					materialData.normalAddress = index; 
					materialData.normalScale = scale; 
					n = FH_TEXTURE_PATH + mapName;	
					
					//printVarComment("Normal ", n)				
					
					vpMaterialTextures.push_back( g_GraphicsCore.VkTextures.getAsset( n ) ) ;
					
					material_index++;
				}
				
				if(type == LIGHTMAP)
				{
					materialData.lightmapAddress = index;					
					materialData.lightmapScale = scale; 
					n = FH_TEXTURE_PATH + mapName;	
					
					//printVarComment("Lightmap ", n)				
					
					vpMaterialTextures.push_back( g_GraphicsCore.VkTextures.getAsset( n ) ) ;
					
					material_index++;													
				}
				
				if(type == EMISSION)
				{
					materialData.emissionAddress = index;	
					materialData.emissionScale = scale; 
					n = FH_TEXTURE_PATH + mapName;	
					
					//printVarComment("Emission ", n)				
					
					vpMaterialTextures.push_back( g_GraphicsCore.VkTextures.getAsset( n ) ) ;
					
					material_index++;
				}
				
				if( type == METALLIC)
				{
					materialData.metallicAddress = index; 
					materialData.metallicScale = scale; 
					n = FH_TEXTURE_PATH + mapName;	
					
					//printVarComment("Metal", n)
				
					vpMaterialTextures.push_back( g_GraphicsCore.VkTextures.getAsset( n ) ) ;
					
					material_index++;
				}
				
				if(type == ROUGHNESS)
				{
					materialData.roughnessAddress = index;					
					materialData.roughnessScale = scale; 
					n = FH_TEXTURE_PATH + mapName;	
					
					//printVarComment("Rough ", n)
				
					vpMaterialTextures.push_back( g_GraphicsCore.VkTextures.getAsset( n ) ) ;
					
					material_index++;													
				}
				
				if(type == DISPLACEMENT)
				{
					materialData.displacementAddress = index;	
					materialData.displacementScale = scale; 
					n = FH_TEXTURE_PATH + mapName;	
					
					//printVarComment("Disp ", n)
				
					vpMaterialTextures.push_back( g_GraphicsCore.VkTextures.getAsset( n ) ) ;
					
					material_index++;
				}
				
				if(type == SPECULAR)
				{
					materialData.specularAddress = index;	
					materialData.specularScale = scale; 
					n = FH_TEXTURE_PATH + mapName;	
					
					//printVarComment("Spec", n)
				
					vpMaterialTextures.push_back( g_GraphicsCore.VkTextures.getAsset( n ) ) ;
					
					material_index++;
				}
								
				
			}
			
			if( getValue(StrVar, 0) == "value" )
			{
				getline(materialStream, StrVar);
				if( getValue(StrVar, 0) == "BaseColor")
				{
					 readFVec3( materialData.colorDefault, StrVar, 1 )
					 //materialData.colorAddress = 0;		// zero value let shader know there is no map, so it choose default value
				}
				
				if( getValue(StrVar, 0) == "Metallic")
				{
					 materialData.metallicDefault = stoi(getValue(StrVar, 1));
					 //materialData.metallicAddress = 0;
				}
				
				if( getValue(StrVar, 0) == "Roughness")
				{
					 materialData.roughnessDefault = stoi(getValue(StrVar, 1));
					 //materialData.roughnessAddress = 0;
				}
				if( getValue(StrVar, 0) == "Alpha")
				{
					 materialData.alphaDefault = stoi(getValue(StrVar, 1));
				}
				if( getValue(StrVar, 0) == "Emission")
				{
					 readFVec3( materialData.emissionDefault, StrVar, 1 )
					 //materialData.emissionAddress = 0;		// zero value let shader know there is no map, so it choose default value
				}
			}
			
			
			
			
			//~ if (getValue(StrVar, 0) == "BaseColor")
			//~ {
				//~ #ifdef MATERIAL_DEBUG
					//~ std::cout << "catch color: " <<  getValue(StrVar, 1) << std::endl;
				//~ #endif
				//~ std::string n = FH_TEXTURE_PATH + getValue(StrVar, 1);
				
				//~ vpMaterialTextures.push_back( VkTextures.getAsset( n ) ) ;
				
				//~ while (getValue(StrVar, 0) != "END")
				//~ {
					//~ if (getValue(StrVar, 0) == "scale") { matInfo COLOR_SCALE = stof(getValue(StrVar, 1)); }
					//~ if (getValue(StrVar, 0) == "size")
					//~ {
						//~ //textureSize.x = stoi(getValue(&StrVar, 1)); textureSize.y = stoi(getValue(&StrVar, 2)); 
						//~ if (textureSize.x == 128 && textureSize.y == 128)	//если пока не обновили минимальный размер карты
						//~ {
							//~ textureSize.x = stoi(getValue(StrVar, 1)); textureSize.y = stoi(getValue(StrVar, 2));	//обновляем минимальный размер карты
						//~ }
						//~ else //если минимальный размер карты уже обновлен ( больше чемм 128х128)
						//~ {
							//~ if (stoi(getValue(StrVar, 1)) != textureSize.x || stoi(getValue(StrVar, 2)) != textureSize.y) { sizesEqual = false; }
						//~ }
					//~ } //размер карт ориентирован на размер карты цвета (диффуз)
					
					//~ getline(materialStream, StrVar);
				//~ }
				
				//~ #ifdef MATERIAL_DEBUG
					//~ printVariable(matInfo COLOR_SCALE)
				//~ #endif
				
				//~ matInfo COLOR_ADDRES = material_index;
				//~ material_index++;
			//~ }

		

			

		}
		
		materialStream.close(); //как только спарсили весь документ, закрываем поток
		
		
		
		return true;		
	}
	else
	{
		std::cout << "Cannot open material source file: " << name << ", LoadMaterial (class MATERIAL)" << std::endl;
		DEBUG_INFO
		return false;
	}

}

//~ bool  Material::createTextureArray()
//~ {
	//~ //cout << "\tCreating and configurating texture array" << endl;
	//~ texture2DArrayMapsCount = textureSubMapsNames.size();

	//~ glGenTextures(1, &texture2DArray);
	//~ glBindTexture(GL_TEXTURE_2D_ARRAY, texture2DArray);
	//~ glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, textureSize.x, textureSize.y, texture2DArrayMapsCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); //зарезервировали память

	//~ stbi_set_flip_vertically_on_load(true);

	//~ int currentlayer = 0;
	//~ for (auto tex : textureSubMapsNames)
	//~ {
		//~ //cout << "\t " << tex << endl;
		//~ //tex = getPathCorrectedBySystem(tex);
		//~ std::string texture = getTexturePath(tex);

		//~ unsigned char* data = stbi_load(texture.c_str(), &textureSize.x, &textureSize.y, &textureColorChannels, STBI_rgb_alpha);

		//~ if (data == nullptr) { std::cout << "\t Error: Cannot open image file [" << texture << "]" << std::endl; delete[] data; return false; }

		//~ glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, currentlayer, textureSize.x, textureSize.y, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

		//~ glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//~ glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//~ glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//~ delete[] data;

		//~ currentlayer++;
	//~ }

	//~ return true;

//~ }

std::string  Material::getMaterialPath(std::string& filename)	
{	
	//~ return getOperationSystem() == "Windows" ? "datas\\materials\\" + filename + ".ltx" : "./datas/materials/" + filename + ".ltx" ;		
	return "datas\\materials\\" + filename + ".ltx";		

}	//тернарный определитель пути



std::string  Material::getTexturePath(std::string& filename)	
{	
	//~ return getOperationSystem() == "Windows" ? "datas\\textures\\" + filename + "" : "./datas/textures/" + filename + "" ;		
	return "datas\\textures\\" + filename + "" ;		
}	//тернарный определитель пути





//~ #include "auxiliary.cpp"
//~ int main()
//~ {
	//~ return 0;
//~ }


