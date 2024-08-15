find_package(ImageMagick)

if(ImageMagick_FOUND)
    set(SVG_ICON ${PROJECT_SOURCE_DIR}/resources/app_icon.svg)
    set(ICO_ICON ${PROJECT_SOURCE_DIR}/resources/app_icon_win.ico)
    add_custom_command(
        OUTPUT ${PROJECT_SOURCE_DIR}/resources/app_icon_win.ico
        COMMAND ${ImageMagick_EXECUTABLE_DIR}/convert ${SVG_ICON} -resize 256x256 ${ICO_ICON}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources
        DEPENDS ${SVG_ICON}
        USES_TERMINAL
        VERBATIM
    )
    add_custom_target(generate_ico DEPENDS ${ICO_ICON})
    add_dependencies(patch_scene generate_ico)
endif()
