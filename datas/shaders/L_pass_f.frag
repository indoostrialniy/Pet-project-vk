#version 450
layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

layout(binding = 0) uniform sampler2D maps[5]; // color, normal, position, PBR, emission
#define f3_BUF_COLOR(uv) texture(maps[0], uv).rgb
#define f3_BUF_NORMAL(uv) texture(maps[1], uv).xyz
#define f3_BUF_POSITION(uv) texture(maps[2], uv).xyz
#define f_BUF_METALLIC(uv) texture(maps[3], uv).r
#define f_BUF_ROUGHNESS(uv) texture(maps[3], uv).g
#define f_BUF_SPECULAR(uv) texture(maps[3], uv).b
#define f3_BUF_EMISSION(uv) texture(maps[4], uv).rgb


#define SHADOWS_ON

//struct FhDynamicLightSourceInfo
//{
//	vec3 position;
//	vec3 direction;
//	vec3 color;
//	uint type;
//	float sector;
//	float radius;
//	float power;
//	float blendWidth;
//	uint exists;
//	mat4 PV[6];
//};

/*
struct FhDynamicLightSourceInfo 
{
    mat4 PV[6];
    vec3 position;             // 12
    vec3 direction;            // 12
    vec3 color;                // 12

    float radius;                   // 4
    float power;                    // 4
    float sector;                   // 4
    float blendWidth;               // 4
    uint type;                  // 4
    uint exists;                // 4

    bool padding[3];
};
*/

struct FhDynamicLightSourceInfo //  6 версия
{
	vec3 position;             // Вектор:  0  1  2  3    4  5  6  7    8  9 10 11
    vec3 direction;            // Вектор: 16 17 18 19   20 21 22 23   24 25 26 27
    vec3 color;                // Вектор: 32 33 34 35   36 37 38 39   40 41 42 43

    float radius;               // Число: 44 45 46 47 
	float power;                // Число: 48 49 50 51								 
	float sector;				// Число: 52 53 54 55								 
	float blendWidth;			// Число: 56 57 58 59						 
	uint type;					// Число: 60 61 62 63					 
	uint exists; 				// Число: 64 65 66 67
				                  
	mat4 PV[6]; 				// Числа: 80 - 463
								// уже 464 байта, надо добить до 512, добавив 48 байт
								// но в std140 массив float выравнивается по 16 байт
								// т.е. 16 байт на ячейку массива. Поэтому нужно тут указать не 12 ячеек, как в C++
								// а в 4 раза меньше - т.е. 3!
	float reserve[3];
};

layout(binding = 1, std140) uniform DynamicLightSourceInfo {
	FhDynamicLightSourceInfo illuminationInfo[16];
} illuminationInfos;


layout(binding = 2) uniform FhGeneralInfo {
	mat4 view;
	mat4 proj;
	vec3 cameraPos;
	float Time;
} commonData;

layout(binding = 3) uniform sampler2D shadowMapsSpots[16]; // spots
layout(binding = 4) uniform samplerCube shadowMapsOmni[16]; //использование samplerCubeShadow позволит аппаратно переложить на него выбор нужной грани куба. Нужен только вектор до точки от источника?


const float PI = 3.14159265359;

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggxV = geometrySchlickGGX(NdotV, roughness);
    float ggxL = geometrySchlickGGX(NdotL, roughness);
    return ggxV * ggxL;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float shadowCalculation(int index)
{
	vec3 worldPos = f3_BUF_POSITION(fragTexCoord);	//texture(gPosition, fragUV).rgb;
	vec3 lightPos = illuminationInfos.illuminationInfo[index].position;

	float lightFarDistance = illuminationInfos.illuminationInfo[index].radius;

	float currentDepth = length(worldPos - lightPos) / lightFarDistance;

	vec4 POS = vec4( f3_BUF_POSITION(fragTexCoord), 1 );		// world coordinate
	POS = illuminationInfos.illuminationInfo[index].PV[0] * POS;	// light POV coordinate
	POS.xyz /= POS.w;											// NDC

	POS.xy = POS.xy*vec2(0.5)+vec2(0.5);	// shadowmap coordinates

	if(POS.x > 1 || POS.x < 0 || POS.y > 1 || POS.y < 0) {return 0.0;}

	vec3 dir = normalize(worldPos - lightPos);
	//POS.y = 1-POS.y;

	float shadow;
	//currentDepth = POS.z;
	//switch(illuminationInfos.illuminationInfo[index].type)
	//{
		//case 0 : shadow = texture(shadowMaps[index], vec3(dir)).r;break;
		//case 1 : 
	shadow = texture(shadowMapsSpots[index], POS.xy ).x;// break;// vec4(POS.xy,0,1)
	//}
	
	float bias = 0.005;

	//outColor = vec4(texture(shadowMaps[index], POS.xy).rrr,1);

	return shadow  < currentDepth - bias ? 0.03 : 0.9 ;
}

float shadowCube(int index) 
{
    vec3 worldPos = f3_BUF_POSITION(fragTexCoord);
	vec3 lightPos = illuminationInfos.illuminationInfo[index].position;
	
	float lightFarDistance = illuminationInfos.illuminationInfo[index].radius;
	float bias = 0.000001;

	// omni
	vec3 dir = normalize(worldPos - lightPos);

	float currentDepth = length(worldPos - lightPos) / lightFarDistance;

	float shadow = texture(shadowMapsOmni[index], vec3(dir)).r;
		
	return shadow < currentDepth - bias ? 0.0 : 0.99 ;
}

float calculateShadow(int index)
{
	//return shadowCube(index);
	//return shadowCalculation(index);

	float lightFarDistance = illuminationInfos.illuminationInfo[index].radius;
	float bias = 0.005;

	vec3 worldPos = f3_BUF_POSITION(fragTexCoord);	//texture(gPosition, fragUV).rgb;
	vec3 lightPos = illuminationInfos.illuminationInfo[index].position;

	// omni
	vec3 dir = normalize(worldPos - lightPos);

	// spot
	vec4 POS = vec4( f3_BUF_POSITION(fragTexCoord), 1 );		// world coordinate
	POS = illuminationInfos.illuminationInfo[index].PV[0] * POS;	// light POV coordinate
	POS.xyz /= POS.w;											// NDC
	POS.xy = POS.xy*vec2(0.5)+vec2(0.5);	// shadowmap coordinates
	if(POS.x > 1 || POS.x < 0 || POS.y > 1 || POS.y < 0) {return 0.0;}

	float currentDepth = length(worldPos - lightPos) / lightFarDistance;

	float shadow;

	switch(illuminationInfos.illuminationInfo[index].type)
	{
		case 0 : shadow = texture(shadowMapsOmni[index], vec3(dir)).r; break;
		case 1 : shadow = texture(shadowMapsSpots[index], POS.xy ).x; break;
	}
	//shadow = texture(shadowMapsOmni[index], vec3(dir)).x;

	return shadow < currentDepth - bias ? 0.1 : 0.9 ;
}

vec3 getLightSourceInfluence(int index)
{
	vec3 cameraPos2 = commonData.cameraPos;	//vec3(14.087, -14.19, 7.33);

	vec3 fragWorldPos = f3_BUF_POSITION(fragTexCoord);
	vec3 lightPos = illuminationInfos.illuminationInfo[index].position;

	float distance = length(lightPos - fragWorldPos);

	

	

	vec3 albedo = f3_BUF_COLOR(fragTexCoord);// * material.baseColorFactor.rgb;
    

	float metallic = f_BUF_METALLIC(fragTexCoord);//0.5;//texture(metallicTex, fragTexCoord).r * material.metallicFactor;
    float roughness = f_BUF_ROUGHNESS(fragTexCoord);//0.025;//texture(roughnessTex, fragTexCoord).r * material.roughnessFactor;
    vec3 N = normalize(f3_BUF_NORMAL(fragTexCoord));

	vec3 ambient = vec3(0.0) * albedo;

	float lightVolumeRadius = 7;//illuminationInfos.illuminationInfo[index].radius;	//радиус влияния источника света
	if( distance > lightVolumeRadius )
	{
		//return (ambient/(ambient+1.0))*(ambient/(ambient+1.0));//
		return vec3(0,0,0);//

		// finalColor = finalColor / (finalColor + 1.0);
		//finalColor = pow(finalColor, vec3(1.0 / 2.2));
		//return finalColor*finalColor;
	}

    vec3 V = normalize(f3_BUF_POSITION(fragTexCoord)-cameraPos2); 
	
	vec3 lightColor = illuminationInfos.illuminationInfo[index].color;
	float lightIntensity = illuminationInfos.illuminationInfo[index].power*4;


	//if(index == 2) lightIntensity*=100;

	vec3 lightSourceDir = normalize(illuminationInfos.illuminationInfo[index].direction);

    vec3 L = normalize(lightPos - fragWorldPos);
    vec3 H = normalize(V + L);
    float attenuation = 1.0 / (distance * distance);


    float Kc = 1.00; //для истанции 32 метра
    float Kl = 4.5/lightVolumeRadius;
    float Kq = pow(9/lightVolumeRadius,2);
    float fatt = 1 / ( Kc + Kl*distance + Kq*distance*distance );
	attenuation = fatt;

	float d = distance;
	float t = 1.0 - d/lightVolumeRadius;
	attenuation = smoothstep(0.0, 1.0, t);

	//attenuation = 0.5;

    vec3 radiance = lightColor * lightIntensity * attenuation;




    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
		F = vec3(0.2);//fresnelSchlick(clamp(dot(H, V),0,1), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; 

    float NdotL = max(dot(N, L), 0.0);

    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

		
	if( illuminationInfos.illuminationInfo[index].type == 1)
	{
		float spotSectorAngleFullDark = illuminationInfos.illuminationInfo[index].sector; // out there is total no light
			float blendWidthDegree = 25;	// width of the fade of the spot
			float spotSectorAngleFullBright  = spotSectorAngleFullDark-blendWidthDegree;
		float spot = smoothstep(cos(spotSectorAngleFullDark*PI/180), cos(spotSectorAngleFullBright*PI/180), dot(lightSourceDir, -L) );

		Lo *= spot;
	}

    

    vec3 finalColor = ambient + Lo;
		//finalColor *= spot;
    finalColor = finalColor / (finalColor + 1.0);
    //finalColor = pow(finalColor, vec3(1.0 / 2.2));
	return finalColor*finalColor;
}

void main() 
{
	outColor = vec4(f3_BUF_COLOR(fragTexCoord), 1);//texture(maps[0], fragTexCoord);
	//vec3 normal = f3_BUF_NORMAL(fragTexCoord);
	//FhDynamicLightSourceInfo info = illuminationInfos.illuminationInfo[0];
	//vec3 position = f3_BUF_POSITION(fragTexCoord);
	
	//vec3 finalColor0 = getLightSourceInfluence(0);
	//vec3 finalColor1 = getLightSourceInfluence(1);
	
	vec3 finalColor = outColor.rgb*0.02;
	//int l=1;
	
	for(int l=0; l<16; l++)
	{
		//if(l==0) continue;	// skip player torch
		
		if( illuminationInfos.illuminationInfo[l].exists != 1) continue;	// discard non existing lightsources
		
		vec3 color = getLightSourceInfluence(l);
		
		#ifdef SHADOWS_ON
			if( l != 0 ) color *= illuminationInfos.illuminationInfo[l].type == 1 ? shadowCalculation(l) : shadowCube(l);
			//if(l==1) color *= illuminationInfos.illuminationInfo[l].type == 1 ? shadowCalculation(1) : shadowCube(1);//
			//if(l==2) color *= illuminationInfos.illuminationInfo[l].type == 1 ? shadowCalculation(2) : shadowCube(2);//
		#endif
		//	//
		
		finalColor += color;
		
	}

	outColor = vec4(sqrt(finalColor), 1.0);


	//float r = sqrt( pow(finalColor0.r, 2) + pow(finalColor1.r, 2) );
	//float g = sqrt( pow(finalColor0.g, 2) + pow(finalColor1.g, 2) );
	//float b = sqrt( pow(finalColor0.b, 2) + pow(finalColor1.b, 2) );

	//r,g,b);
		//finalColor +=finalColor0;
		//finalColor +=finalColor1;
	//shadowCube();

	//outColor = vec4( texture(shadowMaps[1], fragTexCoord).xxx, 1);

	//for(int i=0; i<16; i++)
	{
		//vec4 shadmap0 = texture(shadowMaps[i], fragTexCoord);
	}
	


	outBloom = vec4(f3_BUF_EMISSION(fragTexCoord),1);

	if(outColor.r > 0.5)
	{
		outBloom += outColor;
	}
	else
	{
		//outBloom = vec4(1,0,0,1);
	}
}