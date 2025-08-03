# dump a (sorted) list of cmake variables
function(liquid_debug_print_vars)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()

liquid_debug_print_vars()
message("top: CMAKE_SOURCE_DIR         = ${CMAKE_SOURCE_DIR}")
message("top: CMAKE_BINARY_DIR         = ${CMAKE_BINARY_DIR}")
message("top: CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message("top: CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")
