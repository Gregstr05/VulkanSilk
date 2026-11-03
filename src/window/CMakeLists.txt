file(GLOB_RECURSE
        sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/tests/.*")

add_library(SilkWindow STATIC
        ${sources}
)
add_library(Silk::Window ALIAS SilkWindow)

target_include_directories(SilkWindow PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SilkWindow PUBLIC Silk::VK glfw)

set_property(TARGET SilkWindow PROPERTY CXX_STANDARD 20)