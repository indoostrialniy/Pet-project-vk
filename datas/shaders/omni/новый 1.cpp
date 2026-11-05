struct FhDynamicLightSourceInfo 										// 16 таких структур передаются в буфер к шейдеру - MAX_DYNAMIC_LIGHT_SOURCES
{
public:
  0|				glm::vec3 position 	= glm::vec3(0.0, 0.0, 0.0);
 16|	alignas(16) glm::vec3 direction = glm::vec3(1.0, 0.0, 0.0);
 32|	alignas(16) glm::vec3 color 	= glm::vec3(1.0, 1.0, 1.0);
	
 44|	uint32_t type		= FH_DYNAMIC_LIGHTSOURCE_OMNIDIRECTIONAL;
 48|	float	sector		= 30.0f;
 52|	float	radius		= 10.0f;
 56|	float	power		= 10.0f;
 60|	float	blendWidth	= 10.0f;							// зона плавной границы для фонарика
 64|	uint32_t exists		= 0;								// флаг о существовании источника. Надо ли обрабатывать его в шейдере?
				
 80|	alignas(16) glm::mat4 PV[6]	
	{	glm::mat4(1.0f),
144|	glm::mat4(1.0f),
208|		glm::mat4(1.0f),
		glm::mat4(1.0f),
		glm::mat4(1.0f),
		glm::mat4(1.0f)};
	
	//float reserveOffset[12] {0,0,0,0,0,0,0,0,0,0,0,0};		// массив заглушка, чтобы размер структуры стал кратен 64 байтам
};