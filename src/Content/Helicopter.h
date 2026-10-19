#pragma once

#include "Vehicle.h"

struct FhMicroWorldInitializationInfo;



class Helicopter : public Vehicle
{
public:
	Helicopter( const FhMicroWorldInitializationInfo& info );
	
	~Helicopter();
};
