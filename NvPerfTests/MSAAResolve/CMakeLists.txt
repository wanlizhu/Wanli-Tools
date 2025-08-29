cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCE *.c *.cpp *.h)
add_executable(MSAAResolve ${SOURCE})
target_include_directories(MSAAResolve PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(MSAAResolve PUBLIC Window_GLVK)

