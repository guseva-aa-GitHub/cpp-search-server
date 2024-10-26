cmake_minimum_required(VERSION 3.8)

set(CMAKE_C_COMPILER "gcc" )
set(CMAKE_CXX_COMPILER "g++" )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS NO)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_GENERATOR "Unix Makefiles")
set(CMAKE_BINARY_DIR "build")

project(YandexPr-sprint4)

set(INC "src")

set(SOURCE
		main.cpp  
		src/request_queue.cpp			src/search_server.cpp        
		src/string_processing.cpp 		src/document.cpp
		src/read_input_functions.cpp)		

add_executable(${PROJECT_NAME} ${SOURCE} )
target_include_directories(${PROJECT_NAME} PRIVATE ${INC})			
