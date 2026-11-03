file(GLOB_RECURSE
        sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/tests/.*")

add_library(SilkApp STATIC
        ${sources}
)
add_library(Silk::App ALIAS SilkApp)

target_include_directories(SilkApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SilkApp PUBLIC Silk::VK Silk::Window)

set_property(TARGET SilkApp PROPERTY CXX_STANDARD 20)