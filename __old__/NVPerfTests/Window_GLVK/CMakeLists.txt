cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

if (WIN32)
    include(FetchContent)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.3.9   # pin a version you want
    )
    FetchContent_MakeAvailable(glfw) 
else()
    find_package(glfw3)
endif()

find_package(OpenGL)
find_package(Vulkan)
find_package(Threads)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCE *.c *.cpp *.h)
add_library(Window_GLVK STATIC ${SOURCE})
target_include_directories(Window_GLVK PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(Window_GLVK PUBLIC 
    OpenGL::GL 
    Vulkan::Vulkan 
    Threads::Threads
    X11
    glfw
)

