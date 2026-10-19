#version 450

layout(location = 0) in vec4 FragPos;



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

layout(binding = 0, std140) uniform DynamicLightSourceInfo {
	FhDynamicLightSourceInfo illuminationInfo;
} illuminationInfos;

void main() 
{
	float far = illuminationInfos.illuminationInfo.radius;
	
	float far_plane = 40;
		far_plane = far;
		
	vec3 lightPos = illuminationInfos.illuminationInfo.position;	//
	//vec3 lightPos = vec3(6.97, 8.97, 2.96); // 

    // вычисление расстояния между фрагментом и источником 
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // преобразование к интервалу [0, 1] посредством деления на far_plane
    lightDistance = lightDistance / far_plane;
    
    // запись результата в результирующую глубину фрагмента
    gl_FragDepth = lightDistance;
	
	//depth = lightDistance;
}