#pragma once

#include "MicroWorld.h"

#include <string>

/*
	This header contains description of in-game actors used in game and derived from base MicroWorld class
*/

struct FhMicroWorldInitializationInfo;

class Camera : public MicroWorld
{
public:
	Camera( const FhMicroWorldInitializationInfo& info );
	~Camera();
	
	// локальные параметры камеры
	float 		fCameraMovingSpeedXY = 5.0f;		// скорость перемещения по плоскости - 5 м/сек
	float 		fCameraMovingSpeedZ = 4.0f;			// скорость перемещения по вертикали - 2 м/сек

	float 		fCameraRotationSpeed = 120.0f;		// скорость поворота вокруг оси - 120 град/сек

	float		fCameraViewNear = 0.01f;
	float 		fCameraViewDepth = 1000.0f;
	float 		fCameraFOV = 80.0f; 				// угол обзора камеры
	
	void 		updateCamera();
};




