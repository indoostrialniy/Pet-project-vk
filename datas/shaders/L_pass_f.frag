#version 450
layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

layout(binding = 0) uniform sampler2D maps[5]; // color, normal, position, PBR, emission
layout(binding = 5) uniform usampler2D imaps[1]; // ID
#define f3_BUF_COLOR(uv) texture(maps[0], uv).rgb
#define f3_BUF_NORMAL(uv) texture(maps[1], uv).xyz
#define f3_BUF_POSITION(uv) texture(maps[2], uv).xyz
#define f_BUF_METALLIC(uv) texture(maps[3], uv).r
#define f_BUF_ROUGHNESS(uv) texture(maps[3], uv).g
#define f_BUF_SPECULAR(uv) texture(maps[3], uv).b
#define f3_BUF_EMISSION(uv) texture(maps[4], uv).rgb
#define i_BUF_ID(uv) texture(imaps[0], uv).r


#define SHADOWS_ON

#define LIGHT_TYPE_OMNI 0
#define LIGHT_TYPE_SPOT 1
#define LIGHT_TYPE_SUN 2
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
	float bias = 0.005;
	//float shadow;
	//currentDepth = POS.z;
	//switch(illuminationInfos.illuminationInfo[index].type)
		//case 0 : shadow = texture(shadowMaps[index], vec3(dir)).r;break;
		//case 1 : 
	
	int border = 2;
	int step1 = 1;
	float texelSize = 1.0/2048.0; // /8;
	int count = 1;
	float shadow = 0.0;

	float sh;
	//for(int i=-border; i<= border; i+=step1)
	{
		//for(int j=-border; j<= border; j+=step1)
		{
			int i, j=0;
			shadow = texture(shadowMapsSpots[index], POS.xy + vec2(i,j)*texelSize ).x;// break;// vec4(POS.xy,0,1)
			shadow =  (shadow  < currentDepth - bias ? 0.0 : 1.0);// * sin(POS.y*texelSize*1000000*3.14/180) ;
			// texture(maps[1], fragTexCoord + vec2(i,j)*texelSize).rgb;
			count++;
		}
	}
	//shadow /= count;	


		

	return shadow;
}

float smoothShadow(int index)
{
	int border = 3;
	int step1 = 2;
	float texelSize = 1.0/768.0;
	int count = 1;
	float shadow = 0.0;
	for(int i=-border; i<= border; i+=step1)
	{
		for(int j=-border; j<= border; j+=step1)
		{
			shadow += shadowCalculation(index);
			// texture(maps[1], fragTexCoord + vec2(i,j)*texelSize).rgb;
			count++;
		}
	}
	shadow /= count;
	
	return shadow;
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
		
	return shadow < currentDepth - bias ? 0.0 : 1.0 ;
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

	float lightVolumeRadius = illuminationInfos.illuminationInfo[index].radius;	//радиус влияния источника света
	if( distance > lightVolumeRadius )
	{
		//return (ambient/(ambient+1.0))*(ambient/(ambient+1.0));//
		return vec3(0,0,0);//

		// finalColor = finalColor / (finalColor + 1.0);
		//finalColor = pow(finalColor, vec3(1.0 / 2.2));
		//return finalColor*finalColor;
	}

	

    vec3 V = normalize(f3_BUF_POSITION(fragTexCoord)-cameraPos2); 
	
	vec3 lightColor = illuminationInfos.illuminationInfo[index].color; // pink
	

	float lightIntensity = illuminationInfos.illuminationInfo[index].power;


	//if(index == 2) lightIntensity*=100;

	vec3 lightSourceDir = normalize(illuminationInfos.illuminationInfo[index].direction);
	float attenuation = 1.0 / (distance * distance);

    vec3 L = normalize(lightPos - fragWorldPos);

	

	float d = distance;
	float t = 1.0 - d/lightVolumeRadius;
	attenuation = smoothstep(0.0, 1.0, t);

	if(illuminationInfos.illuminationInfo[index].type == 2) // if ortho sun
	{
		L = -lightSourceDir;  // rays are parallel
		attenuation = 1.0;		// no attenuation
		//lightIntensity *= 2;
		lightColor = vec3(1, 0.958, 0.541);//255;
	}

    vec3 H = normalize(V + L);
    


    //float Kc = 1.00; //для истанции 32 метра
    //float Kl = 4.5/lightVolumeRadius;
    //float Kq = pow(9/lightVolumeRadius,2);
    //float fatt = 1 / ( Kc + Kl*distance + Kq*distance*distance );
	//attenuation = fatt;

	

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
	
	
		
	if( illuminationInfos.illuminationInfo[index].type == 1 
		|| illuminationInfos.illuminationInfo[index].type == 2 )
	{
		float spotSectorAngleFullDark = illuminationInfos.illuminationInfo[index].sector; // out there is total no light
			float blendWidthDegree = 25;	// width of the fade of the spot
			float spotSectorAngleFullBright  = spotSectorAngleFullDark-blendWidthDegree;
		float spot = smoothstep(cos(spotSectorAngleFullDark*PI/180), cos(spotSectorAngleFullBright*PI/180), dot(lightSourceDir, -L) );

		Lo *= spot;
	}

    //return Lo;

    vec3 finalColor = ambient*0 + Lo;
		//finalColor *= spot;
    //finalColor = finalColor / (finalColor + 1.0);
    //finalColor = pow(finalColor, vec3(1.0 / 2.2));
	return finalColor;//*finalColor;
}

void main()
{
	uint id = uint(i_BUF_ID(fragTexCoord));
	vec3 N = normalize(f3_BUF_NORMAL(fragTexCoord));// нормаль фрагмента

	vec3 albedo = f3_BUF_COLOR(fragTexCoord); 		// линейный RGB [0..1]

	vec3 resultColor = albedo * 0.05;				// ambient

	float height = 0; // высота плоскости реки
	if(id==12) height = -0.74;
	if(id==13) height = 5.74;
	
	
	//if(id==12 || id==13) discard;

	

	
	vec3 cameraPos2 = commonData.cameraPos;
	float metallic = f_BUF_METALLIC(fragTexCoord);
	float roughness = f_BUF_ROUGHNESS(fragTexCoord);
	vec3 V = normalize( cameraPos2 - f3_BUF_POSITION(fragTexCoord) ); 
	vec3 F0 = vec3(0.04);
		F0 = mix(F0, albedo, metallic);

	for(int index=0; index<16; index++)
	{
		//break;
		//if(index == 1) break;
		//vec3 lightInfluence = getLightSourceInfluence(index);
		//if(index != 0) lightInfluence *= shadowCalculation(index);
		//resultColor += lightInfluence;
	
		//break;//continue;

		FhDynamicLightSourceInfo info = illuminationInfos.illuminationInfo[index];
		
		// first filter - by source status
		if( info.exists != 1) continue;	// skip non-active lightsources

		// second filter - by distance
		float 	lightVolumeRadius 	= info.radius;	//радиус влияния источника света
		vec3 	fragWorldPos 		= f3_BUF_POSITION(fragTexCoord);
		vec3 	lightPos 			= info.position;
		if( length(lightPos - fragWorldPos) > lightVolumeRadius ) continue;	// if fragment is too far from lightsource, skip it
		
		// third filter - by shadow
		float shadow = shadowCalculation(index);	//
		////if(id == 12 || id == 13) shadow = 1; // свет должен проникать под воду
		if(index == 0) shadow = 1.0;
		if(shadow < 0.05)		continue;			// if fragment is shadowed (with bias), also skip it
		

		// otherwise, if fragment is in radius and not shadowed
		vec3 L = normalize(lightPos - fragWorldPos);
		vec3 	lightSourceDir 		= normalize(info.direction);
		int		lightType			= int(info.type);
		
		float d = length(lightPos - fragWorldPos);
		float t = 1.0 - d/lightVolumeRadius;
		float attenuation = smoothstep(0.0, 1.0, t);

		if( lightType == LIGHT_TYPE_SUN) // if ortho sun
		{
			L = -normalize(lightSourceDir);  // sun rays are parallel
			attenuation = 1.0;		// no attenuation from sun
		}

		vec3 	lightColor 			= info.color; // pink
				//lightColor = vec3(vec3(1, 0.958, 0.541));

		float 	lightIntensity 		= info.power; // Это ватты, как в Blender сила источника Strength
				//lightIntensity *= 1/sqrt(2);

		vec3 E = vec3(0); // освещенность
		if( lightType == LIGHT_TYPE_OMNI || lightType == LIGHT_TYPE_SPOT)
		{
			// 1. Поток -> сила света (если P задаёшь как в Blender, в ваттах)
			vec3 I = lightColor * (lightIntensity / (4.0 * PI));   // Вт/ср
			// 2. Сила света -> освещённость (только для точечных источников)
			float d = max(d, 0.01);                          // защита от d=0
			E = I / (d * d);                                   // Вт/м²
			// color, attenuation, intensity
		}

		if( lightType == LIGHT_TYPE_SUN)
		{
			// Для солнца (directional) затухания нет — там сразу освещённость:
			E = lightColor * lightIntensity;
		}
		
		if( lightType == LIGHT_TYPE_SPOT)
		{
			
			float spotSectorAngleFullDark = info.sector; // out there is total no light
				float blendWidthDegree = 25;	// width of the fade of the spot
				float spotSectorAngleFullBright  = spotSectorAngleFullDark - blendWidthDegree;
			float spot = smoothstep(cos(spotSectorAngleFullDark*PI/180), cos(spotSectorAngleFullBright*PI/180), dot(lightSourceDir, -L) );

			shadow *= spot;	
		}
		
		// fourth filter - by power
		float lightStrength = attenuation * lightIntensity;
		//if(lightStrength < 0.05) continue;				// if lightsource is too weak

		// derivatives
		vec3 H = normalize(V + L);

		float NDF = distributionGGX(N, H, roughness);
		float G = geometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
			F = vec3(0.2);
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic; 

		vec3 waterColor = vec3(0.084, 0.465, 0.149);
		float k = 1;// =
		if(id == 12 || id == 13) 
		{
			albedo = f3_BUF_COLOR(fragTexCoord+0.02*vec2(N.xy)); // distortion
			
			
			
			float depth = abs( height-f3_BUF_POSITION(fragTexCoord).z );
			float viewDepth = length( f3_BUF_POSITION(fragTexCoord) - commonData.cameraPos);
			 smoothstep(-5, height, -depth);//f3_BUF_POSITION(fragTexCoord).z);// /3; //
				k = smoothstep( height-3, height, f3_BUF_POSITION(fragTexCoord).z);

			k = pow(k, 0.97);
			//albedo += waterColor*k;
			//albedo = 1*vec3(k);
			//resultColor = mix(waterColor*0.4, f3_BUF_COLOR(fragTexCoord)*0.1, k );
		//resultColor = mix(waterColor*0.5*shadow, resultColor, k );
				//resultColor = waterColor;
			//outColor = vec4(vec3(f3_BUF_COLOR(fragTexCoord)), 1);
			//return;
		}

		vec3 diffuse = (kD * albedo / PI + specular) * max(dot(N,L), 0.0) * E  ;
			 diffuse = (kD * albedo / PI + specular) * max(dot(N,L), 0.0) * lightColor * lightStrength ;
			//diffuse = mix

		//diffuse = mix((waterColor)*0.5, diffuse*1, k );
		//if(index != 0) 
		resultColor += shadow * diffuse;//*;
		
		//outColor = vec4(vec3( k ), 1);
		//break;
		//outColor = vec4(diffuse, 1);
	}

	

	
	resultColor += f3_BUF_EMISSION(fragTexCoord);
	outColor = vec4(resultColor, 1);
	outBloom = vec4(f3_BUF_EMISSION(fragTexCoord),1);
}

void main2() 
{
	//outColor = vec4(f3_BUF_COLOR(fragTexCoord), 1);//texture(maps[0], fragTexCoord);
	//vec3 normal = f3_BUF_NORMAL(fragTexCoord);
	//FhDynamicLightSourceInfo info = illuminationInfos.illuminationInfo[0];
	//vec3 position = f3_BUF_POSITION(fragTexCoord);
	
	//vec3 finalColor0 = getLightSourceInfluence(0);
	//vec3 finalColor1 = getLightSourceInfluence(1);
	
	vec3 albedo = f3_BUF_COLOR(fragTexCoord); // линейный RGB [0..1]
		albedo = pow(albedo, vec3(2.2)); // на случай, если текстура была sRGBв шейдере (грубее, но работает).

	vec3 finalColor = vec3(0.0);
	//finalColor += ambientIrradiance * albedo / PI;  // ambient как свет

	// Направление «небо → земля», можно получить из матрицы view или захардкодить
	const vec3 up = vec3(0.0, 1.0, 0.0);

	vec3 N = f3_BUF_NORMAL(fragTexCoord);

	vec3 E_ambient = mix(vec3(0.05, 0.06, 0.08),   // снизу: тёмный «подпольный» свет
                     vec3(0.25, 0.30, 0.40),   // сверху: цвет неба
                     N.y * 0.5 + 0.5);
	
	vec3 ambient = E_ambient * albedo / PI;

	finalColor += ambient;

	for(int l=0; l<16; l++)
	{		
		if( illuminationInfos.illuminationInfo[l].exists != 1) continue;	// discard non existing lightsources
		
		vec3 Lo = getLightSourceInfluence(l);
		float shadow = 1.0;

		#ifdef SHADOWS_ON
			if( l != 0 ) 
			{
				switch( illuminationInfos.illuminationInfo[l].type )
				{
					case 0 : shadow = shadowCube(l); break; // omni
					case 1 : shadow = shadowCalculation(l); break; // spot
					case 2 : shadow = shadowCalculation(l); break; // sun
				}
			}
			finalColor += Lo*shadow;
		#endif		
	}
	finalColor += f3_BUF_EMISSION(fragTexCoord);
	outColor = vec4(finalColor, 1.0);
	
	//outBloom  = vec4(max(finalColor - 1.0, vec3(0.0)), 1.0);

	outBloom = vec4(f3_BUF_EMISSION(fragTexCoord),1);
}