#pragma once
#ifndef BI_CYCLE_H
#define BI_CYCLE_H

#include <iostream>
#include <vector>
#include <functional>

#define BI_CYCLE_ENABLED

//~ #warning "Ensure that you have added Execute()-call in main loop!"

namespace biCycle
{	
	/*
		2025-04-15  indoostrialniy  <pawel.iv.2016@yandex.ru>
		
		biCycle library v1.5 description
		Называется велосипедом не просто так ¯\_(ツ)_/¯
	*/
	template <typename DataPack>							// DataPack - type of structure with data, which will be pushed as argument into "function" via Execute()
	struct Wrapper											// mixed func Wrapper
	{	
		std::function<bool(DataPack&)> 	function;			// returned bool signalize about completing
		
		DataPack						data;
		
		std::function<void()>			start_callback;		// empty callback
		std::function<void()>			end_callback;
		
		std::function<bool(DataPack&)>	startFunc;			// data callback
		std::function<bool(DataPack&)>	endFunc;
	};

	
	
	template <typename DataPack>	
	class Sequencer
	{
		std::vector<Wrapper<DataPack>> 	sequences;					// func-shell-vector
	public:
		~Sequencer()				{	sequences.clear(); 	}		// auto clearing when exit
		
		void CallSequence( const Wrapper<DataPack>& newSequence)	// функция CallSequence() лишь принимает ссылочную оберточную структуру Wrapper функции и сохраняет ее в приватном векторе sequences
		{
			sequences.push_back( std::move(newSequence) );	
			
			// так как вносим в конец вектора новую последовательность, которую подразумевается сразу же начать исполнять, то для вызова начального колбека
			// получим последний элемент вектора функцией .back(), которая и будет нашей вносимой
			
			if(sequences.back().startFunc)
			{	sequences.back().startFunc(sequences.back().data);	}		// вызов стартовой функции!!!
			
			if(sequences.back().start_callback)
			{ sequences.back().start_callback();	}
			
		}

		void Execute() 										// функция Execute() должна вызываться из основной части программы. Например, можно вызывать ее в каждом цикле главной петли
		{	
			if( !sequences.empty() )
			{
				auto it = sequences.begin();
				
				while (it != sequences.end()) 
				{
					if(it->function)
					{
						bool result = it->function(it->data);
					
						if (result) 
						{
							if (it->endFunc) 		it->endFunc(it->data);		
							if (it->end_callback) 	it->end_callback();
							
							it = sequences.erase(it); // erase returns next iterator
						} 
						else 
						{
							++it;
						}
					}
					else // если функция вдруг оказалась пустой
					{
						//std::cout << "\t\tEmpty main func!\n";
						
						if (it->endFunc) 		it->endFunc(it->data);
						if (it->end_callback) 	it->end_callback();
						
						it = sequences.erase(it); // erase returns next iterator
					}
					
				}
			}
		}

		
	};
	
}



#endif
