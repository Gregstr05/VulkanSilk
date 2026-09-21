
file(GLOB_RECURSE
        sources CONFIGURE_DEPENDS "*.cpp"
)

add_library(SilkMath STATIC ${sources})

add_library(Silk::Math ALIAS SilkMath)
