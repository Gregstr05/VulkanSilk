cmake_minimum_required(VERSION 3.20)

project(VulkanSilk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src/math)
add_subdirectory(src/vk)
add_subdirectory(src/app)
add_subdirectory(src/window)

include(FetchContent)
FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

add_executable(Sandbox
        src/main.cpp
)

target_include_directories(Sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(Sandbox PRIVATE Silk::App Silk::Math Silk::Window)

target_compile_features(SilkVK PRIVATE cxx_std_20)
