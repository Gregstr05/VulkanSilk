cmake_minimum_required(VERSION 3.20)

project(VulkanSilk)

add_subdirectory(src/math)
add_subdirectory(src/vk)

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

target_link_libraries(Sandbox PUBLIC Silk::Math Silk::VK glfw)

set_property(TARGET Sandbox PROPERTY CXX_STANDARD 20)
