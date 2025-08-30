cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(NvPerfTests)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Window_GLVK)
add_subdirectory(UnitTests/MSAA)
add_subdirectory(UnitTests/Bandwidth)
