#version 450

layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in flat uint fragMaterialIndex;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in mat3 txrxMatrix;
layout(location = 7) in float sysTime;
layout(location = 8) in vec3 tangent;

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


layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;
layout(location = 3) out vec4 outPBR;
layout(location = 4) out vec4 outEmission;

vec4 getColor()
{
	int colorMapAddress = int(materialInfos.materialInfo[ fragMaterialIndex ].colorAddress)-1;
	float colorMapScale =	  materialInfos.materialInfo[ fragMaterialIndex ].colorScale;

	switch(fragMaterialIndex)
	{
		case 0: return colorMapAddress >= 0 ? texture(material0_textures[ colorMapAddress ], fragTexCoord * colorMapScale) : vec4(materialInfos.materialInfo[ fragMaterialIndex ].colorDefault.rgb, 1);
		case 1: return colorMapAddress >= 0 ? texture(material1_textures[ colorMapAddress ], fragTexCoord * colorMapScale) : vec4(materialInfos.materialInfo[ fragMaterialIndex ].colorDefault.rgb, 1);
		case 2: return colorMapAddress >= 0 ? texture(material2_textures[ colorMapAddress ], fragTexCoord * colorMapScale) : vec4(materialInfos.materialInfo[ fragMaterialIndex ].colorDefault.rgb, 1);
		case 3: return colorMapAddress >= 0 ? texture(material3_textures[ colorMapAddress ], fragTexCoord * colorMapScale) : vec4(materialInfos.materialInfo[ fragMaterialIndex ].colorDefault.rgb, 1);
	};
}



float getDisplacement()
{
	int dispMapAddress = int(materialInfos.materialInfo[ fragMaterialIndex ].displacementAddress)-1;
	float dispMapScale = materialInfos.materialInfo[ fragMaterialIndex ].displacementScale;

	switch(fragMaterialIndex)
	{
		case 0: return dispMapAddress >= 0 ? texture(material0_textures[ dispMapAddress ], fragTexCoord * dispMapScale).x : 0;
		case 1: return dispMapAddress >= 0 ? texture(material1_textures[ dispMapAddress ], fragTexCoord * dispMapScale).x : 0;
		case 2: return dispMapAddress >= 0 ? texture(material2_textures[ dispMapAddress ], fragTexCoord * dispMapScale).x : 0;
		case 3: return dispMapAddress >= 0 ? texture(material3_textures[ dispMapAddress ], fragTexCoord * dispMapScale).x : 0;
	};
}

vec3 getEmission()
{
	int emissionMapAddress = int(materialInfos.materialInfo[ fragMaterialIndex ].emissionAddress)-1;
	float emissionMapScale = materialInfos.materialInfo[ fragMaterialIndex ].emissionScale;

	if( emissionMapAddress >= 0 )
	{
		switch(fragMaterialIndex)
		{
			case 0: return texture(material0_textures[ emissionMapAddress ], fragTexCoord * emissionMapScale).rgb;
			case 1: return texture(material1_textures[ emissionMapAddress ], fragTexCoord * emissionMapScale).rgb;
			case 2: return texture(material2_textures[ emissionMapAddress ], fragTexCoord * emissionMapScale).rgb;
			case 3: return texture(material3_textures[ emissionMapAddress ], fragTexCoord * emissionMapScale).rgb;
		};
	}
	else
	{
		return vec3(0,0,0);
	}
}


vec3 getNormalFrag()
{
	int normMapAddress = int(materialInfos.materialInfo[ fragMaterialIndex ].normalAddress)-1;
	float normMapScale =  materialInfos.materialInfo[ fragMaterialIndex ].normalScale;

	vec3 normal = fragNormal;//normalize(vec3(0, 0, 1));
	
	if( normMapAddress >= 0 ) // if there is a map
	{
		switch(fragMaterialIndex)
		{
			case 0: normal = texture(material0_textures[ normMapAddress ], fragTexCoord * normMapScale).xyz; break;
			case 1: normal = texture(material1_textures[ normMapAddress ], fragTexCoord * normMapScale).xyz; break;
			case 2: normal = texture(material2_textures[ normMapAddress ], fragTexCoord * normMapScale).xyz; break;
			case 3: normal = texture(material3_textures[ normMapAddress ], fragTexCoord * normMapScale).xyz; break;
		};

		//normal = vec3(0.5, 0.5, 1.0);
		normal = ( normal-vec3(0.5) ) * 2;
        
		vec3 N = txrxMatrix * normalize(fragNormal);        // повернутая матрицей модели нормаль ТРЕУГОЛЬНИКА
        vec3 T  = txrxMatrix * normalize(tangent);         //+vec3(0.001, 0.0001, 0.001)    // повернутый матрицей модели вектор tangent
        vec3 B  = txrxMatrix * normalize(cross(N,T) );           // повернутый матрицей модели вычисленный вектор bitangent
		// T  = txrxMatrix * normalize(vec3(2,0,0));             // повернутый матрицей модели вектор tangent
        // B  = txrxMatrix * normalize(vec3(0,2,0));           // повернутый матрицей модели вычисленный вектор bitangent
        // N = txrxMatrix * normalize(vec3(0,0,1));

		mat3 TBN = mat3(T,B,N);                                               // матрица TBN
        normal = normalize(TBN * normal);                                       // мировые координаты!!!
		//normal = vec3(0,0,1);
	}
	else
	{
		normal = txrxMatrix * fragNormal;
	}
	return normal;
}


float getMetallic()
{
	int metalMapAddress = int(materialInfos.materialInfo[ fragMaterialIndex ].metallicAddress)-1;
	float metalMapScale = materialInfos.materialInfo[ fragMaterialIndex ].metallicScale;
	
	if(metalMapAddress >= 0)
	{
		switch(fragMaterialIndex)
		{
			case 0: return texture(material0_textures[ metalMapAddress ], fragTexCoord * metalMapScale).x; 
			case 1: return texture(material1_textures[ metalMapAddress ], fragTexCoord * metalMapScale).x;
			case 2: return texture(material2_textures[ metalMapAddress ], fragTexCoord * metalMapScale).x;
			case 3: return texture(material3_textures[ metalMapAddress ], fragTexCoord * metalMapScale).x;
		};
	}
	else {return 0;}
}

float getRoughness()
{
	int roughMapAddress = int(materialInfos.materialInfo[ fragMaterialIndex ].roughnessAddress)-1;
	float roughMapScale = materialInfos.materialInfo[ fragMaterialIndex ].roughnessScale;
	
	if(roughMapAddress >= 0)
	{
		switch(fragMaterialIndex)
		{
			case 0: return texture(material0_textures[ roughMapAddress ], fragTexCoord * roughMapScale).x; 
			case 1: return texture(material1_textures[ roughMapAddress ], fragTexCoord * roughMapScale).x;
			case 2: return texture(material2_textures[ roughMapAddress ], fragTexCoord * roughMapScale).x;
			case 3: return texture(material3_textures[ roughMapAddress ], fragTexCoord * roughMapScale).x;
		};
	}
	else {return 0;}
}

void main() {
    // color 0
	// normal 1

	//float colorMapAddress = materialInfos.materialInfo[ fragMaterialIndex ].colorAddress-1;
	//float normMapAddress = materialInfos.materialInfo[ fragMaterialIndex ].normalAddress-1;


	//outNormal = vec4(texture(material0_textures[ 1 ], fragTexCoord ).xyz, 1);
	//outNormal = texture(material0_textures[ 1 ], fragTexCoord );

	

	//float disp = getDisplacement();

	//outColor = vec4(fragNormal, 1);
	//outNormal = vec4(fragNormal, 1);//vec4(outColor.xyz, 1);	//

	outColor = getColor();
	outNormal = vec4(getNormalFrag(),1);

	outPosition = vec4(worldPosition,1);


	outPBR = vec4(getMetallic(), getRoughness(), 0, 1);
	outEmission = vec4(getEmission(), 1);
	
	//outNormal = vec4(materialInfos.materialInfo[0].emissionDefault.rgb, 1);
}