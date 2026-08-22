ouif_add_app(ouif_fonts SOURCES main.cpp)
target_compile_definitions(ouif_fonts PRIVATE OUIF_EXAMPLE_FONT_PATH="${PROJECT_SOURCE_DIR}/examples/arial_narrow_7.ttf")
