#pragma once

#ifndef OPTIONAL_ADV_H
#define OPTIONAL_ADV_H

#include <iostream>
#include <string>

// allows to trace only variable type
// данный макрос просто автоматически вносит имя переменной в качестве аргумента конструктора
#define FH_OPTIONAL(type, variableName) fh::optional<type> variableName(#variableName)
#define fh_optional(type, variableName) fh::optional<type> variableName(#variableName)
#define fh_value() value( __FILE__, std::to_string(__LINE__) )

namespace fh
{
		
	template <typename T>
	class optional
	{
	private:
		T data;
		bool initialized = false;
		std::string name = "";
			
	public:
		optional() {}
		optional(std::string n)
		{
			name = n;
		}
		~optional()
		{
			name.clear();
		}

		void operator=(const T& value)
		{
			initialized = true;
			data = value;
		}

		bool has_value()
		{
			return initialized;
		}
		
		T value()
		{		
			if(initialized)
			{
				return data;
			}
			else
			{
				//~ std::string log = "bad_optional_access to fh::optional<" + std::string(typeid(data).name()).substr(1) + "> " + name + "\n";
				std::string log = "bad_optional_access to fh::optional<" + std::string(typeid(data).name()) + "> " + name + "\n";

				throw std::runtime_error( log );
			}	
		}
		
		T value( std::string file, std::string line )
		{		
			if(initialized)
			{
				return data;
			}
			else
			{
				//~ std::string log = "bad_optional_access to fh::optional<" + std::string(typeid(data).name()).substr(1) + "> " + name + "\n";
				std::string log = "bad_optional_access to fh::optional<" + std::string(typeid(data).name()) + "> " + name + ". File: " + file + ", line: " + line +"\n";

				throw std::runtime_error( log );
			}	
		}
		
	};
};

#endif
