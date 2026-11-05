#version 450
// к binding = 0 будем ссылаться в descriptor layout
layout(binding = 0) uniform FhGeneralInfo {
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
	float Time;
} commonData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in uint  inMaterialIndex;
layout(location = 4) in vec3  inTangent;

layout(location = 0) out vec3 worldPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out uint fragMaterialIndex;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out mat3 txrxMatrix;
layout(location = 7) out float sysTime;
layout(location = 8) out vec3 tangent;


layout(binding = 1) uniform sampler2D material0_textures[16];
layout(binding = 2) uniform sampler2D material1_textures[16];
layout(binding = 3) uniform sampler2D material2_textures[16];
layout(binding = 4) uniform sampler2D material3_textures[16];

struct FhMaterialInfo
{
	uint colorAddress;
	uint normalAddress;
	uint roughnessAddress;
	uint metallicAddress;
	
	uint specularAddress;
	uint lightmapAddress;
	uint emissionAddress;
	uint displacementAddress;
	
	float colorScale;
	float normalScale;
	float roughnessScale;
	float metallicScale;
	
	float specularScale;
	float lightmapScale;
	float emissionScale;
	float displacementScale;
	
	float alphaDefault;
	float roughnessDefault;
	float metallicDefault;
	float specularDefault;
	
	vec4 colorDefault;
	vec4 emissionDefault;
};

layout(binding = 5) uniform FhGpassMaterialsInfo {
	FhMaterialInfo materialInfo[4];
} materialInfos;



//push constants block
layout( push_constant ) uniform constants
{
	mat4 model;
	uint type;
	uint reserve1;
	float reserve2;
	float reserve3;
} PushConstants;




float getDisplacement()
{
	int dispMapAddress = int(materialInfos.materialInfo[ inMaterialIndex ].displacementAddress)-1;
	float dispMapScale =	1;//  materialInfos.materialInfo[ inMaterialIndex ].displacementScale;

	switch(inMaterialIndex)
	{
		case 0: return dispMapAddress >= 0 ? texture(material0_textures[ dispMapAddress ], inUV * dispMapScale).x : 0;
		case 1: return dispMapAddress >= 0 ? texture(material1_textures[ dispMapAddress ], inUV * dispMapScale).x : 0;
		case 2: return dispMapAddress >= 0 ? texture(material2_textures[ dispMapAddress ], inUV * dispMapScale).x : 0;
		case 3: return dispMapAddress >= 0 ? texture(material3_textures[ dispMapAddress ], inUV * dispMapScale).x : 0;
	};
}


vec3 getNormalVert()
{
	int normMapAddress = int(materialInfos.materialInfo[ inMaterialIndex ].normalAddress)-1;
	float normMapScale = 1;//materialInfos.materialInfo[ inMaterialIndex ].normalScale;

	vec3 normal = inNormal; // from triangle

	if( normMapAddress >= 0 ) // if there is a map
	{
		switch( inMaterialIndex )
		{
			case 0: normal = texture( material0_textures[ normMapAddress ], inUV * normMapScale).xyz; break;
			case 1: normal = texture( material1_textures[ normMapAddress ], inUV * normMapScale).xyz; break;
			case 2: normal = texture( material2_textures[ normMapAddress ], inUV * normMapScale).xyz; break;
			case 3: normal = texture( material3_textures[ normMapAddress ], inUV * normMapScale).xyz; break;
		};
		//normal = (normal-vec3(0.5)) * 2;
		normal = (normalize(normal)-vec3(0.5)) * 2;
        
		vec3 N = mat3(PushConstants.model) * normalize(inNormal);        // повернутая матрицей модели нормаль ТРЕУГОЛЬНИКА
        vec3 T  = mat3(PushConstants.model) * normalize(inTangent);             // повернутый матрицей модели вектор tangent
        vec3 B  = mat3(PushConstants.model) * normalize(cross(N,T) );           // повернутый матрицей модели вычисленный вектор bitangent
		// T  = txrxMatrix * normalize(vec3(2,0,0));             // повернутый матрицей модели вектор tangent
        // B  = txrxMatrix * normalize(vec3(0,2,0));           // повернутый матрицей модели вычисленный вектор bitangent
        // N = txrxMatrix * normalize(vec3(0,0,1));

		mat3 TBN = mat3(T,B,N);                                               // матрица TBN
        normal = normalize(TBN * normal);                                       // мировые координаты!!!
		
	}
	
	else
	{
		normal = mat3(PushConstants.model) * fragNormal;
	}
	//return vec3(1);
	return normal;
}



void main() 
{
	vec4 WPos = vec4(inPosition, 1.0);
	//float disp = getDisplacement();


	//vec3 normal = getNormalVert();
		//normal = inNormal;
		//normal = (normal-vec3(0.5)) * 2;
	//WPos += vec4(normal*disp*0.2, 0);
	
	WPos = PushConstants.model * WPos;
	//vec4 disp 
	//int dispAddress = materialInfos.materialInfo[inMaterialIndex].displacementAddress-1;
	//dispAddress >= 0? texture(, inUV);
	
	//vec3 color = texture(material0_textures[ 0 ], inUV ).xyz;
	//vec3 norm = texture(material0_textures[ 1 ], inUV ).xyz;

	fragNormal = inNormal;
	
	worldPosition = WPos.xyz;
	
	gl_Position = commonData.proj * commonData.view * WPos;
    
	txrxMatrix = mat3(PushConstants.model);
	
	fragMaterialIndex = inMaterialIndex;	//int(inUVmat.z);//
	
	fragTexCoord = inUV;//.xy;
	//fragTexCoord = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1);
	
	sysTime = commonData.Time;
	
	int Type = int(PushConstants.type);

	tangent = inTangent;

}