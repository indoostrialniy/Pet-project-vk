#pragma once

#include "Weapon.h"

struct FhMicroWorldInitializationInfo;

class SR3 : public Weapon
{
public:
	SR3( const FhMicroWorldInitializationInfo& info );
	~SR3();
};
