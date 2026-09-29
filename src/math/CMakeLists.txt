file(GLOB_RECURSE
        sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/tests/.*")

add_library(SilkMath STATIC ${sources})
add_library(Silk::Math ALIAS SilkMath)

target_include_directories(SilkMath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.15.3
)
FetchContent_MakeAvailable(Catch2)

add_executable(SilkMathTests tests/Vector.tests.cpp)
target_link_libraries(SilkMathTests PRIVATE Silk::Math Catch2::Catch2WithMain)

enable_testing()
add_test(NAME SilkMathTests COMMAND SilkMathTests)

set_property(TARGET SilkMath SilkMathTests PROPERTY CXX_STANDARD 20)
