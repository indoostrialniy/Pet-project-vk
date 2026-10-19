cmake_minimum_required(VERSION 3.20)
project(EngineVK VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Отключаем разделение по конфигурациям
set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)

# 2. Устанавливаем единый выходной каталог
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Для мультиконфигурационных генераторов (Visual Studio, Xcode)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Определим источники и заголовки один раз
#set(HEADERS    src/macros.h	src/auxiliary.h src/globals.h src/MicroWorld.h src/vkTexture.h src/GameplayClasses.h	src/fhEngine.h src/Entity.h src/Level.h src/Laboratory.h src/Window_API_GLFW.h )

file(GLOB_RECURSE HEADERS
    "src/*h"
	#"Window_Interface/*.h"
    "Core/*.h"
	"Content/*.h"
	"Resources/*.h"
	"Render/*.h"
)


#set(SOURCES    src/auxiliary.cpp src/vkTexture.cpp src/globals.cpp src/MicroWorld.cpp src/GameplayClasses.cpp src/fhEngine.cpp src/Entity.cpp src/Level.cpp src/Laboratory.cpp src/Window_API_GLFW.cpp )# specify source code files

file(GLOB_RECURSE SOURCES
    "src/*cpp"
	#"Window_Interface/*.cpp"
    "Core/*.cpp"
	"Content/*.cpp"
	"Resources/*.cpp"
	"Render/*.cpp"
)

include_directories(include	)													# specify include-folder at Windows
include_directories(include/sol	)													# specify include-folder at Windows

include_directories(src	)
#include_directories(src/Window_Interface)
include_directories(src/Core)
include_directories(src/Content)
include_directories(src/Resources)
include_directories(src/Render)



if(WIN32)
    message(STATUS "This is Windows!")
	
		
	# Проверяем установку Vulkan SDK
	find_package(Vulkan REQUIRED)
	if(NOT VULKAN_FOUND)
		  message(FATAL_ERROR "Vulkan SDK not found!")
	endif()
	
	# Выводим информацию о найденном Vulkan SDK
	message(STATUS "Vulkan SDK found: ${Vulkan_VERSION}")
	
	# Устанавливаем директории для include-файлов
	include_directories(${Vulkan_INCLUDE_DIR})
	
	# specify libraries folder
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		message(STATUS "Compiler: GCC (version ${CMAKE_CXX_COMPILER_VERSION})")
		link_directories(libs/mingw/)				
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		message(STATUS "Compiler: MSVC (version ${CMAKE_CXX_COMPILER_VERSION})")
		link_directories(libs/msvc/)
	endif()
	
	add_library(lua STATIC IMPORTED)
	set_target_properties(lua PROPERTIES
		IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libs/mingw/liblua54.a"
	)

	add_executable(fhEngine ${SOURCES} ${HEADERS})									# create executable by sources
	#target_link_libraries(fhEngine PRIVATE ${Vulkan_LIBRARY})
		
	target_link_libraries(fhEngine ${Vulkan_LIBRARY} vulkan-1 glfw3 gdi32 sfml-graphics sfml-window sfml-system sfml-audio  lua)	# linking libraries
	
	#target_link_libraries(Engine opengl32 winmm gdi32)							# auxiliary avoid errors
	
	# hide console by minGW and MSVC
	#if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	#	set_target_properties(Engine PROPERTIES WIN32_EXECUTABLE TRUE)
	#elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	#	target_compile_definitions(Engine PRIVATE _WIN32_WINNT=0x0601)
    #    set_target_properties(Engine PROPERTIES  LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:WINDOWS" )
	#endif()
	
	
	
	
elseif(UNIX)
    message(STATUS "This is Linux!")

    #include_directories(include	)
	
	# Ищем SFML
	find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

	#ищем glew
	#find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)


	# Проверяем, что SFML найден
	if(NOT SFML_FOUND)
		message(FATAL_ERROR "SFML 2.5 not found! Install it!")
	endif()

	# Выводим информацию о найденном SFML
	message(STATUS "SFML found: ${SFML_VERSION}")
	#message(STATUS "SFML libraries: ${SFML_LIBRARIES}")

	# Создаем цель ПЕРВОЙ!
    add_executable(fhEngine ${SOURCES} ${HEADERS})
	
	target_link_directories(fhEngine PRIVATE /usr/lib/)  # Путь к SFML
    # Указываем библиотеки, которые нужно линковать с проектом

    target_link_libraries(fhEngine
		GLEW #glew
		#GL
		glfw
        sfml-window
        sfml-graphics
        sfml-system
    )

    # Дополнительные библиотеки для Linux
    target_link_libraries(fhEngine
        GL       # OpenGL
        pthread  # Потоки
        X11      # X11 для окон
    )

endif()



# Поиск SFML
#find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)



# Указываем пути к библиотекам





#set(SFML_DIR "/usr/lib/x86_64-linux-gnu/cmake/SFML")  # Стандартный путь в Linux
#find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)



# скрыть конколь
#set_target_properties(Engine PROPERTIES
#    WIN32_EXECUTABLE TRUE  # Для Windows (отключает консоль)
#    MACOSX_BUNDLE TRUE     # Для macOS (если нужно bundle-приложение)
#)

