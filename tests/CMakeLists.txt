add_executable(ouif_core_tests core_tests.cpp)
target_link_libraries(ouif_core_tests PRIVATE OUIF::ouif)
ouif_target_defaults(ouif_core_tests)
target_compile_definitions(ouif_core_tests PRIVATE OUIF_TEST_FONT_PATH="${PROJECT_SOURCE_DIR}/examples/arial_narrow_7.ttf")

add_test(NAME ouif_core_tests COMMAND ouif_core_tests)
