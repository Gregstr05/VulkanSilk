file(GLOB_RECURSE
        sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/tests/.*")

add_library(SilkVK STATIC
        ${sources}
)
add_library(Silk::VK ALIAS SilkVK)

target_include_directories(SilkVK PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(FetchContent)
FetchContent_Declare(volk
        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG 1.3.270
)
FetchContent_MakeAvailable(volk)

target_link_libraries(SilkVK PRIVATE volk)
