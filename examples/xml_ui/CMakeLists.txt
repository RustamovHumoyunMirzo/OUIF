add_executable(ouif_xml_ui main.cpp)
target_link_libraries(ouif_xml_ui PRIVATE OUIF::ouif)
target_compile_definitions(ouif_xml_ui PRIVATE OUIF_XML_UI_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
