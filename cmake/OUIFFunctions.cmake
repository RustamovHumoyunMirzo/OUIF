function(ouif_target_defaults target)
    if(NOT TARGET "${target}")
        message(FATAL_ERROR "ouif_target_defaults expected an existing target, got '${target}'")
    endif()

    target_compile_features("${target}" PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options("${target}" PRIVATE /W4 /permissive-)
    else()
        target_compile_options("${target}" PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

function(ouif_add_app target)
    cmake_parse_arguments(OUIF_APP "" "" "SOURCES" ${ARGN})
    if(NOT OUIF_APP_SOURCES)
        message(FATAL_ERROR "ouif_add_app requires SOURCES")
    endif()

    add_executable("${target}" ${OUIF_APP_SOURCES})
    target_link_libraries("${target}" PRIVATE OUIF::ouif)
    ouif_target_defaults("${target}")
endfunction()
