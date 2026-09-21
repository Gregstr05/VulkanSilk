cmake_minimum_required(VERSION 3.20)

project(VulkanSilk)

add_subdirectory(src/math)

add_executable(Sandbox
        src/main.cpp
)

target_link_libraries(Sandbox PUBLIC Silk::Math)
