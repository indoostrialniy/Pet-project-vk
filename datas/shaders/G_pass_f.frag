#version 450
layout(early_fragment_tests) in;
layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in flat uint fragMaterialIndex;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in mat3 txrxMatrix;
layout(location = 7) in float sysTime;
layout(location = 8) in vec3 tangent;
layout(location = 9) in flat uint Type;
layout(location = 10) in vec3 POV;
layout(location = 11) in flat uint ID;
layout(location = 12) in flat struct VSOut
{
	uint v[48];
} fs_in;

#define MAX_SAMPLERS_PER_MATERIAL 16

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
	
	uint flowmapAddress;
	
	float colorScale;
	float normalScale;
	float roughnessScale;
	float metallicScale;
	
	float specularScale;
	float lightmapScale;
	float emissionScale;
	float displacementScale;
	
	float flowmapScale;
	
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
layout(location = 5) out uint outID;




float getDisplacement(FhMaterialInfo info)
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	int dispMapAddress = int(info.displacementAddress)-1;

	if(dispMapAddress >= 0)
	{
		return texture(material0_textures[ MAX_SAMPLERS_PER_MATERIAL * globalMaterialIndex  + dispMapAddress ], fragTexCoord ).r ;
	}
	else
	{
		return 0;
	}
}

vec2 getNewUV(float time, FhMaterialInfo info, uint offset, vec2 startUV, float dir)
{
	if(time > 10.0) return vec2(0,0); // watchdog timer

	int flowmapAddress = int(info.flowmapAddress)-1;
	if( flowmapAddress >= 0)
	{
		vec2 newUV = texture(material0_textures[offset  + flowmapAddress], startUV).xy; // flow vector for this pixel from map
			newUV = (newUV - vec2(0.5)) * 2; // convert to world space
			newUV = -vec2(newUV.x, -newUV.y) * 0.05*dir; // correction
		
		// time == [0.0 ... 0.99]
		for(float t=0; t<time; t+= 0.016) // 0.016 заменить на tick из push констант. Надо знать текущее время между кадрами!
		{
			vec2 flowVector = texture(material0_textures[offset  + flowmapAddress], startUV + newUV).xy; // flow vector for this pixel from map
				flowVector = (flowVector - vec2(0.5)) * 2; // convert to world space
				flowVector = -vec2(flowVector.x, -flowVector.y) * 0.05*dir; // correction
			
			newUV += flowVector*0.125*dir;
		}
		return startUV + newUV*(1);//*time*0.2;
	}
}

vec3 bcColor(float time, FhMaterialInfo info, uint offset, vec2 startUV, uint flowmapAddress, uint mapAddress)
{
	// 1. Вектор потока: центр (0.5, 0.5) -> ноль
	vec2 flowVector = texture(material0_textures[offset  + flowmapAddress], startUV).xy; // flow vector for this pixel from map
		flowVector = (flowVector - vec2(0.5)) * 2; // convert to world space
		flowVector = -vec2(flowVector.x, -flowVector.y) * 1; // correction

	float flowStrength = 0.15;
	// 2. Две фазы времени, сдвинутые на полпериода
	//float phase1 = abs(sin(time*0.25));
	//float phase2 = abs(sin(time*0.25 + 0.5));
	float phase1 = fract(time);
	float phase2 = fract(time)+ 0.5;

	// 3. Два сэмпла со "скачущим" смещением UV (sawtooth)
	vec2 uvA = startUV + flowVector * phase1 * flowStrength;
	vec2 uvB = startUV + flowVector * phase2 * flowStrength;
	vec4 texA = texture(material0_textures[offset+mapAddress], uvA );
	vec4 texB = texture(material0_textures[offset+mapAddress], uvB );

	// 4. Вес — треугольная волна: 1 в начале цикла, 0 в середине
	float wA = abs(2.0 * phase1 - 1.0);   // = 1 - |2*phase1 - 1| при другой записи
	float wB = 1.0 - wA;

	// 5. Смешивание
	vec4 result = texA * wA + texB * wB;   // или lerp(texA, texB, wB)
	return result.rgb;
}

vec3 bcColor2(float time, FhMaterialInfo info, uint offset, vec2 startUV, uint flowmapAddress, uint mapAddress)
{
	vec3 staticColor = texture(material0_textures[offset+mapAddress],  startUV ).rgb;
	
	vec3 movingColor1 = vec3(0);
	vec3 movingColor2 = vec3(0);

	float speed = 0.125;
	
	if( flowmapAddress >= 0)
	{
		vec2 newUV = texture(material0_textures[offset  + flowmapAddress], startUV).xy; // flow vector for this pixel from map
			newUV = (newUV - vec2(0.5)) * 2; // convert to world space
			newUV = -vec2(newUV.x, -newUV.y) * 0.05; // correction
			
		vec2 newUV2 = texture(material0_textures[offset  + flowmapAddress], startUV).xy; // flow vector for this pixel from map
			newUV2 = (newUV2 - vec2(0.5)) * 2; // convert to world space
			newUV2 = -vec2(newUV2.x, -newUV2.y) * 0.05; // correction
		
		// time == [0.0 ... 0.99]
		for(float t=0; t<time; t+= 0.016) // 0.016 (T 60FPS) заменить на tick из push констант. Надо знать текущее время между кадрами!
		{
			vec2 flowVector = texture(material0_textures[offset  + flowmapAddress], startUV + newUV).xy; // flow vector for this pixel from map
				flowVector = (flowVector - vec2(0.5)) * 2; // convert to world space
				flowVector = -vec2(flowVector.x, -flowVector.y) * 0.05; // correction
			newUV += flowVector*speed;
			
			vec2 flowVector2 = texture(material0_textures[offset  + flowmapAddress], startUV + newUV2).xy; 
				flowVector2 = (flowVector2 - vec2(0.5)) * 2; // convert to world space
				flowVector2 = -vec2(flowVector2.x, -flowVector2.y) * 0.05; // correction
			
			//if( abs(t-time)>30*0.016) newUV2 += flowVector2*speed;
			if( t > 0.5) newUV2 += flowVector2*speed;
			
			//if( t < 0.016*30)	{newUV2 += flowVector2*speed;}
			//else 				{newUV2 -= flowVector2*speed;}
		}
		
		float d = -0.09; // смещение на секунду
		for(float t=0; t<time+d; t+= 0.016) // 0.016 (T 60FPS) заменить на tick из push констант. Надо знать текущее время между кадрами!
		{
			
		}
		
		movingColor1 = texture(material0_textures[offset+mapAddress],  startUV + newUV ).rgb;//*time*0.2;
		movingColor2 = texture(material0_textures[offset+mapAddress],  startUV + newUV2 ).rgb;//*time*0.2;

	}
	
	float r = 0.5;
	float k = 1;
	float f1 = 2*r/k*pow(sin(3.14*time/k),2);
	float f2 = 2*r/k*pow(sin(3.14*(time+0.05)/k),2);

	
	return movingColor1*f1*1 + movingColor2*(f2)*1;

	//return staticColor*(1-time) + movingColor1*time; // OK
	
	//return texture(material0_textures[offset+mapAddress], startUV ).rgb;
	
	
}


float K(float x, float period)
{
	return (x - int(x/period)*period)/period;
}

float K2(float x, float r, float k, float period, float offset)
{
	return x< 1*offset ? 0 : 2*r/k*pow(sin(3.14*(x-offset)/(period*k)),2);
}

float K3(float x, float period)
{
	return 1 - pow( 2*(x-0.5), 2);
}

float K4(float x, float r, float k, float period, float offset)
{
	return x > offset ? 1-pow( (x-0.75)/0.25, 2) : 0;
}

vec3 bcColor3(float time, FhMaterialInfo info, uint offset, vec2 startUV, uint flowmapAddress, uint mapAddress)
{
	vec3 staticColor = texture(material0_textures[offset+mapAddress],  startUV ).rgb;
	vec3 movingColor1 = vec3(0);
	vec3 movingColor2 = vec3(0);
	vec3 movingColor3 = vec3(0);
	float speed = 0.125*2;
	vec2 flowVector;
	
	// time: 0 0.1 0.5 0.8 1 1.2 1.4 1.9 2 2.3 2.5 2.8 3 ...
	float period = 1.0; //flowspeed attenuation
	float k = K(time, period);//(time - int(time/period)*period)/period;
	// k:	0              0.33           0.66         1

	float TIME = 1;
	float step = 0.016;
	//for(float t=0; t<time*2; t+=0.016) // 0.016 (T 60FPS) заменить на tick из push констант. Надо знать текущее время между кадрами!
	int steps = int(k/step);
	vec2 offsets[100]; // массив для хранения смещений

	if( flowmapAddress >= 0)
	{
		vec2 newUV = texture(material0_textures[offset  + flowmapAddress], startUV).xy; // flow vector for this pixel from map
			newUV = (newUV - vec2(0.5)) * 2; // convert to world space
			newUV = -vec2(newUV.x, -newUV.y) * 0.05; // correction
		// time == [0.0 ... 0.99]
		
		vec2 newUV2 = newUV;
		
		offsets[0] = newUV;
		
		for(int s=1; s<100; s++)
		{
			// 1 take a new offset
			flowVector = texture(material0_textures[offset  + flowmapAddress], startUV + newUV).xy; // flow vector for this pixel from map
				flowVector = (flowVector - vec2(0.5)) * 2; // convert to world space
				flowVector = -vec2(flowVector.x, -flowVector.y) * 0.05; // correction
			
			// 2 push offset to accumulator
				newUV += flowVector*speed;
				offsets[s] = flowVector*speed;
		}
		
		vec2 off1 = vec2(0,0);
		vec2 off2 = vec2(0,0);
		vec2 off3 = newUV2;//vec2(0,0);
		
		// k = 0.5, int(k/step) = 31
		//int sdvig = 20;
		
		// фора первого
		//for(int s=0;s<15;s++) {off1 += offsets[s];}
		
		// фора второго
		//for(int s=0;s<30;s++) {off2 += offsets[s];}
		
		// фора третьего
		//for(int s=0;s<45;s++) {off3 += offsets[s];}
		
		
		for(int s=0; s<int(k/step); s++)
		{			
			//if(s<int(k/step))		
			//	off1 += offsets[s];
				
			//if(s>=15)		
			//	off2 += offsets[s-15]; // 15 - 45 = 31 шаг
			
			//if(s>=40 && s<int(k/step)+40) 
			//	off3 += offsets[s-40];
		}	
		
		
		for(int s=0; s<int(k/step)+0; s++) // в каждый момент времени координаты вычисляются по циклу
		{
			// z.B. k = 0.5, тогда при частоте 60 ФПС будет всего 30-й кадр
			if(s<int(k/step))			// первый пробежит целиком
				off1 += offsets[s];
			
			if(s>=20)					// второй подождет 20 шагов (1/3 от 60) и тоже пробежит, сколько успеет
				off2 += offsets[s-20];	

			if(s>=40)					// второй подождет 40 шагов (2/3 от 60) и тоже пробежит, сколько успеет
				off3 += offsets[s-40];	
				
		}
		
		/*
		for(float t=0; t<k; t+=step) // step on-map iteration
		{
			// 1 take a new offset
			flowVector = texture(material0_textures[offset  + flowmapAddress], startUV + newUV2).xy; // flow vector for this pixel from map
				flowVector = (flowVector - vec2(0.5)) * 2; // convert to world space
				flowVector = -vec2(flowVector.x, -flowVector.y) * 0.05; // correction
			// 2 push offset to accumulator
				newUV2 += flowVector*speed;
				//offsets[ int((t+step)/step) ] = flowVector*speed;
			
			off1 += flowVector*speed;
			if(t>0.5) off2 += flowVector*speed;
		}
		*/
		
		//movingColor1 = texture(material0_textures[offset+mapAddress],  startUV + newUV ).rgb;// PERFECT
		movingColor1 = texture(material0_textures[offset+mapAddress],  startUV + off1 ).rgb;
		movingColor2 = texture(material0_textures[offset+mapAddress],  startUV + off2 ).rgb;
		movingColor3 = texture(material0_textures[offset+mapAddress],  startUV + off3 ).rgb;
	}
	//float r = 0.5;
	//float k = 1;
	float f1 = K3( k, period );	//K2(time, 0.5, 1, period, 0);	//2*r/k*pow(sin(3.14*time/k),2);
	float f2 = K4(k, 0.5, 1, period, 0.5);
//K2(time, 0.5, 1, period, 0);	//time>0.5 ? 2*r/k*pow(sin(3.14*(time-0.5)/k),2) : 0;
	
	//float m=
	//float f3 = 

	//return vec3(flowVector, 0.5);//
	vec3 result = vec3(0);
		//result += staticColor*(1-f1);
		//result += movingColor1*abs(sin(3.14*k)); // OK!
		//result += movingColor2*( k > 0.667 ? abs(sin(3.14*(k-0.667))) : 0)  ;// (k>0.5 ? sin(2*3.14*(k-0.5)) : 0) ;//*(K2(time, 0.5, 1, period, 0)+0.0);//*(1-K2(time, 0.5, 1, period, 0));//;// +
		result += movingColor3*( k > 1.33 ? abs(sin(3.14*(k-1.33))) : 0);//abs(sin(3.14*(k-1.33)));//0.5*K2(time, 0.5, 1, period, 0.35);
		
	return result/2;
}


const float Phases = 3.0;

vec3 sampleFlow(vec2 uv, vec2 flowVec, float time, float speed, float strength, sampler2D diffuseTex) 
{
    float w[3], p[3], wSum = 0.0;

    for (int i = 0; i < 3; ++i) 
	{
        p[i]   = fract(time * speed + float(i) / Phases);
        w[i]   = min(p[i], 1.0 - p[i]);      // 0 в прыжке, max в середине
        wSum  += w[i];
    }

    vec3 result = vec3(0.0);
    for (int i = 0; i < 3; ++i) 
	{
        float wi = w[i] / wSum;              // сумма всех wi ровно 1
        vec2 uvi = uv + flowVec * (p[i] - 0.5) * strength;
        result  += texture(diffuseTex, uvi).rgb * wi;
    }
    return result;
}

void main() 
{
	uint globalMaterialIndex = fs_in.v[fragMaterialIndex] - 1;
	FhMaterialInfo info = materialInfos.materialInfo[globalMaterialIndex]; // current material description
	uint offset = MAX_SAMPLERS_PER_MATERIAL * globalMaterialIndex;

	uint TRIPLANAR = 0;
	TRIPLANAR = (Type >> 4)  & 0xFFu; // нужен 4-й бит
	//TRIPLANAR = (Type & 0x0010u);
	//TRIPLANAR = 0;

	bool TRANSPARENT = bool(Type & 0x00000001);

	float alpha = info.alphaDefault;

	//if(ID==12 || ID==13) discard;

	// NORMAL
	int normalMapAddress = int(info.normalAddress)-1;
	//float normalScale = 1;//info.normalScale;
	float period = 5.0; //flowspeed attenuation
	float k = (sysTime - int(sysTime/period)*period)/period;
	vec2 Tcoord = (ID==12 || ID==13) ? fragTexCoord + 1*vec2(0,-1)*k : fragTexCoord;

	

	int flowmapAddress = int(info.flowmapAddress)-1;
	if( flowmapAddress >= 0)
	{
		vec2 flowVector = texture(material0_textures[offset  + flowmapAddress], fragTexCoord).xy;
			flowVector = (flowVector - vec2(0.5)) * 2; // to world space
			flowVector = -vec2(flowVector.x, -flowVector.y);
		
		Tcoord = fragTexCoord + k * flowVector*0.5;
		
		////Tcoord = getNewUV(sysTime, info, offset, fragTexCoord, 1);//abs(sysTime/3.14);
		
		//int colorMapAddress = int(info.colorAddress)-1;
		//vec3 color1 = texture(material0_textures[ offset + colorMapAddress ], getNewUV(sysTime, info, offset, fragTexCoord, 1) ).rgb ;
		//vec3 color2 = texture(material0_textures[ offset + colorMapAddress ], getNewUV(sysTime, info, offset, fragTexCoord, 1) ).rgb ;

		//outColor = vec4(pow(color1, vec3(2.2)), alpha); // turn value to linear.

		//outColor = outColor*2;
		//return;
			
		//Tcoord = fragTexCoord * vec2(1);
		//outColor = vec4(flowVector/2+vec2(0.5),0,1);
		//return;
	}
	float nA = 1;//(ID==12 || ID==13) ? 0.0 : 1.0; // alpha for normal

	if(normalMapAddress >= 0)
	{
		// txrxMatrix — готовая transpose(inverse(mat3(model))) с CPU, не inverse на месте
		vec3 N = normalize(txrxMatrix * fragNormal);
		vec3 T = normalize(txrxMatrix * (tangent.xyz - fragNormal * dot(tangent.xyz, fragNormal)));
		vec3 B = cross(N, T);// * tangent.w;

		mat3 TBN = mat3(T, B, N);
	
		vec3 tempNorm = texture(material0_textures[offset  + normalMapAddress], fragTexCoord ).rgb;
		
		
		if( flowmapAddress >= 0)
		{
			//color = bcColor3(sysTime, info, offset, fragTexCoord, flowmapAddress, colorMapAddress);
			vec2 flowVec = texture(material0_textures[offset  + flowmapAddress], fragTexCoord).xy; 
				flowVec = (flowVec - vec2(0.5)) * 2; // convert to world space
				flowVec = -vec2(flowVec.x, -flowVec.y) * 0.5; // correction
				
				float speed = 0.15; 
				float strength = 2;
			tempNorm = sampleFlow(fragTexCoord, flowVec, sysTime, speed, strength, material0_textures[offset+normalMapAddress] ) ;
		}
		
		

			//tempNorm = bcColor(sysTime, info, offset, fragTexCoord, flowmapAddress, normalMapAddress);		
		outNormal = vec4(normalize(TBN * ( tempNorm * 2.0 - 1.0)), nA);
	}
	else	{	outNormal = vec4(normalize(txrxMatrix * fragNormal), nA);	}	


	// COLOR
	int colorMapAddress = int(info.colorAddress)-1;
	//float colorScale = info.colorScale;
	//#define UVSS fragTexCoord// * info.colorScale
	//#define UVSS fract(fragTexCoord * info.colorScale)
	//vec2 newUV = fragTexCoord;
		//int a = int(newUV.r) << int(info.colorScale);
		//newUV.g << info.colorScale;
	//#define UVSS fragTexCoord << int(info.colorScale)
	
	

	if(TRIPLANAR == 1)
	{
		if(colorMapAddress >= 0)
		{
			vec3 n = outNormal.xyz;//getNormalFrag(info);
			float nx = abs(n.x);
			float ny = abs(n.y);
			float nz = abs(n.z);
			float nxyz = nx + ny + nz;

			float vx = nx/nxyz;
			float vy = ny/nxyz;
			float vz = nz/nxyz;
			float scale = 1;
			vec2 Xplane = worldPosition.yz * scale * vec2(1,1);
			vec2 Yplane = worldPosition.xz * scale * vec2(1,1);
			vec2 Zplane = worldPosition.xy * scale * vec2(1,1);
			
			vec3 cx = texture(material0_textures[ offset  + colorMapAddress ], Xplane ).rgb;
			vec3 cy = texture(material0_textures[ offset  + colorMapAddress ], Yplane ).rgb;
			vec3 cz = texture(material0_textures[ offset  + colorMapAddress ], Zplane ).rgb;

			vec3 color = cx*vx + cy*vy + cz*vz;
			outColor = vec4(pow(color, vec3(2.2)), alpha); // turn value to linear.
		}
		else	{
			vec3 color = info.colorDefault.rgb;	
			outColor = vec4(pow(color, vec3(2.2)), alpha); // turn value to linear.
			}
	}
	else
	{
		if(colorMapAddress >= 0)
		{
			// sRGB color
			//vec2 Tcoord = (ID==12 || ID==13) ? fragTexCoord + vec2(1,1)*cos(6.28*sysTime) : fragTexCoord;
			
			vec3 color = texture(material0_textures[offset+colorMapAddress], fragTexCoord ).rgb;
			
			
			if( flowmapAddress >= 0)
			{
				//color = bcColor3(sysTime, info, offset, fragTexCoord, flowmapAddress, colorMapAddress);
				vec2 flowVec = texture(material0_textures[offset  + flowmapAddress], fragTexCoord).xy; 
					flowVec = (flowVec - vec2(0.5)) * 2; // convert to world space
					flowVec = -vec2(flowVec.x, -flowVec.y) * 0.5; // correction
					
					float speed = 0.15; 
					float strength = 1;
				color = sampleFlow(fragTexCoord, flowVec, sysTime, speed, strength, material0_textures[offset+colorMapAddress] ) ;
			}
			
			
			outColor = vec4(pow(color.rgb, vec3(2.2)), alpha); // turn value to linear.
			// or use more accurate formula
			// или сразу грузить текстуру в sRGB

			//outColor = outColor*2;
			//outColor = vec4(1,1,0, 1);
		}
		else	{
			outColor = vec4(info.colorDefault.rgb, alpha);	
			}
	}	
	
	if(ID==12 || ID==13) alpha = 0; 
	

	//if(  ) outColor = vec4(1);

	// POSITION
	if(ID==12) outPosition = vec4(worldPosition, 0);
	else {outPosition = vec4(worldPosition, 1);}

	// PBR
	float metallic = info.metallicDefault;
	int metalMapAddress = int(info.metallicAddress)-1;
	//float metallicScale = 1;//info.metallicScale;
	if(metalMapAddress >= 0)	{
		metallic = texture(material0_textures[ offset  + metalMapAddress ], fragTexCoord ).r ;		}

	float roughness = info.roughnessDefault;
	int roughMapAddress = int(info.roughnessAddress)-1;
	//float roughnessScale = 1;//info.roughnessScale;
	if(roughMapAddress >= 0)	{
		roughness = texture(material0_textures[ offset  + roughMapAddress ], fragTexCoord ).r ;	}

	vec3 emission = info.emissionDefault.rgb;
	int emissionMapAddress = int(info.emissionAddress)-1;
	//float emissionScale = 1;//info.emissionScale;
	if(emissionMapAddress >= 0)	{
		emission = texture(material0_textures[ offset  + emissionMapAddress ], fragTexCoord ).rgb ;	}

	outPBR = vec4(metallic, roughness, 0, 1);
	outEmission = vec4(emission, 1);
	outID = ID;

	//outColor = vec4(vec3(TRIPLANAR), 1);
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