add_executable(ouif_core_tests core_tests.cpp)
target_link_libraries(ouif_core_tests PRIVATE OUIF::ouif)
ouif_target_defaults(ouif_core_tests)

add_test(NAME ouif_core_tests COMMAND ouif_core_tests)
