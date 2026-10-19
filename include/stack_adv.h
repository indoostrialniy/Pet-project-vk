#pragma once
#ifndef STACK_ADV_H
#define STACK_ADV_H

/*
	2025-01-20  indoostrialniy  <pawel.iv.2016@yandex.ru>
	
	Описание библиотеки stack_adv v0.3
	
	Назначение:
	
	Ко всем обозначениям добавлена приставка "stackADV" для минимизации вероятности конфликта имен в проекте, использующем множество библиотек.
	 
	Способ применения:
	1)	Подключить данный заголовочный файл:
		#include "stack_adv.h"
	
	
	Описание работы:
	
	Терминология: 
	
*/

//~ #define STACK_ADV_DEBUG

#include <iostream>
#include <memory>

template <class stackADVTemplate>
struct stackADVWrapper													// структура обертки хранимой информации
{
	~stackADVWrapper() { std::cout << "Destructor of wrapper" << std::endl; }

	
	stackADVWrapper(const stackADVTemplate& newElem) : data(newElem) 
	{
		//std::cout << "Constructor of wrapper" << std::endl;
	}	// приходит либо ссылка, либо rvalue, в любом случае, сразу инициализируем данные
	
	stackADVWrapper* prev = nullptr;									// указатель на предыдущий элемент
	stackADVWrapper* next = nullptr;									// указатель на следующий элемент
	
	stackADVTemplate data;												// единственный экземпляр данных
};


template <class stackADVTemplate>
class stackADV
{
public:
	~stackADV(){}

private:
	stackADVWrapper<stackADVTemplate>* elements = nullptr;
	
	stackADVWrapper<stackADVTemplate>* previosTop = nullptr;

public:	
	void push( const stackADVTemplate& newElement) // добавить элемент
	{	
		#ifdef STACK_ADV_DEBUG
			std::cout << "Pushed element " << &newElement << std::endl;
		#endif
		
		stackADVWrapper<stackADVTemplate>* rookie = new stackADVWrapper<stackADVTemplate> (std::move(newElement));	// создаем новый экземпляр обертки, заворачивая в него присланный объект
		
		//~ #ifdef STACK_ADV_DEBUG
			//~ std::cout << "Pushed element " << newElement.name << std::endl;
		//~ #endif
		
		if(elements == nullptr)											// если оберток еще вообще нету
		{
			elements = rookie;											// запоминаем первую обертку
		}
		else 															// а если элементы уже есть, то
		{		
			previosTop = elements;										// отдельно, во имя цели существования этой библиотеки, запомним старый топ
				
			elements->next = rookie;									// старому топу назначим следующим новый
			rookie->prev = elements;									// новому назначим предыдущим старый
			
			elements = rookie;											// и сделаем топовым новый элемент
		}
	}
	
		
	bool empty()
	{
		return elements == nullptr ? true : false;
	}
	
	
	void printData()
	{
		if(top()) 		std::cout << "\n\nTop: " << top()->name << std::endl;
		if(subtop()) 	std::cout << "Subtop: " << 	subtop()->name << std::endl;
		if(subsubtop()) std::cout << "Subsubtop: " << 	subsubtop()->name << std::endl;
		if(prevtop())	std::cout << "PrevTop: " << prevtop()->name << "\n" <<std::endl;
		
		//~ if(top()) 		std::cout << "\n\nTop: " << top() << std::endl;
		//~ if(subtop()) 	std::cout << "Subtop: " << 	subtop() << std::endl;
		//~ if(subsubtop()) std::cout << "Subsubtop: " << 	subsubtop() << std::endl;
		//~ if(prevtop())	std::cout << "PrevTop: " << prevtop() << "\n" <<std::endl;
	}
	
	void pop() 															// убрать из стека верхний элемент 
	{
		// проблема с удалением старого топа
		//~ std::cout << "\n__Pop\n";
		//~ if(previosTop != nullptr)
		//~ {
			//~ delete previosTop;
			//~ previosTop = nullptr;
		//~ }
		
		previosTop = elements;											// запомним старый топовый elements, который еще рано удалять
		
		if(elements != nullptr)											// если стек не пуст
		{	
			
			// далее идет просто подмена указателей
			if(elements->prev != nullptr)								// если есть предыдущий элемент
			{
				//stackADVWrapper<stackADVTemplate>* temp = elements->prev;
				//~ std::cout << "Flag 1" << std::endl;
				
				
				
				//~ if(previosTop != nullptr)									// если предыдущий топовый элемент не пуст, то надо его очистить и записать новый элемент
				//~ {
					// надо проверить, остается ли такой элемент еще где то
					//~ if(previosTop->data != nullptr)
					//~ {
						//~ delete previosTop->data;
						//~ previosTop->data = nullptr;
					//~ }
					//~ delete previosTop;										// удалим его оболочку
				//~ }
				
				
				
				//delete elements->data;
				//delete elements;
				
				elements = elements->prev;									// заменим топовый элемент на предыдущий
			}
			else 															// если предыдущего элемента нет
			{
				//delete elements->data;
				//delete elements;
				
				//~ if(previosTop != nullptr)									// если предыдущий топовый элемент не пуст
				//~ {
					//delete previosTop->data;								// удалим его старые данные
					//~ delete previosTop;										// удалим его оболочку
				//~ }
				//~ std::cout << "Flag 4" << std::endl;
				
				elements = nullptr; 										// делаем топовый элемент снова nullptr
			}
		}
	}
	
	void clear()
	{
		int watchdog = 0;
		
		if(elements != nullptr)
		{
			if(elements->prev == nullptr)	// если предыдущего нет, то просто удалим
			{
				pop();
			}
			else
			{
				while(elements->prev != nullptr)
				{
					pop();
					
					watchdog++;
					if(watchdog>1000) {std::cout << "Watchdog has been triggered! " << __FILE__ << ", " << __LINE__ << std::endl; break;}
				}
				pop();
			}
		}
		
		//delete previosTop->data;
		//delete previosTop;
		previosTop = nullptr;
	}
	
	
	void leaveOnlyTopThree()		// удалить все элементы, кроме top, subtop и subsubtop
	{
		std::cout << "\t\tXXX!! Empty stackADV::leaveOnlyTopThree!" << std::endl;
		// XXX проблема с удалением старого топового элемента в функции pop()
	}
	
	stackADVTemplate* top()	// получить верхний элемент
	{
		if(elements != nullptr)
		{
			return &elements->data;
		}
		else
		{
			#ifdef STACK_ADV_DEBUG
				std::cout << "No elements in stackADV! Return nullptr. stackADV::top() !"<<std::endl;
			#endif
			return nullptr;//stackADVTemplate();
		}
	}
	
	stackADVTemplate* subtop() // получить предповерхностный элемент
	{
		if(elements != nullptr && elements->prev != nullptr)
		{
			return &elements->prev->data;
		}
		else
		{
			#ifdef STACK_ADV_DEBUG
				std::cout << "No subtop element in stackADV! Return nullptr. stackADV::subtop() !"<<std::endl;
			#endif
			return nullptr;//stackADVTemplate();
		}		
	}
	
	stackADVTemplate* subsubtop() // получить предпредповерхностный элемент
	{
		if(elements != nullptr && elements->prev != nullptr && elements->prev->prev != nullptr)
		{
			return &elements->prev->prev->data;
		}
		else
		{
			#ifdef STACK_ADV_DEBUG
				std::cout << "No subsubtop element in stackADV! Return nullptr. stackADV::subsubtop() !"<<std::endl;
			#endif
			return nullptr;//stackADVTemplate();
		}		
	}

	stackADVTemplate* prevtop() // получить предыдущий топовый элемент
	{	
		if(previosTop != nullptr )
		{
			return &previosTop->data;
		}
		else
		{
			#ifdef STACK_ADV_DEBUG
				std::cout << "No prevtop element in stackADV! Return nullptr. stackADV::prevtop() !"<<std::endl;
			#endif
			return nullptr;//stackADVTemplate();
		}		
	}

	
};


#endif

