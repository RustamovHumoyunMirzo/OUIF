ouif_add_app(ouif_blur_effect SOURCES main.cpp)
target_compile_definitions(ouif_blur_effect PRIVATE OUIF_EXAMPLE_CAT_PATH="${CMAKE_SOURCE_DIR}/examples/cat.jpg")
