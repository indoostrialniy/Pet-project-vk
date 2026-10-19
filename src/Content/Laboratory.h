#pragma once
#include "Level.h"

#include <string>

/*
	This header contains description of level "Laboratory" derived from base Level class
*/
class Graphics;

class Laboratory : public Level
{
public:
	//Laboratory();
	
	Laboratory(std::string name, Graphics& core);
	   	
	~Laboratory();
};
