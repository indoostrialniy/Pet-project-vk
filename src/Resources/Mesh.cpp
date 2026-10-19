#include "Mesh.h"

#include <iostream>
#include <nameof.hpp>
#include "macros.h"
#include "auxiliary.h"
#include <cstdio>
#include "globals.h"

#include "fhRender.h"

#include <tiny_obj_loader.h>


//~ #define MESH_DEBUG

//~ Mesh::Mesh() {}
Mesh::Mesh( std::string meshName, Graphics& core ) : g_GraphicsCore(core)
{
	#ifdef MESH_DEBUG
		std::cout << "Constructor of [" << NAMEOF_TYPE(decltype(this)) << "] [" <<  meshName << "]" << std::endl;
	#endif
		
	resourceName = meshName;
		
	// get Host-side model data (vector)
	//status = loadModelObj(meshVertexDataVector, meshName); // в собственный meshVertexDataVector загрузим OBJ-модель
	status = meshLoadBinary(meshName);
	
	// make GPU-side buffer objects: meshVertexBuffer, meshVertexBufferMemory
	g_GraphicsCore.createVertexBuffer( 	meshVertexDataVector2, meshVertexBuffer, meshVertexBufferMemory);	// создание буфера для хранения координат и цветов точек фигуры. Короче - VBO
	
	//status = meshLoadBinary(meshName) ? true : false;
}


Mesh::~Mesh()
{
	#ifdef MESH_DEBUG
		std::cout << "Destructor of [" << NAMEOF_TYPE(decltype(this)) << "] [" <<  resourceName << "]" << std::endl;
	#endif
	
	// createVertexBuffer( vertices, buffer, bufferMemory) - destroyVertexBuffer(buffer, bufferMemory)
	
	g_GraphicsCore.destroyVertexBuffer( meshVertexBuffer, meshVertexBufferMemory);	
	
	//~ std::cout << "\nEnd of Destructor of [" << NAMEOF_TYPE(decltype(this)) << "] [" <<  resourceName << "]" << std::endl;
}


int	Mesh::getVerticesCount()			
{	
	return meshVerticesCount;	
}


VkBuffer& Mesh::getMeshVertexBuffer()
{ 
	return meshVertexBuffer;
}


bool Mesh::getStatus()
{
	return status;
}

std::string Mesh::getResourceName()
{
	return resourceName;
}


bool Mesh::loadModelObj( std::vector<Vertex>& verts, std::string filename ) 
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str() )) // MODEL_PATH.c_str()
	{
		throw std::runtime_error(err);
	}

	//~ std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) 
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			// The OBJ format assumes a coordinate system where a vertical coordinate of 0 means
			// the bottom of the image, however we’ve uploaded our image into Vulkan in a top to bottom orientation
			// where 0 means the top of the image. Solve this by flipping the vertical component of the texture coordinates
			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			
			vertex.color = {1.0f, 1.0f, 1.0f};
			
			//gameObjects[0].
			verts.push_back(vertex);

			// ниже идет код для дедупликации вершин, но мне он не нужен
			// if (uniqueVertices.count(vertex) == 0) 
			// {
				// uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				// vertices.push_back(vertex);
			// }

			// indices.push_back(uniqueVertices[vertex]);
		}
	}
	
	meshVerticesCount = meshVertexDataVector.size();
	
	return true; // OBJ loaded successfully!
}


	
	
	


bool Mesh::meshLoadBinary(std::string name)
{
	std::string meshFileSource = name; //путь уже есть в названии! meshGetPathBinary(name);

	#ifdef MESH_DEBUG
		printVarComment("Load binary ", meshFileSource)
	#endif

	
	FILE* fp;
	fp = fopen(meshFileSource.c_str(), "rb");

	if (fp != nullptr)
	{
		// 1. read first two (!) float values in file which are number of triangles and number of vertices
		float ch[2];							
		fread(&ch, sizeof(float), 2, fp);			

		meshFacesCount = ch[0];
		meshVerticesCount = ch[1];
		
		#ifdef MESH_DEBUG
			printVariable(meshFacesCount)
			printVariable(meshVerticesCount)
		#endif
		
		
		
		// 2. read next 32 bytes, which are the skeleton name (optional)
		std::string skeleton = "noskeleton";		// по умолчанию скелет отсутствует
		char stroka[32];
		fread(&stroka, sizeof(char), 32, fp);		// считаем 32 символа, в которых может быть обозначен используемый скелет
		skeleton = stroka;							// запишем вероятно-обозначенный скелет в строку
		
		if (getValue(skeleton, 0) != "noskeleton")	// имя скелета всегда будет самым первым в строке из 32 символов, поэтому если там НЕ дефолтное слово "noskeleton"
		{
			// Model is skeletal!
			meshAttributesCount = 23; // so there will be more than 9 attributes
			meshSkeletonName = getValue(skeleton, 0); // присвоили значение optional-переменной
		}
		#ifdef MESH_DEBUG
			printVariable(skeleton)
			if( meshSkeletonName.has_value() ) { printVariable( meshSkeletonName.value() ) }
			printVariable(meshAttributesCount)
		#endif
		
		
		// 3. read one more float, which is a number of AABB-floats. DEPRECATED!
		float aabb[1];
		fread(&aabb, sizeof(float), 1, fp); // leave this reading as a duct-tape solution (CEAE 0.4.4 offers no AABB in meshes)!
		
			
		// 4. read the whole file to load geometry
		float* vertices = new float[meshVerticesCount * meshAttributesCount];
		fread(vertices, sizeof(float), meshVerticesCount * meshAttributesCount, fp);

		fclose(fp);
		
		//~ printVariable(meshVerticesCount)
		//~ printVariable(meshAttributesCount)
		
		
		// meshVertexDataVector2 заполнить
		for(int v=0; v<meshVerticesCount; v++) // перебор вершин
		{
			Vertex2 vertex{};
			
			vertex.pos 			= {	vertices[9*v + 0], vertices[9*v + 1], vertices[9*v + 2]	};			// x y z
			vertex.normal 		= { vertices[9*v + 3], vertices[9*v + 4], vertices[9*v + 5] };			// nx ny nz
			vertex.texCoord	= {	vertices[9*v + 6], 1.0f - vertices[9*v + 7] };							//u, v. Vulkan invert the V axis
			vertex.materialIndex = static_cast<uint32_t>(vertices[9*v + 8]);							// material index
			
			// t - triangle index
			int t = v/3; // 0 0 0 1 1 1 2 2 2 3 3 3 4 4 4 5 5 5 6 6 6 7 7 7 ...
			//~ printVariable(t)
			// нужны все 3 точки полигона
			float P0x = vertices[t*3*meshAttributesCount + 0*meshAttributesCount + 0];
			float P0y = vertices[t*3*meshAttributesCount + 0*meshAttributesCount + 1];
			float P0z = vertices[t*3*meshAttributesCount + 0*meshAttributesCount + 2];
			float P0u = vertices[t*3*meshAttributesCount + 0*meshAttributesCount + 6];
			float P0v = 1.0f - vertices[t*3*meshAttributesCount + 0*meshAttributesCount + 7];
			
			float P1x = vertices[t*3*meshAttributesCount + 1*meshAttributesCount + 0];
			float P1y = vertices[t*3*meshAttributesCount + 1*meshAttributesCount + 1];
			float P1z = vertices[t*3*meshAttributesCount + 1*meshAttributesCount + 2];
			float P1u = vertices[t*3*meshAttributesCount + 1*meshAttributesCount + 6];
			float P1v = 1.0f - vertices[t*3*meshAttributesCount + 1*meshAttributesCount + 7];
			
			float P2x = vertices[t*3*meshAttributesCount + 2*meshAttributesCount + 0];
			float P2y = vertices[t*3*meshAttributesCount + 2*meshAttributesCount + 1];
			float P2z = vertices[t*3*meshAttributesCount + 2*meshAttributesCount + 2];
			float P2u = vertices[t*3*meshAttributesCount + 2*meshAttributesCount + 6];
			float P2v = 1.0f - vertices[t*3*meshAttributesCount + 2*meshAttributesCount + 7];
		
			//~ printVariable(P0x)
			//~ printVariable(P0y)
			//~ printVariable(P0z)
			
			//~ printVariable(P1x)
			//~ printVariable(P1y)
			//~ printVariable(P1z)
			
			//~ printVariable(P2x)
			//~ printVariable(P2y)
			//~ printVariable(P2z)
			
			// E1=P1-P0		E2=P2-P0
			float E1x = P1x - P0x;
			float E1y = P1y - P0y;
			float E1z = P1z - P0z;
			float dU1 = P1u - P0u;
			float dV1 = P1v - P0v;
				
			float E2x = P2x - P0x;
			float E2y = P2y - P0y;
			float E2z = P2z - P0z;
			float dU2 = P2u - P0u;
			float dV2 = P2v - P0v;
			float coeff = dU1*dV2 - dU2*dV1;
			
			
									
			glm::vec3 tangent = ( 1/(coeff) )*glm::vec3(dV2*E1x-dV1*E2x, dV2*E1y-dV1*E2y, dV2*E1z-dV1*E2z);

			//~ printVariable(coeff)
			//~ printVec3(tangent)

			// ← вот этого не хватает:
			if (glm::length(tangent) > 0.0001f) {
				tangent = glm::normalize(tangent);
			} else {
				tangent = glm::vec3(1.0f, 0.0f, 0.0f); // заглушка для вырожденных случаев
			}

			vertex.tangent = tangent;	// 
			
			//~ printVec3( vertex.tangent )
			//~ std::cout << "\n";
			//~ printVec3(vertex.pos)
			//~ printVec3(vertex.normal)
			//~ printVec3(vertex.texCoord_mat)
			//printVariable(vertex.materialIndex)
			
			meshVertexDataVector2.push_back(vertex);
		}
		//~ printVariable(meshVertexDataVector2.size())
		//~ printVariable(sizeof(meshVertexDataVector2))
		
		meshVerticesCount = meshVertexDataVector2.size();

		// 4.1 make & fill GPU-side buffers in Vulkan!
		////glGenBuffers(1, &VBO);
		////glGenVertexArrays(1, &VAO);
		////glBindVertexArray(VAO);
		////glBindBuffer(GL_ARRAY_BUFFER, VBO);
		////glBufferData(GL_ARRAY_BUFFER, sizeof(float) * meshVerticesCount * meshAttributesCount, vertices, GL_STATIC_DRAW);	//4 байта на флоат для attributesCount флоатов для meshAttributesCount вершин

		delete[] vertices;					// no more need in host-side vertices block. Don`t forget to free the memory
		
		// 4.2 configure GPU-side buffers
		//~ #define XYZ_ATTRIB 0
		//~ #define NORMAL_ATTRIB 1
		//~ #define UV_ATTRIB 2
		//~ #define MATERIAL_ATTRIB 3
		
		// Format (9 floats) for NON-SKELETAL meshes: X Y Z,		NX NY NZ,		UV.u UV.v,		Material index,

		//~ glVertexAttribPointer(XYZ_ATTRIB, 3, GL_FLOAT, GL_FALSE, meshAttributesCount * sizeof(GLfloat), (GLvoid*)0);
		//~ glEnableVertexAttribArray(XYZ_ATTRIB);
		//~ glVertexAttribPointer(NORMAL_ATTRIB, 3, GL_FLOAT, GL_FALSE, meshAttributesCount * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float)));
		//~ glEnableVertexAttribArray(NORMAL_ATTRIB);
		//~ glVertexAttribPointer(UV_ATTRIB, 2, GL_FLOAT, GL_FALSE, meshAttributesCount * sizeof(GLfloat), (GLvoid*)(6 * sizeof(float)));
		//~ glEnableVertexAttribArray(UV_ATTRIB);
		//~ glVertexAttribPointer(MATERIAL_ATTRIB, 1, GL_FLOAT, GL_FALSE, meshAttributesCount * sizeof(GLfloat), (GLvoid*)(8 * sizeof(float)));
		//~ glEnableVertexAttribArray(MATERIAL_ATTRIB);

		//~ if( meshSkeletonName.has_value() )		// if mesh has specified skeleton, configure more attributes
		//~ {
		// Format (23 floats) for SKELETAL meshes: X Y Z,		NX NY NZ,		UV.u UV.v,		Material index,		Bone1-2, Bone3-4, Bone5-6, Bone7

			//~ #define Bone1_Bone2 4
			//~ #define Bone3_Bone4 5
			//~ #define Bone5_Bone6 6
			//~ #define Bone7 7

			//~ glVertexAttribPointer(Bone1_Bone2, 4, GL_FLOAT, GL_FALSE, meshAttributesCount * sizeof(GLfloat), (GLvoid*)(9 * sizeof(float)));
			//~ glEnableVertexAttribArray(Bone1_Bone2);
			//~ glVertexAttribPointer(Bone3_Bone4, 4, GL_FLOAT, GL_FALSE, meshAttributesCount * sizeof(GLfloat), (GLvoid*)(13 * sizeof(float)));
			//~ glEnableVertexAttribArray(Bone3_Bone4);
			//~ glVertexAttribPointer(Bone5_Bone6, 4, GL_FLOAT, GL_FALSE, meshAttributesCount * sizeof(GLfloat), (GLvoid*)(17 * sizeof(float)));
			//~ glEnableVertexAttribArray(Bone5_Bone6);
			//~ glVertexAttribPointer(Bone7, 2, GL_FLOAT, GL_FALSE, meshAttributesCount * sizeof(GLfloat), (GLvoid*)(21 * sizeof(float)));
			//~ glEnableVertexAttribArray(Bone7);
		//~ }

		////glBindVertexArray(0);

		//ДОБАВИТЬ проверку корректности загрузки модели 
		
		#ifdef MESH_DEBUG
			printMessage("Mesh loaded successfully!")
		#endif
		
		return true;
	}
	else
	{
		std::cout << "\t Error: Cannot open file [" << meshFileSource << "] (class Mesh). Status of " << name << " FALSE!" << std::endl;
		DEBUG_INFO
		
		fclose(fp);
		return false;
	}

}


	


std::string Mesh::meshGetPathBinary(std::string filename)	
{	
	//~ return getOperationSystem() == "Windows" ? "datas\\meshes\\" + filename + ".model" : "./datas/meshes/" + filename + ".model" ;	//тернарный определитель пути	
	return "datas\\meshes\\" + filename + ".model";
}	

	
	
	

	

	
