#pragma once
#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H


/*
	2026-10-07  indoostrialniy  <pawel.iv.2016@yandex.ru>
	
	Описание библиотеки ResourceManager v2.0.

	Назначение:
	Заголовочный файл "resourceManager.h" описывает шаблонный класс resManAsset для обработки экземпляров классов-ресурсов,
	который позволяет исключить возникновение экземпляров-двойников классов-ресурсов. 
	 
	Ко всем обозначениям добавлена приставка resMan для минимизации вероятности конфликта имен в проекте, использующем множество библиотек.
	 
	Способ применения:
	1)	Подключить данный заголовочный файл:
		#include "resourceManager.h"
	
	2)	При необходимости вывода в консоль сообщений об обработке ресурсов, перед подключением заголовочного файла объявить:
		#define RESOURCE_MANAGER_REPORTS
	
	3)	В нужном пользователю месте объявить экземпляр класса resManAsset, проинициализированный пользовательским классом YourOwnClass, обрабатывающим некий ресурс.
		Для удобства стоит указать в конструкторе этого экземпляра строковое описание типа хранимых уникальных ресурсов (проще будет потом разобраться в ресурсах):
			а)	resManAsset <YourOwnClass> 	yourAssetManager;
			б)	resManAsset <YourOwnClass> 	yourAssetManager("StringNameOfWhatYouStoreHere");
	
	Описание работы:
	Для реализации своего назначения библиотека содержит:
	1) Шаблонный класс resManAsset, содержащий:
		
		1.1) Шаблонный метод getAsset(std::string assetName), который создает shared_ptr, проверяет наличие экземпляра ресурса assetName и, 
			если он есть, "лочит" этот shared_ptr об weak_ptr из приватного словаря менеджера и возвращает созданный shared_ptr, 
			либо, если ресурса еще нет, полностью инициализирует созданный shared_ptr новым экземпляром пользовательского объекта, а в словарь помещает weak_ptr на него.
		
		1.2) Метод clear() сканирует словарь на предмет нулевых weak_ptr и при наличии оных через рекурсию производит удаление.
			Для подстраховки добавлен сторожевой таймер watchDog
	
		1.3) Метод printData(), позволяющий вывести в консоль список уникальных экземпляров ресурсов типа assetType. 
			
			
			
			// XXX #define RESOURCE_MANAGER_ASSET_DETAILS
			// XXX	позволит вызывать для ресурсов еще и их собственные методы printData(), позволяющие вывести в консоль подробности
	
*/

#include <iostream>
#include <unordered_map>
#include <memory>
#include <mutex>

//~ #define RESOURCE_MANAGER_REPORTS

class Graphics;

template <class R>
class resManAsset	// шаблонный класс, содержащий методы для обработки ресурсов
{
	// менеджер просто хранит словарь weak_ptr для общего контроля, а менеджмент ресурсов ведется в предоставленном из объекта shared_ptr
public:

	std::mutex mtx;

	Graphics* g_GraphicsCore;

	//~ resManAsset() {};
	
	void init(Graphics& core) { g_GraphicsCore = &core; }
	//~ resManAsset(Graphics* core) : g_GraphicsCore(core)	{}
	
	//~ resManAsset(std::string type)	
	//~ {
		//~ assetType = type;	
		//~ #ifdef RESOURCE_MANAGER_REPORTS
			//~ std::cout << "\n Create resource manager for: " << assetType << std::endl;
		//~ #endif	
	//~ }

	~resManAsset() 
	{
		#ifdef RESOURCE_MANAGER_REPORTS
			std::cout << "\n Delete resource manager for: " << assetType << std::endl;	
		#endif
		resources.clear();	// очистка словаря слабых указателей автоматически приведет к их выходу из области видимости и, следоватлеьно, удалению
	}
	
	
	
	std::shared_ptr<R> getAsset( const std::string& assetName)
	{
		std::shared_ptr<R> res;								// создадим новый shared_ptr для хранения ресурса
		
		std::lock_guard<std::mutex> lock(mtx);				// блокировка на случай многопоточности
		
		if( resources.count(assetName) != 0 )									// если найден дубликат
		{
			#ifdef RESOURCE_MANAGER_REPORTS
				std::cout << "\n\n\tResource [" << assetName << "] already has an instance, return shared_ptr reference." << std::endl;
			#endif
			
			res = resources[assetName].lock();									// отметим, что старого ресурса стало больше
		}
		else 																	// если ресурс ранее не был зарегистрирован
		{
			#ifdef RESOURCE_MANAGER_REPORTS
				std::cout << "\tCreate new instance of object [" << assetName << "]" << std::endl;
			#endif
				
			res = std::make_shared<R>(assetName, *g_GraphicsCore);			// создадим новый экземпляр, записав его адрес в shared_ptr
			
			resources[assetName] = std::weak_ptr<R>(res); 	// занесем в словарь слабую ссылку о новом ресурсе на основании созданного shared_ptr
			
			#ifdef RESOURCE_MANAGER_REPORTS
				std::cout << "\tWeak prt created [" << assetName << "]" << std::endl;
			#endif
		}
		
		//~ clear();																// на всякий случай запустим очистку пустых ключей
		/// XXX если очистку оставить, будет затык!!!!
		
		return res;																// и вернем указатель на запрашиваемый ресурс
	}
	
	
	std::shared_ptr<R> getAsset()
	{
		
		if (resources.empty()) 
		{
			//return std::nullopt; // Нет элементов
		}

		// Берём любой элемент — begin() даёт итератор на какой‑то существующий
		auto it = resources.begin();
		std::shared_ptr<R> sp = it->second.lock();

		if (!sp) 
		{
			// Слабый указатель истёк (объект Material уже удалён)
			//return std::nullopt;
		}
		return sp;
		//~ std::shared_ptr<R> res;								// создадим новый shared_ptr для хранения ресурса
		
		//~ //std::lock_guard<std::mutex> lock(mtx);				// блокировка на случай многопоточности
		
		//~ if( !resources.empty() )									// если найден дубликат
		//~ {
			//~ #ifdef RESOURCE_MANAGER_REPORTS
				//~ std::cout << "\n\n\tResource [" << assetName << "] already has at least one valid material instance, return shared_ptr reference." << std::endl;
			//~ #endif
			
			//~ res = resources.begin()->second->lock();
		//~ }
				
		//~ return res;																// и вернем указатель на запрашиваемый ресурс
	}
	
	
	//~ int watchdog = 0;
	// XXX Метод для удаления из словаря weak_ptr, если его счетчик равен 0 (т.е. если expired() == true). Но когда его вызывать-то? Мутить таймер или вызываеть его в конце функции getAsset()?
	void clear()
	{
		std::lock_guard<std::mutex> lock(mtx);				// блокировка на случай многопоточности

		for (auto it = resources.begin(); it != resources.end(); ) 
		{
			if (it->second.expired()) 
			{
				#ifdef RESOURCE_MANAGER_REPORTS
					std::cout << "Object [" << it->first << "] was deleted!" << std::endl;
				#endif
				it = resources.erase(it);  // Удаляем элемент и получаем следующий итератор
			} 
			else 
			{
				++it;
			}
		}
		
	}
	
	void printData()
	{
		std::cout << "\n Check counts!" << std::endl;
		for(auto res : resources)
			std::cout << res.first << " " << res.second.use_count() << std::endl;
	}

	
private:
	std::string assetType = "undefined asset type";
	
	std::unordered_map <std::string, std::weak_ptr<R> > 	resources;
	
	//доступ к карте с указателями на объекты осуществляется только через публичные функции addObject/deleteObject, во избежание утечек
};

#endif




/*
	Шаблон, которого должен придерживаться пользовательский класс ресурса.
	Общий вид:
	
	class Resource
	{
	public:
		// 3 обязательных переменных класса-ресурса и 2 метода публичны
		bool 			status = false;		// статус конфигурации ресурса
		std::string 	name = "default";	// name должен присутствовать всегда, для обеспечения шаблонирования классом resManAsset

		Resource(std::string resourceName)
		{
			// на основании возвращенного пользовательской функцией loadAndInitializeYourResource() булева значения устанавливается статус всего класса ресурса
			// и если он будет false, класс-менеджер просто удалит этот объект
			status = loadAndInitializeYourResource(); 
			//в функцию loadAndInitializeYourResource() вынесен пользовательский функционал
		}
		
		~Resource() {}
		
		void printData()	//обязательная функция, но можно оставить тело пустым
		{
			std::cout << "\tDetails of [" << name << "]:\n";
			// печать параметров данного ресурса
		}
		
	private:
		bool loadAndInitializeYourResource()
		{
			return true;
		}
		
		// пользовательские переменные и методы
	};
		
*/
