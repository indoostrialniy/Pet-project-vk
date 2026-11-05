#version 450
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
layout(location = 9) out uint Type;
layout(location = 10) out struct VSOut
{
	uint v[48];	// прямая переиндексация 48 материалов, назначенных объекту
} vs_out;

#define INSTANCED_RENDERING

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

layout(binding = 0) uniform FhGeneralInfo 						// к binding = 0 будем ссылаться в descriptor layout
{
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
	float Time;
} commonData;

layout(binding = 1) uniform sampler2D materials_textures[2048]; 	// 70 materials * 16 samplers = 1120
layout(binding = 2) uniform FhGpassMaterialsInfo 
{
	FhMaterialInfo materialInfo[128];							// 70 materials description
} materialInfos;

struct PushConstantsStruct
{
	// minimum 128 bytes: 
	mat4 model;					// 64 bytes - entity model matrix
	uint type;					// 68 bytes
	uint uboOffset;				// 72 bytes
	float reserve2;				// 76 bytes
	float reserve3;				// 76 77 78 79 bytes
	uvec4 materialIndexes[3];
	//uint materialIndexes0;
	//uint materialIndexes1;
	//uint materialIndexes2;
	//uint materialIndexes3;
	//uint materialIndexes4;
	//uint materialIndexes5;
	//uint materialIndexes6;
	//uint materialIndexes7;
	//uint materialIndexes8;
	//uint materialIndexes9;
	//uint materialIndexes10;
	//uint materialIndexes11;
	//uint materialIndexes[3];	// We`ll pack here 48 short integers and in GLSL (read by 4 bytes straight) decomposite it using bitwise operatons	
};

layout(binding = 3) uniform FhGpassPushConstantsInfo 
{
	PushConstantsStruct constants[700];							// 64 push constants structs
} pushConstantsBuffer;


//push constants block
layout( push_constant ) uniform constants
{
	// minimum 128 bytes: 
	mat4 model;					// 64 bytes - entity model matrix
	uint type;					// 68 bytes
	uint uboOffset;				// 72 bytes
	float reserve2;				// 76 bytes
	float reserve3;				// 80 bytes
	uint materialIndexes[12];	// We`ll pack here 48 short integers and in GLSL (read by 4 bytes straight) decomposite it using bitwise operatons
} PushConstants;


void main() 
{
	int offset = int(PushConstants.uboOffset);
	//int offset = 0;
	
	PushConstantsStruct pushes = pushConstantsBuffer.constants[gl_InstanceIndex+offset];
	//PushConstantsStruct pushes = pushConstantsBuffer.constants[0];

	vec4 WPos = vec4(inPosition, 1.0);
	//float disp = getDisplacement();
	//vec3 normal = getNormalVert();
		//normal = inNormal;
		//normal = (normal-vec3(0.5)) * 2;
	//WPos += vec4(normal*disp*0.2, 0);
	
	WPos = pushes.model * WPos;
	
	//vec4 disp 
	//int dispAddress = materialInfos.materialInfo[inMaterialIndex].displacementAddress-1;
	//dispAddress >= 0? texture(, inUV);
	
	//vec3 color = texture(material0_textures[ 0 ], inUV ).xyz;
	//vec3 norm = texture(material0_textures[ 1 ], inUV ).xyz;

	fragNormal = inNormal;
	
	worldPosition = WPos.xyz;
	
	//gl_Position = commonData.proj * commonData.view * WPos;
    
	gl_Position = PushConstants.model * WPos; // PushConstants.model - proj*view
	
	txrxMatrix = transpose(inverse(mat3(pushes.model)));	//mat3(PushConstants.model);
	
	fragMaterialIndex = inMaterialIndex;	//int(inUVmat.z);//
	
	fragTexCoord = inUV;//.xy;
	//fragTexCoord = vec2(gl_VertexIndex & 1, (gl_VertexIndex >> 1) & 1);
	
	sysTime = commonData.Time;
	
	Type = uint(pushes.type);

	tangent = inTangent;

	
	
	#ifdef INSTANCED_RENDERING
		// unpack material indexes for instances
		// 	uvec4 materialIndexes[3]; - 4*4*3 = 48 bytes
		for(int v=0; v<3; v++)
		{
			for(int ve=0; ve<4; ve++)
			{
				//uint a = uint(vec4(1)[1]);
				uint BYTE = pushes.materialIndexes[v][ve]; // unpack 4 values from here
				vs_out.v[16*v + 4*ve + 0] = (BYTE >> 0)  & 0xFFu;
				vs_out.v[16*v + 4*ve + 1] = (BYTE >> 8)  & 0xFFu;
				vs_out.v[16*v + 4*ve + 2] = (BYTE >> 16) & 0xFFu;
				vs_out.v[16*v + 4*ve + 3] = (BYTE >> 24) & 0xFFu;
			}
			
		}
	#else
		// unpack material indexes
		for(int i=0; i<12; i++)
		{
			vs_out.v[4*i+0] = (PushConstants.materialIndexes[i] >> 0)  & 0xFFu;
			vs_out.v[4*i+1] = (PushConstants.materialIndexes[i] >> 8)  & 0xFFu;
			vs_out.v[4*i+2] = (PushConstants.materialIndexes[i] >> 16) & 0xFFu;
			vs_out.v[4*i+3] = (PushConstants.materialIndexes[i] >> 24) & 0xFFu;
		}
	#endif

}