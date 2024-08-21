# cmake_minimum_required(VERSION 3.5)
cmake_minimum_required(VERSION 3.21.1)

if(APPLE)
    set(LANGS CXX OBJCXX)
else()
    set(LANGS CXX)
endif()

project(patch_scene VERSION 0.2.5 LANGUAGES ${LANGS})
# cmake search path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(git_version)

enable_testing(true)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets SvgWidgets PrintSupport Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SvgWidgets PrintSupport LinguistTools Test)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(WIN32)
    list(APPEND PROJECT_SOURCES app.rc)
endif()

if(APPLE)
    list(APPEND PROJECT_SOURCES macos_utils.h macos_utils.mm)
    set(MACOSX_BUNDLE_ICON_FILE patch_scene.icns)
    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/patch_scene.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
    # If you are developing for iOS or macOS you should consider setting an
    # explicit, fixed bundle identifier manually though.
    if(${QT_VERSION} VERSION_LESS 6.1.0)
        set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER space.ceam.patch_scene)
    endif()

    set(MACOSX_BUNDLE_BUNDLE_NAME "PatchScene")
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_COPYRIGHT "ceam.space")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "PatchScene")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
endif()

qt_add_executable(patch_scene
    MANUAL_FINALIZATION
    MACOSX_BUNDLE
    ${PROJECT_SOURCES}
    ${app_icon_macos}
    about_window.h about_window.cpp about_window.ui
    app_settings.h app_settings.cpp
    application.h application.cpp
    battery_item_model.h battery_item_model.cpp
    connection.h connection.cpp
    connection_item_model.h connection_item_model.cpp
    device.h device.cpp
    device_common.h device_common.cpp
    device_item_model.h device_item_model.cpp
    device_library.h device_library.cpp
    device_pixmap.h device_pixmap.cpp device_pixmap.ui
    device_xlet.h device_xlet.cpp
    deviceproperties.h deviceproperties.cpp deviceproperties.ui
    diagram.h diagram.cpp
    diagram_image.h diagram_image.cpp
    diagram_item_model.h diagram_item_model.cpp
    diagram_meta.h diagram_meta.cpp
    diagram_meta_dialog.h diagram_meta_dialog.cpp diagram_meta_dialog.ui
    diagram_state_machine.h diagram_state_machine.cpp
    export_document.h export_document.cpp
    favorites_widget.h favorites_widget.cpp
    furniture_item_model.h furniture_item_model.cpp
    preferences_dialog.h preferences_dialog.cpp preferences_dialog.ui
    resources/patch_scene_resources.qrc
    return_item_model.h return_item_model.cpp
    send_item_model.h send_item_model.cpp
    socket.h socket.cpp
    tablecellcheckbox.h tablecellcheckbox.cpp
    tablecellconnector.h tablecellconnector.cpp
    undo_commands.h undo_commands.cpp
    #tests/test_device_data.h tests/test_device_data.cpp
)

target_include_directories(patch_scene PRIVATE ${PROJECT_BINARY_DIR})
add_dependencies(patch_scene git_version)

target_link_libraries(patch_scene
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport Qt${QT_VERSION_MAJOR}::SvgWidgets)

set_target_properties(patch_scene PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

add_custom_target(translations
    SOURCES
        patch_scene_ru.ts
)

include(GNUInstallDirs)
install(TARGETS patch_scene
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(patch_scene)
    # qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ru)

    qt_add_translations(
        patch_scene
        TS_FILES patch_scene_ru.ts
    )
endif()

include(icon_tools)
include(summary)
include(InstallRequiredSystemLibraries)

find_program(GITHUB_CLI gh)

if(APPLE)
    find_program(PLIST_BUDDY PlistBuddy PATHS /usr/libexec)
    find_program(DMGBUILD dmgbuild PATHS $ENV["PATH"])

    set(FILE_ICNS "${PROJECT_SOURCE_DIR}/resources/patch_scene_file.icns")
    set(APP_ZIP_FILE "PatchScene-v${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.app.zip")
    set(DMG_FILE "PatchScene-v${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.dmg")

    configure_file(scripts/macos_make_dmg.sh ${PROJECT_BINARY_DIR}/macos_make_dmg.sh @ONLY)
    configure_file(resources/dmg_settings.json ${PROJECT_BINARY_DIR}/dmg_settings.json @ONLY)
    add_custom_target(dmg
        COMMAND ${PROJECT_BINARY_DIR}/macos_make_dmg.sh
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS patch_scene
    )
    add_custom_target(app-zip
        COMMAND zip -r ${APP_ZIP_FILE} "PatchScene.app/"
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/dist
    )
    add_custom_target(run
        COMMAND open ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.app
    )

    if(GITHUB_CLI)
        add_custom_target(dev_new_release
            COMMAND git push --tags
            COMMAND ${GITHUB_CLI} release create "v${PROJECT_VERSION}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
        add_custom_target(dev_upload
            COMMAND ${GITHUB_CLI} release upload --clobber "v${PROJECT_VERSION}" ${APP_ZIP_FILE}
            COMMAND ${GITHUB_CLI} release upload --clobber "v${PROJECT_VERSION}" ${DMG_FILE}
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/dist
        )
    endif()
endif()

if(WIN32)
    install(TARGETS patch_scene
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    qt6_generate_deploy_app_script(
        TARGET patch_scene
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
