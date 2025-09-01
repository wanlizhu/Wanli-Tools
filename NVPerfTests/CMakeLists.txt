cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(NVPerfTests)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(GLHooks)
add_subdirectory(Window_GLVK)
add_subdirectory(UnitTests/MSAA)
