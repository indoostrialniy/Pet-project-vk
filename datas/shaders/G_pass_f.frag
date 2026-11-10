#version 450

layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in flat uint fragMaterialIndex;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in mat3 txrxMatrix;
layout(location = 7) in float sysTime;
layout(location = 8) in vec3 tangent;
layout(location = 9) in flat uint Type;
layout(location = 10) in vec3 POV;
layout(location = 11) in flat struct VSOut
{
	uint v[48];
} fs_in;



layout(binding = 1) uniform sampler2D material0_textures[2048];


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

layout(binding = 2) uniform FhGpassMaterialsInfo 
{
	FhMaterialInfo materialInfo[128];
} materialInfos;


layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;
layout(location = 3) out vec4 outPBR;
layout(location = 4) out vec4 outEmission;


float getAlpha()
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	
	return info.alphaDefault;
}


vec4 getColor()
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	int colorMapAddress = int(info.colorAddress)-1;
	//float colorScale = info.colorScale;

	if(colorMapAddress >= 0)
	{
		return texture(material0_textures[ 16 * globalMaterialIndex  + colorMapAddress ], fragTexCoord * info.colorScale ) ;
	}
	else
	{
		return vec4(info.colorDefault.rgb, getAlpha());
	}
}



vec3 getNormalFrag()
{
	vec3 normal = fragNormal;//normalize(vec3(0, 0, 1));
	
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	int normalMapAddress = int(info.normalAddress)-1;
	float normalScale = info.normalScale;

	if(normalMapAddress >= 0)
	{
		//normal = texture(material0_textures[ 16 * globalMaterialIndex  + normalMapAddress ], fragTexCoord * normalScale ).rgb ;
		//normal = ( normal-vec3(0.5) ) * 2;
		//vec3 N = txrxMatrix * normalize(fragNormal);        // повернутая матрицей модели нормаль ТРЕУГОЛЬНИКА
        //vec3 T  = txrxMatrix * normalize(tangent);         //+vec3(0.001, 0.0001, 0.001)    // повернутый матрицей модели вектор tangent
        //vec3 B  = txrxMatrix * normalize(cross(N,T) );           // повернутый матрицей модели вычисленный вектор bitangent
		//mat3 TBN = mat3(T,B,N);                                               // матрица TBN
        //normal = normalize(TBN * normal);                                       // мировые координаты!!!

		// txrxMatrix — готовая transpose(inverse(mat3(model))) с CPU, не inverse на месте
		vec3 N = normalize(txrxMatrix * fragNormal);
		vec3 T = normalize(txrxMatrix * (tangent.xyz - fragNormal * dot(tangent.xyz, fragNormal)));
		vec3 B = cross(N, T);// * tangent.w;

		mat3 TBN = mat3(T, B, N);
		normal = normalize(TBN * ((texture(material0_textures[16 * globalMaterialIndex  + normalMapAddress], fragTexCoord * normalScale).rgb) * 2.0 - 1.0));
	}
	else
	{
		normal = txrxMatrix * fragNormal;
	}	
	return normalize(normal);
}

vec4 getColorTriplanar()
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	int colorMapAddress = int(info.colorAddress)-1;
	float colorScale = info.colorScale;

	if(colorMapAddress >= 0)
	{
		vec3 n = getNormalFrag();
		float nx = abs(n.x);
		float ny = abs(n.y);
		float nz = abs(n.z);
		float nxyz = nx + ny + nz;

		float vx = nx/nxyz;
		float vy = ny/nxyz;
		float vz = nz/nxyz;
		float scale = colorScale;

		vec2 Xplane = worldPosition.yz * scale * vec2(1,1);
		vec2 Yplane = worldPosition.xz * scale * vec2(1,1);
		vec2 Zplane = worldPosition.xy * scale * vec2(1,1);
		
		vec4 cx = texture(material0_textures[ 16 * globalMaterialIndex  + colorMapAddress ], Xplane );
		vec4 cy = texture(material0_textures[ 16 * globalMaterialIndex  + colorMapAddress ], Yplane );
		vec4 cz = texture(material0_textures[ 16 * globalMaterialIndex  + colorMapAddress ], Zplane );

		return cx*vx + cy*vy + cz*vz;
	}
	else
	{
		return vec4(info.colorDefault.rgb, 1);
	}
}

float getMetallic()
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	int metalMapAddress = int(info.metallicAddress)-1;

	if(metalMapAddress >= 0)
	{
		return texture(material0_textures[ 16 * globalMaterialIndex  + metalMapAddress ], fragTexCoord ).r ;
	}
	else
	{
		return info.metallicDefault;
	}
}

float getRoughness()
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	int roughMapAddress = int(info.roughnessAddress)-1;

	if(roughMapAddress >= 0)
	{
		return texture(material0_textures[ 16 * globalMaterialIndex  + roughMapAddress ], fragTexCoord ).r ;
	}
	else
	{
		return info.roughnessDefault;
	}
}

vec3 getEmission()
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	int emissionMapAddress = int(info.emissionAddress)-1;

	if(emissionMapAddress >= 0)
	{
		return texture(material0_textures[ 16 * globalMaterialIndex  + emissionMapAddress ], fragTexCoord ).rgb ;
	}
	else
	{
		return info.emissionDefault.rgb;
	}
}

float getDisplacement()
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	int dispMapAddress = int(info.displacementAddress)-1;

	if(dispMapAddress >= 0)
	{
		return texture(material0_textures[ 16 * globalMaterialIndex  + dispMapAddress ], fragTexCoord ).r ;
	}
	else
	{
		return 0;
	}
}

void main() 
{
	//for(int m=0; m<64; m++)
	{
		//vec4 color = texture(material0_textures[m], fragTexCoord);
		
		//FhMaterialInfo str = materialInfos.materialInfo[m];
	}
	
	//if( length(POV-worldPosition) < 10)
	{
		uint TRIPLANAR = 0;
		TRIPLANAR = (Type >> 4)  & 0xFFu; // нужен 4-й бит

		float alpha = getAlpha();

		outColor = TRIPLANAR == 1 ? vec4(getColorTriplanar().xyz, alpha) : vec4(getColor().xyz, alpha);
		outNormal = vec4(getNormalFrag(), 1); 
		//outNormal = vec4( txrxMatrix*fragNormal, 1);
		outPosition = vec4(worldPosition, 1);
		outPBR = vec4(getMetallic(), getRoughness(), 0, 1);
		outEmission = vec4(getEmission(), 1);
	}
	/*
	else
	{
		outColor = vec4(getColor().xyz, 1);
		outNormal = vec4(fragNormal, 1);
		outPosition = vec4(worldPosition, 1);
		outPBR = vec4(0, 1, 0, 1);
		outEmission = vec4(0, 0, 0, 1);
	}
	*/
	
	//outColor = vec4(0, 1, 0, 1);
	//outNormal = vec4(fragNormal, 1);
	//outPosition = vec4(worldPosition, 1);
	//outPBR = vec4(0, 1, 0, 1);
	//outEmission = vec4(0, 0, 0, 1);
}