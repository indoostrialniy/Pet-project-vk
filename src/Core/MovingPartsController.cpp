#include "MovingPartsController.h"

#include "macros.h"
#include "Entity.h"
#include <thread>
#include <chrono>
#include <stop_token>
#include <atomic>

#include "fhStructs.h"
#include "fhEnumerations.h"

MovingPartsController::MovingPartsController(Graphics& core) : MicroWorld(core)
{
	
}

MovingPartsController::~MovingPartsController()
{
	// join() old threads first
	stopAllScenarios();
	
	// now clean the maps
	mainMovingPartsMassive.clear();
	threads_.clear();
	runnings_.clear();
	
}


// XXX совмещенные MMP_ROTATABLE_MOVABLE и MMP_ROTATABLE_MOVABLE_REVERSE надо обрабатывать немного по-другому, с учетом, возможно, разного времени на изменение


void MovingPartsController::addScenario(FhMovementScenarioInfo scenario, int scenarioIndex)
{
	if( !mainMovingPartsMassive.contains(scenarioIndex) )
	{
		// 1
		mainMovingPartsMassive[scenarioIndex] = scenario;
		
		// 2
		threads_[scenarioIndex] = std::thread{};
		
		// 3
		runnings_[scenarioIndex] = false;
	}
	else
	{
		std::cout << "Cannot add scenario ["<< scenarioIndex << "] to controller by [" << MicroWorldName << "!\n";
		DEBUG_INFO
	}
}


void MovingPartsController::launchScenario(int scenarioIndex) 
{	
	if( mainMovingPartsMassive.contains(scenarioIndex) && runnings_.contains(scenarioIndex) && threads_.contains(scenarioIndex) )
	{			
		// 1	
		if( runnings_[scenarioIndex] )		// if thread still running, do nothing and return
		{
			return;
		}
		// but if func has ended execution (runnings_ == false)
		
		// 2
		runnings_[scenarioIndex] = true;  	// start again with True
		if (threads_[scenarioIndex].joinable())  // because func ended, we can join() old thread
		{
			threads_[scenarioIndex].join();
		}
		// 3
		threads_[scenarioIndex] = std::thread(&MovingPartsController::executeScenario, this, scenarioIndex);	// start new thread
	}
	else
	{
		std::cout << "No scenario [" << scenarioIndex << "] found in controller by [" << MicroWorldName << "!\n";
		DEBUG_INFO
		return;
	}
  
}


void MovingPartsController::executeScenario(int scenarioIndex)
{
/*
	this func would`t start without checking if( mainMovingPartsMassive.contains(scenarioIndex) && runnings_.contains(scenarioIndex) && threads_.contains(scenarioIndex) )
	in MovingPartsController::launchScenario(int scenarioIndex) 

	so it`s safe to access mainMovingPartsMassive[scenarioIndex] and runnings_[scenarioIndex]!
*/
		
	using clock = std::chrono::steady_clock;
	auto last = clock::now();

	while( runnings_[scenarioIndex] )	// (running_) 
	{
		auto now = clock::now();
		std::chrono::duration<float> dt = now - last;
		last = now;
		
		#define TYPE2 mainMovingPartsMassive[scenarioIndex].transformationType

		float T;
		if(TYPE2 == MMP_MOVABLE || TYPE2 == MMP_MOVABLE_REVERSE)
		{
			T = mainMovingPartsMassive[scenarioIndex].fTimeForMoving;			//printVarComment("Time for rot: ", T)
		}
		if(TYPE2 == MMP_ROTATABLE || TYPE2 == MMP_ROTATABLE_REVERSE)
		{
			T = mainMovingPartsMassive[scenarioIndex].fTimeForRotation;			//printVarComment("Time for rot: ", T)
		}
				
		float x, y;
		float x1, y1;
				
				
		if( mainMovingPartsMassive[scenarioIndex].fTick < T )
		{

			mainMovingPartsMassive[scenarioIndex].fTick += dt.count(); // счетчик секунд float		
			
			// 1 определяем шкалу времени
			if(TYPE2 == MMP_ROTATABLE || TYPE2 == MMP_ROTATABLE_REVERSE)	
			{
				if( mainMovingPartsMassive[scenarioIndex].fTick > mainMovingPartsMassive[scenarioIndex].fTimeForRotation)	// 
				{
					mainMovingPartsMassive[scenarioIndex].fTick = mainMovingPartsMassive[scenarioIndex].fTimeForRotation;
				}
				x =  mainMovingPartsMassive[scenarioIndex].fTick / mainMovingPartsMassive[scenarioIndex].fTimeForRotation ;	// абсцисса: 0 ... 1
			}
			
			if(TYPE2 == MMP_MOVABLE	|| TYPE2 == MMP_MOVABLE_REVERSE)
			{
				if( mainMovingPartsMassive[scenarioIndex].fTick > mainMovingPartsMassive[scenarioIndex].fTimeForMoving)	// 
				{
					mainMovingPartsMassive[scenarioIndex].fTick = mainMovingPartsMassive[scenarioIndex].fTimeForMoving;
				}
				x =  mainMovingPartsMassive[scenarioIndex].fTick / mainMovingPartsMassive[scenarioIndex].fTimeForMoving ;	// абсцисса: 0 ... 1
			}
			
			

			// 2. Обработка поворотов/смещений...
			if(TYPE2 == MMP_ROTATABLE || TYPE2 == MMP_MOVABLE)	//  невозвратных
			{	y = mainMovingPartsMassive[scenarioIndex].status ? 1-x : x;		}	// линейный закон смешивания
			
			if(TYPE2 == MMP_ROTATABLE_REVERSE || TYPE2 == MMP_MOVABLE_REVERSE)	//  возвратных
			{	y = 1-(x-0.5f)*(x-0.5f)/0.25f;		}							// параболический закон изменения коэффициента смешивания
			
			//~ printVariable(T)
			//~ printVariable(y)
			
			//~ printVariable(dt)
			//~ printVariable(mainMovingPartsMassive[scenarioIndex].fTick)
			
			// 3 Применение изменений
			if(TYPE2 == MMP_ROTATABLE || TYPE2 == MMP_ROTATABLE_REVERSE)			// || TYPE == MMP_ROTATABLE_MOVABLE || TYPE == MMP_ROTATABLE_MOVABLE_REVERSE)
			{
				components[ mainMovingPartsMassive[scenarioIndex].name ]->setLocalRotation( glm::slerp( mainMovingPartsMassive[scenarioIndex].defaultQuat, mainMovingPartsMassive[scenarioIndex].targetQuat, y ) );	//применяем смешивание
			}
			
			if(TYPE2 == MMP_MOVABLE || TYPE2 == MMP_MOVABLE_REVERSE)				// || TYPE == MMP_ROTATABLE_MOVABLE || TYPE == MMP_ROTATABLE_MOVABLE_REVERSE)
			{
				components[ mainMovingPartsMassive[scenarioIndex].name ]->setLocalPosition( glm::mix( mainMovingPartsMassive[scenarioIndex].defaultPose, mainMovingPartsMassive[scenarioIndex].targetPose, y ) );
			}
						
			//~ printVariable(x)
			//~ printVariable(y)

			//float k = act->mainMovingPartsMassive[0].fTick / act->mainMovingPartsMassive[0].fTimeForRotation;	// like x		
			//printVariable(k)
			
			//act->components[act->mainMovingPartsMassive[act->MMP_ZERO].name]->rotation = glm::slerp( act->mainMovingPartsMassive[act->MMP_ZERO].defaultQuat, act->mainMovingPartsMassive[act->MMP_ZERO].targetQuat, k );

		}
		else
		{
			printMessage("End process")
			
			if(TYPE2 == MMP_ROTATABLE || TYPE2 == MMP_MOVABLE)
			{
				mainMovingPartsMassive[scenarioIndex].status = !mainMovingPartsMassive[scenarioIndex].status;
			}
			// по завершении обработки сбрасываем счетчик
			mainMovingPartsMassive[scenarioIndex].fTick = 0.0f;
			
			runnings_[scenarioIndex] = false;		//running_ = false;	// функция сообщает, что отработала
			//выход из потока
			break;
		}
	}

}


void MovingPartsController::stopAllScenarios() 
{
	for(auto scnr : mainMovingPartsMassive)
	{
		if( runnings_.contains(scnr.first) && threads_.contains(scnr.first) )
		{
			runnings_[scnr.first] = false;
			if (threads_[scnr.first].joinable()) 
			{
				threads_[scnr.first].join();
			}
		}
	}
}






