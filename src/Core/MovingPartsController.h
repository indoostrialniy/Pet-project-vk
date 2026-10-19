#pragma once
#include "MicroWorld.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>
#include "globals.h"
#include <memory>
#include <thread>
#include <optional>


struct FhMovementScenarioInfo;

class Graphics;

// переименовать partStateController
class MovingPartsController : public MicroWorld
{
protected:
	MovingPartsController(Graphics& core);

public:
	~MovingPartsController();
		    
protected:
	void addScenario(FhMovementScenarioInfo scenario, int scenarioIndex);

    void launchScenario(int scenarioIndex);		// launched from derived classes: Weapon, Box etc
	
	
private:
	std::map<int, FhMovementScenarioInfo> mainMovingPartsMassive;	// словарь сценариев (расшифровка индекса описывается в дочерних классах отдельно)
	std::map<int, std::thread> threads_;					// словарь хэндлов на потоки
	std::map<int, std::atomic<bool>> runnings_;				// словарь флагов состояния потоков
	
	void executeScenario(int scenarioIndex);
	
	void stopAllScenarios();								// join() all existing threads
};
