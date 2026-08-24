cmake_minimum_required(VERSION 3.24)

project(
    OUIF
    VERSION 0.1.0
    DESCRIPTION "An Open User Interface Framework for modern C++"
    HOMEPAGE_URL "https://github.com/RustamovHumoyunMirzo/OUIF"
    LANGUAGES C CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FetchContent)

option(OUIF_BUILD_SHARED "Build OUIF as a shared library" ON)
option(OUIF_BUILD_EXAMPLES "Build OUIF examples" ON)
option(OUIF_BUILD_TESTS "Build OUIF tests" ON)
option(OUIF_FETCH_BGFX "Fetch bgfx with CMake FetchContent" ON)
option(OUIF_FETCH_GLFW "Fetch GLFW for OUIF's private window backend" ON)
option(OUIF_USE_KATANA "Build Katana CSS parser for OUIF stylesheet support" ON)
option(OUIF_USE_PUGIXML "Build pugixml for OUIF XML UI support" ON)
option(OUIF_INSTALL "Generate install targets" ON)
set(OUIF_DEPS_DIR "${PROJECT_SOURCE_DIR}/external" CACHE PATH "Directory for OUIF-managed source dependencies")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

if(OUIF_FETCH_BGFX)
    set(BGFX_BUILD_TOOLS ON CACHE BOOL "Build bgfx tools" FORCE)
    set(BGFX_BUILD_TOOLS_BIN2C OFF CACHE BOOL "Build bgfx bin2c tool" FORCE)
    set(BGFX_BUILD_TOOLS_SHADER ON CACHE BOOL "Build bgfx shader compiler" FORCE)
    set(BGFX_BUILD_TOOLS_GEOMETRY OFF CACHE BOOL "Build bgfx geometry tools" FORCE)
    set(BGFX_BUILD_TOOLS_TEXTURE OFF CACHE BOOL "Build bgfx texture tools" FORCE)
    set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "Build bgfx examples" FORCE)
    set(BGFX_BUILD_TESTS OFF CACHE BOOL "Build bgfx tests" FORCE)
    set(BGFX_LIBRARY_TYPE STATIC CACHE STRING "Build bgfx as static libraries" FORCE)

    set(OUIF_BGFX_CMAKE_DIR "${OUIF_DEPS_DIR}/bgfx.cmake" CACHE PATH "bgfx.cmake source directory")
    if(EXISTS "${OUIF_BGFX_CMAKE_DIR}/CMakeLists.txt")
        add_subdirectory("${OUIF_BGFX_CMAKE_DIR}" "${PROJECT_BINARY_DIR}/_deps/bgfx-build")
    else()
        message(FATAL_ERROR
            "bgfx sources were not found at ${OUIF_BGFX_CMAKE_DIR}. "
            "Run scripts/fetch-deps.ps1 or scripts/fetch-deps.sh, then configure again."
        )
    endif()
endif()

if(OUIF_FETCH_GLFW)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW documentation" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "Generate GLFW install targets" FORCE)

    set(OUIF_GLFW_DIR "${OUIF_DEPS_DIR}/glfw" CACHE PATH "GLFW source directory")
    if(EXISTS "${OUIF_GLFW_DIR}/CMakeLists.txt")
        add_subdirectory("${OUIF_GLFW_DIR}" "${PROJECT_BINARY_DIR}/_deps/glfw-build")
    else()
        message(FATAL_ERROR
            "GLFW sources were not found at ${OUIF_GLFW_DIR}. "
            "Run scripts/fetch-deps.ps1 or scripts/fetch-deps.sh, then configure again."
        )
    endif()
endif()

if(OUIF_USE_KATANA)
    set(OUIF_KATANA_DIR "${OUIF_DEPS_DIR}/katana" CACHE PATH "Katana parser source directory")
    if(EXISTS "${OUIF_KATANA_DIR}/src/katana.h")
        add_library(
            ouif_katana STATIC
            "${OUIF_KATANA_DIR}/src/foundation.c"
            "${OUIF_KATANA_DIR}/src/katana.lex.c"
            "${OUIF_KATANA_DIR}/src/katana.tab.c"
            "${OUIF_KATANA_DIR}/src/parser.c"
            "${OUIF_KATANA_DIR}/src/selector.c"
            "${OUIF_KATANA_DIR}/src/tokenizer.c"
        )
        target_include_directories(ouif_katana PUBLIC "${OUIF_KATANA_DIR}/src")
        if(MSVC)
            target_include_directories(ouif_katana PRIVATE "${OUIF_KATANA_DIR}/visualc/include")
            target_compile_definitions(ouif_katana PRIVATE YY_NO_UNISTD_H)
        endif()
        set_target_properties(ouif_katana PROPERTIES POSITION_INDEPENDENT_CODE ON)
    else()
        message(FATAL_ERROR
            "Katana CSS parser sources were not found at ${OUIF_KATANA_DIR}. "
            "Run scripts/fetch-deps.ps1 or scripts/fetch-deps.sh, then configure again."
        )
    endif()
endif()

if(OUIF_USE_PUGIXML)
    set(OUIF_PUGIXML_DIR "${OUIF_DEPS_DIR}/pugixml" CACHE PATH "pugixml source directory")
    if(EXISTS "${OUIF_PUGIXML_DIR}/CMakeLists.txt")
        set(PUGIXML_BUILD_TESTS OFF CACHE BOOL "Build pugixml tests" FORCE)
        add_subdirectory("${OUIF_PUGIXML_DIR}" "${PROJECT_BINARY_DIR}/_deps/pugixml-build")
    else()
        message(FATAL_ERROR
            "pugixml sources were not found at ${OUIF_PUGIXML_DIR}. "
            "Run scripts/fetch-deps.ps1 or scripts/fetch-deps.sh, then configure again."
        )
    endif()
endif()

include(cmake/OUIFFunctions.cmake)

set(OUIF_LIBRARY_TYPE STATIC)
if(OUIF_BUILD_SHARED)
    set(OUIF_LIBRARY_TYPE SHARED)
endif()

add_library(ouif ${OUIF_LIBRARY_TYPE})
add_library(OUIF::ouif ALIAS ouif)

target_sources(
    ouif
    PRIVATE
        src/Animation.cpp
        src/Application.cpp
        src/Effect.cpp
        src/Layout.cpp
        src/Renderer.cpp
        src/Resources.cpp
        src/Widget.cpp
        src/Widgets.cpp
        src/Xml.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/OUIF/OUIF.h
            include/OUIF/Animation.h
            include/OUIF/Application.h
            include/OUIF/Color.h
            include/OUIF/Effect.h
            include/OUIF/Event.h
            include/OUIF/Export.h
            include/OUIF/Geometry.h
            include/OUIF/Layout.h
            include/OUIF/Renderer.h
            include/OUIF/Resources.h
            include/OUIF/Style.h
            include/OUIF/Widget.h
            include/OUIF/Window.h
            include/OUIF/Widgets.h
            include/OUIF/Xml.h
)

target_compile_features(ouif PUBLIC cxx_std_20)

function(ouif_add_file target id file_path)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "ouif_add_file target '${target}' does not exist")
    endif()
    get_filename_component(OUIF_RESOURCE_PATH "${file_path}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    if(NOT EXISTS "${OUIF_RESOURCE_PATH}")
        message(FATAL_ERROR "ouif_add_file resource '${OUIF_RESOURCE_PATH}' does not exist")
    endif()
    file(READ "${OUIF_RESOURCE_PATH}" OUIF_RESOURCE_HEX HEX)
    string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1, " OUIF_RESOURCE_BYTES "${OUIF_RESOURCE_HEX}")
    set(OUIF_RESOURCE_OUT "${CMAKE_CURRENT_BINARY_DIR}/ouif_resource_${target}_${id}.cpp")
    file(GENERATE OUTPUT "${OUIF_RESOURCE_OUT}" CONTENT
"#include <OUIF/Resources.h>
#include <cstdint>

namespace {
const std::uint8_t resource_${id}[] = { ${OUIF_RESOURCE_BYTES} };
struct ResourceRegister_${id} {
    ResourceRegister_${id}() {
        ouif::Resources::register_bytes(${id}, resource_${id}, sizeof(resource_${id}));
    }
};
ResourceRegister_${id} register_resource_${id};
}
")
    target_sources(${target} PRIVATE "${OUIF_RESOURCE_OUT}")
endfunction()

if(OUIF_BUILD_SHARED)
    target_compile_definitions(ouif PUBLIC OUIF_SHARED)
endif()
target_include_directories(
    ouif
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_definitions(ouif PRIVATE OUIF_BUILDING_LIBRARY)

if(TARGET bgfx)
    target_link_libraries(ouif PRIVATE bgfx bx bimg)
    target_include_directories(ouif PRIVATE "${bgfx_SOURCE_DIR}/bgfx/3rdparty/stb")
    target_compile_definitions(ouif PRIVATE OUIF_WITH_BGFX=1)
    target_compile_definitions(ouif PRIVATE OUIF_SHADER_DIR="$<TARGET_FILE_DIR:ouif>/ouif-shaders")
else()
    target_compile_definitions(ouif PRIVATE OUIF_WITH_BGFX=0)
endif()

if(TARGET glfw)
    target_link_libraries(ouif PRIVATE glfw)
    target_compile_definitions(ouif PRIVATE OUIF_WITH_GLFW=1)
else()
    message(FATAL_ERROR "OUIF requires GLFW. Enable OUIF_FETCH_GLFW or provide a glfw target before adding OUIF.")
endif()

if(TARGET ouif_katana)
    target_link_libraries(ouif PRIVATE ouif_katana)
    target_compile_definitions(ouif PRIVATE OUIF_WITH_KATANA=1)
else()
    target_compile_definitions(ouif PRIVATE OUIF_WITH_KATANA=0)
endif()

if(TARGET pugixml::pugixml)
    target_link_libraries(ouif PRIVATE pugixml::pugixml)
    target_compile_definitions(ouif PRIVATE OUIF_WITH_PUGIXML=1)
elseif(TARGET pugixml)
    target_link_libraries(ouif PRIVATE pugixml)
    target_compile_definitions(ouif PRIVATE OUIF_WITH_PUGIXML=1)
else()
    target_compile_definitions(ouif PRIVATE OUIF_WITH_PUGIXML=0)
endif()

if(TARGET bgfx AND TARGET shaderc)
    set(OUIF_SHADER_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ouif-shaders")

    function(ouif_compile_shader name type platform profile output_dir)
        add_custom_command(
            OUTPUT "${OUIF_SHADER_OUTPUT_DIR}/${output_dir}/${name}.bin"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${OUIF_SHADER_OUTPUT_DIR}/${output_dir}"
            COMMAND
                $<TARGET_FILE:shaderc>
                -f "${PROJECT_SOURCE_DIR}/src/shaders/${name}.sc"
                -o "${OUIF_SHADER_OUTPUT_DIR}/${output_dir}/${name}.bin"
                --type ${type}
                --platform ${platform}
                --profile ${profile}
                -i "${PROJECT_SOURCE_DIR}/src/shaders"
                -i "${bgfx_SOURCE_DIR}/bgfx/src"
            DEPENDS
                "${PROJECT_SOURCE_DIR}/src/shaders/${name}.sc"
                "${PROJECT_SOURCE_DIR}/src/shaders/varying.def.sc"
                shaderc
            VERBATIM
        )
    endfunction()

    if(WIN32)
        ouif_compile_shader(vs_ouif_rect vertex windows s_5_0 dx11)
        ouif_compile_shader(fs_ouif_rect fragment windows s_5_0 dx11)
        ouif_compile_shader(vs_ouif_text vertex windows s_5_0 dx11)
        ouif_compile_shader(fs_ouif_text fragment windows s_5_0 dx11)
        ouif_compile_shader(fs_ouif_texture fragment windows s_5_0 dx11)
        ouif_compile_shader(fs_ouif_blur fragment windows s_5_0 dx11)
        set(OUIF_SHADER_BINARIES
            "${OUIF_SHADER_OUTPUT_DIR}/dx11/vs_ouif_rect.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/dx11/fs_ouif_rect.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/dx11/vs_ouif_text.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/dx11/fs_ouif_text.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/dx11/fs_ouif_texture.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/dx11/fs_ouif_blur.bin"
        )
    elseif(APPLE)
        ouif_compile_shader(vs_ouif_rect vertex osx metal metal)
        ouif_compile_shader(fs_ouif_rect fragment osx metal metal)
        ouif_compile_shader(vs_ouif_text vertex osx metal metal)
        ouif_compile_shader(fs_ouif_text fragment osx metal metal)
        ouif_compile_shader(fs_ouif_texture fragment osx metal metal)
        ouif_compile_shader(fs_ouif_blur fragment osx metal metal)
        set(OUIF_SHADER_BINARIES
            "${OUIF_SHADER_OUTPUT_DIR}/metal/vs_ouif_rect.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/metal/fs_ouif_rect.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/metal/vs_ouif_text.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/metal/fs_ouif_text.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/metal/fs_ouif_texture.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/metal/fs_ouif_blur.bin"
        )
    else()
        ouif_compile_shader(vs_ouif_rect vertex linux 140 glsl)
        ouif_compile_shader(fs_ouif_rect fragment linux 140 glsl)
        ouif_compile_shader(vs_ouif_text vertex linux 140 glsl)
        ouif_compile_shader(fs_ouif_text fragment linux 140 glsl)
        ouif_compile_shader(fs_ouif_texture fragment linux 140 glsl)
        ouif_compile_shader(fs_ouif_blur fragment linux 140 glsl)
        set(OUIF_SHADER_BINARIES
            "${OUIF_SHADER_OUTPUT_DIR}/glsl/vs_ouif_rect.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/glsl/fs_ouif_rect.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/glsl/vs_ouif_text.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/glsl/fs_ouif_text.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/glsl/fs_ouif_texture.bin"
            "${OUIF_SHADER_OUTPUT_DIR}/glsl/fs_ouif_blur.bin"
        )
    endif()

    add_custom_target(ouif_shaders DEPENDS ${OUIF_SHADER_BINARIES})
    add_dependencies(ouif ouif_shaders)
endif()

if(MSVC)
    target_compile_options(ouif PRIVATE /W4 /permissive-)
else()
    target_compile_options(ouif PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(OUIF_BUILD_EXAMPLES)
    add_subdirectory(examples/hello)
    add_subdirectory(examples/complex_layout)
    add_subdirectory(examples/css_style)
    add_subdirectory(examples/animations)
    add_subdirectory(examples/blur_effect)
    add_subdirectory(examples/fonts)
    add_subdirectory(examples/xml_ui)
    add_subdirectory(examples/clipping_scroll)
endif()

if(OUIF_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(OUIF_INSTALL)
    install(
        TARGETS ouif
        EXPORT OUIFTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        FILES cmake/OUIFFunctions.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OUIF
    )

    if(TARGET ouif_shaders)
        install(
            DIRECTORY "${OUIF_SHADER_OUTPUT_DIR}/"
            DESTINATION ${CMAKE_INSTALL_BINDIR}/ouif-shaders
            OPTIONAL
        )
    endif()

    install(
        EXPORT OUIFTargets
        NAMESPACE OUIF::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OUIF
    )

    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/OUIFConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        cmake/OUIFConfig.cmake.in
        "${PROJECT_BINARY_DIR}/OUIFConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OUIF
    )

    install(
        FILES
            "${PROJECT_BINARY_DIR}/OUIFConfig.cmake"
            "${PROJECT_BINARY_DIR}/OUIFConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OUIF
    )
endif()
