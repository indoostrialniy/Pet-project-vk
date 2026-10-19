#pragma once

#include "MicroWorld.h"

struct FhMicroWorldInitializationInfo;

/*
	This header contains description of in-game actors used in game and derived from base MicroWorld class
*/


class Scena : public MicroWorld
{
public:
	Scena( const FhMicroWorldInitializationInfo& info );
	~Scena();
};
