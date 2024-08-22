find_package(ImageMagick)

if(ImageMagick_FOUND)
    set(SVG_APP_ICON ${PROJECT_SOURCE_DIR}/resources/app_icon.svg)
    set(SVG_FILE_ICON ${PROJECT_SOURCE_DIR}/resources/file_icon.svg)
    set(ICO_ICON ${PROJECT_SOURCE_DIR}/resources/app_icon_win.ico)
    add_custom_command(
        OUTPUT  ${ICO_ICON}
        COMMAND ${ImageMagick_EXECUTABLE_DIR}/convert -background none ${SVG_APP_ICON} -resize 256x256 ${ICO_ICON}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources
        DEPENDS ${SVG_APP_ICON}
        USES_TERMINAL
        VERBATIM
    )
    add_custom_target(generate_ico DEPENDS ${ICO_ICON})
    add_dependencies(patch_scene generate_ico)

    if(APPLE)
        set(ICNS_APP_ICON ${PROJECT_SOURCE_DIR}/resources/patch_scene.icns)
        set(ICNS_FILE_ICON ${PROJECT_SOURCE_DIR}/resources/patch_scene_file.icns)
        get_filename_component(MAGICK_HOME ${ImageMagick_EXECUTABLE_DIR} DIRECTORY)

        add_custom_command(
            OUTPUT  ${ICNS_APP_ICON}
            COMMAND MAGICK_HOME=${MAGICK_HOME} ${PROJECT_SOURCE_DIR}/scripts/svg2icns.bash ${SVG_APP_ICON} ${ICNS_APP_ICON}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources
            DEPENDS ${SVG_APP_ICON}
            USES_TERMINAL
            VERBATIM
        )
        add_custom_command(
            OUTPUT ${ICNS_FILE_ICON}
            COMMAND MAGICK_HOME=${MAGICK_HOME} ${PROJECT_SOURCE_DIR}/scripts/svg2icns.bash ${SVG_FILE_ICON} ${ICNS_FILE_ICON}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources
            DEPENDS ${SVG_FILE_ICON}
            USES_TERMINAL
            VERBATIM
        )
        add_custom_target(generate_icns DEPENDS ${ICNS_APP_ICON} ${ICNS_FILE_ICON})
    endif()
endif()
