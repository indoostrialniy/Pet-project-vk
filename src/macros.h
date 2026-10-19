#pragma once
#ifndef MACROS_H
#define MACROS_H

#include <string>
#include <nameof.hpp>
#include <iostream>
#include <iomanip>

// МАКРОСЫ
#define printMessage(msg) std::cout << "\n" << msg << "\n";
#define printVariable(var) std::cout << "\n" << (#var) << " " << var << "\n";
#define printVarComment(msg, var) std::cout << "\n" <<msg << " " << var << "\n";

#define printVec2(vec) std::cout << "\n\tXY   " << (#vec) << ": " << (vec.x) << " " << (vec.y) << std::endl; 
#define printVec3(vec) std::cout << "\n\tXYZ  " << (#vec) << ": " << (vec.x) << " " << (vec.y) << " " << (vec.z) << std::endl; 
#define printVec4(vec) std::cout << "\n\tXYZW " << (#vec) << ": " << (vec.x) << " " << (vec.y) << " " << (vec.z) << " " << (vec.w) << std::endl;
#define printQuat(vec) std::cout << "\n\tXYZW " << (#vec) << ": " << (vec[0]) << " " << (vec[1]) << " " << (vec[2]) << " " << (vec[3]) << " " << std::endl;

#define printMat2x3(text, matrix) {std::cout << "\n" << (text) << " [" << (#matrix) << "] " << std::endl; std::cout << "\t\t" << matrix[0][0] << " " << matrix[1][0] << std::endl;std::cout << "\t\t" << matrix[0][1] << " " << matrix[1][1] << std::endl;std::cout << "\t\t" << matrix[0][2] << " " << matrix[1][2] << std::endl;}
#define printMat3(text, matrix) {std::cout << "\n" << (text) << " [" << (#matrix) << "] " << std::endl; std::cout << "\t\t" << matrix[0][0] << " " << matrix[1][0] << " " << matrix[2][0] << std::endl;std::cout << "\t\t" << matrix[0][1] << " " << matrix[1][1] << " " << matrix[2][1] << std::endl;std::cout << "\t\t" << matrix[0][2] << " " << matrix[1][2] << " " << matrix[2][2] << std::endl;}
//#define printMat4(text, matrix) {std::cout << "\n" << (text) << " [" << (#matrix) << "] " << std::endl; std::cout << "\t\t" << matrix[0][0] << " " << matrix[1][0] << " " << matrix[2][0] << " " << matrix[3][0] << std::endl;std::cout << "\t\t" << matrix[0][1] << " " << matrix[1][1] << " " << matrix[2][1] << " " << matrix[3][1] << std::endl;std::cout << "\t\t" << matrix[0][2] << " " << matrix[1][2] << " " << matrix[2][2] << " " << matrix[3][2] << std::endl;std::cout << "\t\t" << matrix[0][3] << " " << matrix[1][3] << " " << matrix[2][3] << " " << matrix[3][3] << std::endl;}

#define printMat4(text, matrix) \
{\
	std::cout << "\n" << (text) << " [" << (#matrix) << "] " << std::endl; \
	for (int r = 0; r < 4; ++r) {\
        for (int c = 0; c < 4; ++c) {\
            std::cout << std::setw(10) << matrix[c][r] << " "; \
        }\
        std::cout << "\n";\
    }\
}\

#define readFVec2( vec, line, offMicroWorld ) { vec.x = stof(getValue(line, offMicroWorld));	vec.y = stof(getValue(line, offMicroWorld+1));	}
#define readFVec3( vec, line, offMicroWorld ) { vec.x = stof(getValue(line, offMicroWorld));	vec.y = stof(getValue(line, offMicroWorld+1));	vec.z = stof(getValue(line, offMicroWorld+2));	}
#define readFVec4( vec, line, offMicroWorld ) { vec.x = stof(getValue(line, offMicroWorld));	vec.y = stof(getValue(line, offMicroWorld+1)); vec.z = stof(getValue(line, offMicroWorld+2));	vec.w = stof(getValue(line, offMicroWorld+3));	}


#define readQuatWXYZ( q, line, offMicroWorld) { q.w = stof(getValue(line, offMicroWorld));	q.x = stof(getValue(line, offMicroWorld+1)); q.y = stof(getValue(line, offMicroWorld+2));	q.z = stof(getValue(line, offMicroWorld+3));	}



// макросы для проверки валидности переменных

//#define CHECK_PTR(pointer) [&]()	{	if(pointer != nullptr)	{return true;}	else {	std::cout << "------------------------------------------------------------------------------------\nError by access ptr [" << NAMEOF(pointer) << "] by type [" << NAMEOF_TYPE(decltype(pointer)) << "]. It`s nullptr! " << __FILE__<< ", " << __LINE__<< "\n------------------------------------------------------------------------------------" << std::endl;	return false;	}	} ()
#define CHECK_VALUE(value) [&]()	{	if(value >= 0.0f)	{return true;}	else {	std::cout << "\n------------------------------------------------------------------------------------\nError by using variable [" << NAMEOF(value) << "] by type [" << NAMEOF_TYPE(decltype(value)) << "]. It`s lesser than zero: " << value<< ", " << __FILE__<< ", " << __LINE__<< "!\n------------------------------------------------------------------------------------\n" << std::endl;	return false;	}	} ()

#define CHECK_PTR(pointer) [&]() \
	{	if(pointer != nullptr)	{return true;}	\
		else {	std::cout << "------------------------------------------------------------------------------------\nError by access ptr [" << (pointer) << "]. It`s nullptr! File: " << __FILE__<< ", Line: " << __LINE__<< ", Function: " << __FUNCTION__<< "\n------------------------------------------------------------------------------------" << std::endl;	return false;	}	} ()
		//else {	std::cout << "------------------------------------------------------------------------------------\nError by access ptr [" << NAMEOF(pointer) << "] by type [" << NAMEOF_TYPE(decltype(pointer)) << "]. It`s nullptr! File: " << __FILE__<< ", Line: " << __LINE__<< ", Function: " << __FUNCTION__<< "\n------------------------------------------------------------------------------------" << std::endl;	return false;	}	} ()

#define CHECK_ASSET(pointer) [&]() \
	{	if(pointer->status)	{return true;}	\
		else {	std::cout << "------------------------------------------------------------------------------------\nError by access ptr [" << NAMEOF(pointer) << "] by type [" << NAMEOF_TYPE(decltype(pointer)) << "]. It`s nullptr! File: " << __FILE__<< ", Line: " << __LINE__<< ", Function: " << __FUNCTION__<< "\n------------------------------------------------------------------------------------" << std::endl;	return false;	}	} ()


//~ #define CHECK_HANDLE(handle) [&]() \
	//~ {	if(handle != VK_NULL_HANDLE)	{return true;}	\
		//~ else {	std::cout << "------------------------------------------------------------------------------------\nError by using vulkan handle [" << (handle) << "]. It`s VK_NULL_HANDLE! File: " << __FILE__<< ", Line: " << __LINE__<< ", Function: " << __FUNCTION__<< "\n------------------------------------------------------------------------------------" << std::endl;	return false;	}	} ()


#define CHECK_HANDLE(handle)                                      \
    do {                                                             \
        if ((handle) == VK_NULL_HANDLE) {                            \
            std::cerr << "==========================================" \
                      << "\n[Vulkan] VK_NULL_HANDLE!"                 \
                      << "\n  Handle: " #handle                       \
                      << "\n  File: "   << __FILE__                   \
                      << "\n  Line: "   << __LINE__                   \
                      << "\n  Func: "   << __FUNCTION__               \
                      << "\n==========================================" \
                      << std::endl;                                   \
            return VK_ERROR_INITIALIZATION_FAILED;                    \
        }                                                            \
    } while (0)


#define VK_IS_VALID_HANDLE(handle)                                   \
    ((handle) != VK_NULL_HANDLE)


// проверка хэндла "нормально валидного" - сообщит об ошибке в случае невалидности
#define HANDLE_VALID(handle)                                  \
    ((handle) != VK_NULL_HANDLE ? true :                             \
        (std::cerr << "==========================================\n" \
				   << "\t[Vulkan] VK_NULL_HANDLE: [" #handle             \
				   << "] in " << __FUNCTION__ << "()" 				\
                   << "\n\t\t\tat " << __FILE__ << ":" << __LINE__           \
                   << "\n==========================================" \
                      << std::endl, false))



#define HANDLE_NULL(handle)                                  \
    ((handle) == VK_NULL_HANDLE ? true :                             \
        (std::cerr << "==========================================\n" \
				   << "\t[Vulkan] Error: HANDLE VALID: [" #handle             \
				   << "] in " << __FUNCTION__ << "()" 				\
                   << "\n\t\t\tat " << __FILE__ << ":" << __LINE__           \
                   << "\n==========================================" \
                      << std::endl, false))


#define IF_DYNAMIC_CAST(castedType, castedPtr, pointer) 	castedType castedPtr = dynamic_cast<castedType>(pointer); if( [&]() { if(castedPtr != nullptr) {return true;} else { std::cout << "\n------------------------------------------------------------------------------------\nCannot dynamic cast [" << NAMEOF_TYPE(decltype(pointer)) <<" " <<  (#pointer) << "] to type [" << (#castedType) << "]! Nullptr! " <<  __FILE__ << ", " << __LINE__<< "!\n------------------------------------------------------------------------------------\n" << std::endl; return false; } } () )





//[ " <<(#funcName)  << " ]
#ifdef TRACE_CALLS
	#define FUNC(funcName)  funcName; std::cout << "\n\t[Trace report]: " << (#funcName)  << " was successfully called from " << __FUNCTION__ << "() [" << __FILE__ << ": " << __LINE__ << "] \n"; 
#else			
	#define FUNC(funcName)  funcName;	
#endif

#ifdef TRACE_CALLS
	#define DEBUG_INFO std::cout << " File: " << __FILE__<< ", L: " << __LINE__<< ", " << __FUNCTION__<< "(). \n ";
#else
	#define DEBUG_INFO std::cout << " File: " << __FILE__<< ", L: " << __LINE__<< ", " << __FUNCTION__<< "(). " << std::endl;
#endif


#define PUT_DEBUG_INFO \
    std::cout << "[DEBUG] [" << __FUNCTION__ << "]" \
              << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; system("pause");



#endif
