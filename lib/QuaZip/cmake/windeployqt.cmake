get_target_property(_qmake_executable Qt${QUAZIP_QT_MAJOR_VERSION}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

function(windeployqt target)

    # --verbose 1

    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${_qt_bin_dir}/windeployqt.exe"
            --verbose 1
            $<$<CONFIG:Debug>:--debug>
            $<$<CONFIG:Release>:--release>
            --no-plugins
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            --no-compiler-runtime
            \"$<TARGET_FILE:${target}>\"
            COMMENT "Deploying Qt libraries using windeployqt for compilation target '${target}' ..."
    )

endfunction()