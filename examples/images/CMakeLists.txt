ouif_add_app(ouif_images SOURCES main.cpp)
target_compile_definitions(ouif_images PRIVATE OUIF_EXAMPLE_CAT_PATH="${CMAKE_SOURCE_DIR}/examples/cat.jpg")
