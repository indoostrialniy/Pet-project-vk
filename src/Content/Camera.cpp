/*
	This header contains realization of in-game actors used in game.
*/

#include "Camera.h"

#include "macros.h"		// prints
#include "auxiliary.h" 	// Stopwatch definition
#include "globals.h"	// g_Key

#include "Entity.h"		// entityInitializationParameters


Camera::Camera( const FhMicroWorldInitializationInfo& info ) : MicroWorld(info.vulkanCore)
{
	MicroWorldName = info.name; 
	printVarComment("\n\tConstructor of: ", info.name)
	
	position = glm::vec3(-14.3, 4.5, 4.66); // start position
	//~ position = glm::vec3(11, -45, 2); // start position


	//~ FhEntityInitializationInfo SM_FPS_Arms;
		//~ SM_FPS_Arms.initSysname = "SM_FPS_Arms";
		//~ SM_FPS_Arms.initPos = glm::vec3(0.072, 0.0, -0.151);
		//~ SM_FPS_Arms.initRot = glm::quat(1.0, 0.0, 0.0, 0.0);
		//~ SM_FPS_Arms.initScale = glm::vec3(1.0, 1.0, 1.0);
		//~ SM_FPS_Arms.initMeshName = "datas/meshes/SM_FPS_Arms.model";	//dont forget initCollisionType
		//~ //SM_FPS_Arms.initCollisionType = emptyCollision;
		//~ SM_FPS_Arms.initMeshMaterialsVector = { "M_Arm_01", }; 
	//~ addEntity( SM_FPS_Arms );

	
	//rotation = glm::quat(0.456, 0.304, -0.455, -0.696);
	
	//~ printVec3(position)
	//~ printQuat(rotation)
}

void Camera::updateCamera()
{
	// 1. обязательно обновить матрицу векторов собственного пространства
	m3VectorsForwardLeftUp = mat3_cast(rotation) * glm::mat3(1.0f);
	
	float fMoveSpeedXY 		= 1.0f * fCameraMovingSpeedXY * Stopwatch::timeSinceLastFrame;// float(tick)/1000.0f; // сдвиг 1 см за шаг
	float fMoveSpeedZ 		= 1.0f * fCameraMovingSpeedZ * Stopwatch::timeSinceLastFrame;//float(tick)/1000.0f;
	float fRotationSpeed 	= 		 fCameraRotationSpeed * Stopwatch::timeSinceLastFrame;//float(tick)/1000.0f;
	
	// определим вектора направления камеры в плоскости X-Y
	glm::vec3 forwardVec 	= glm::normalize(glm::vec3(m3VectorsForwardLeftUp[0].x, m3VectorsForwardLeftUp[0].y, 0.0f)); // нормализовали вектор в плоскости 
	glm::vec3 upVec 		= glm::vec3(0, 0, 1);
	glm::vec3 leftVec 		= glm::cross(upVec, forwardVec); //плоскостный вектор влево получим путем векторного произведения вертикального на впередсмотрящий

	//printMat3("Camera vectors ", m3VectorsForwardLeftUp)
	//printVec3(position)

	// перемещаемся в горизонтальной плоскости
	if( g_Key[GLFW_KEY_W] ) 		{ position += forwardVec * fMoveSpeedXY; }
	if( g_Key[GLFW_KEY_S] ) 		{ position -= forwardVec * fMoveSpeedXY; } 
	if( g_Key[GLFW_KEY_A] ) 		{ position += leftVec * fMoveSpeedXY; }
	if( g_Key[GLFW_KEY_D] ) 		{ position -= leftVec * fMoveSpeedXY; }

	// перемещаемся вверх-вниз
	if( g_Key[GLFW_KEY_SPACE] ) 	{ position += upVec * fMoveSpeedZ * 1.5f; }
	if( g_Key[GLFW_KEY_C] ) 		{ position -= upVec * fMoveSpeedZ; }	

	glm::vec3 cameraAngles = glm::degrees(glm::eulerAngles(rotation));	// для ограничения углов атаки

	//по хорошему здесь надо делать ограничение на такой угол, который чуть больше того, на который поворачивается камера при текущем ФПС за 1 кадр, но пока оставим -80 + 80 град.

	//вращаемся 
	if( g_Key[GLFW_KEY_UP]  	&& cameraAngles.y > -80) 	{ rotation = glm::angleAxis(glm::radians(-fRotationSpeed), m3VectorsForwardLeftUp[1]) * rotation; }	//
	if( g_Key[GLFW_KEY_DOWN]  	&& cameraAngles.y < 80) 	{ rotation = glm::angleAxis(glm::radians( fRotationSpeed), m3VectorsForwardLeftUp[1]) * rotation; }	//
	if( g_Key[GLFW_KEY_LEFT] ) 								{ rotation = glm::angleAxis(glm::radians( fRotationSpeed), glm::vec3(0, 0, 1)) * rotation; }
	if( g_Key[GLFW_KEY_RIGHT]) 								{ rotation = glm::angleAxis(glm::radians(-fRotationSpeed), glm::vec3(0, 0, 1)) * rotation; }
}


Camera::~Camera() {	printVarComment("\n\tDestructor of Camera ", MicroWorldName)	}


