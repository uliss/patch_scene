cmake_minimum_required(VERSION 3.21.1)

if(APPLE)
    set(LANGS CXX OBJCXX)
else()
    set(LANGS CXX)
endif()

project(
    patch_scene
    VERSION 0.3.5
    LANGUAGES ${LANGS})
# cmake search path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(git_version)

enable_testing(true)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets SvgWidgets PrintSupport Network Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets SvgWidgets PrintSupport Network
                                                       LinguistTools Test)

set(PROJECT_SOURCES
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resources/patch_scene_resources.qrc
    about_window.cpp
    about_window.h
    about_window.ui
    about_shortcuts.h
    about_shortcuts.cpp
    about_shortcuts.ui
    app_settings.cpp
    app_settings.h
    app_version.cpp
    app_version.h
    application.cpp
    application.h
    background_properties_dialog.h
    background_properties_dialog.cpp
    background_properties_dialog.ui
    battery.cpp
    battery.h
    battery_item_model.cpp
    battery_item_model.h
    bezier_editor_handle.h
    bezier_editor_handle.cpp
    connection.cpp
    connection.h
    connection_data.cpp
    connection_data.h
    connection_database.cpp
    connection_database.h
    connection_editor.h
    connection_editor.cpp
    connection_item_model.cpp
    connection_item_model.h
    connection_style.cpp
    connection_style.h
    connector_type.cpp
    connector_type.h
    device.cpp
    device.h
    device_common.cpp
    device_common.h
    device_item_model.cpp
    device_item_model.h
    device_library.cpp
    device_library.h
    device_pixmap.cpp
    device_pixmap.h
    device_pixmap.ui
    device_xlet.cpp
    device_xlet.h
    device_xlet_view.cpp
    device_xlet_view.h
    deviceproperties.cpp
    deviceproperties.h
    deviceproperties.ui
    diagram.cpp
    diagram.h
    diagram_item_model.cpp
    diagram_item_model.h
    diagram_meta.cpp
    diagram_meta.h
    diagram_meta_dialog.cpp
    diagram_meta_dialog.h
    diagram_meta_dialog.ui
    diagram_scene.cpp
    diagram_scene.h
    diagram_state_machine.cpp
    diagram_state_machine.h
    diagram_updates_blocker.cpp
    diagram_updates_blocker.h
    export_document.cpp
    export_document.h
    favorites_widget.cpp
    favorites_widget.h
    furniture_item_model.cpp
    furniture_item_model.h
    library_item_model.h
    library_item_model.cpp
    logging.hpp
    preferences_dialog.cpp
    preferences_dialog.h
    preferences_dialog.ui
    return_item_model.cpp
    return_item_model.h
    scale_widget.h
    scale_widget.cpp
    scene_background.cpp
    scene_background.h
    scene_connections.cpp
    scene_connections.h
    scene_devices.cpp
    scene_devices.h
    segment_point_handle.h
    segment_point_handle.cpp
    send_item_model.cpp
    send_item_model.h
    socket.cpp
    socket.h
    svg_render_factory.cpp
    svg_render_factory.h
    table_cell_power.cpp
    table_cell_power.h
    table_cell_socket.cpp
    table_cell_socket.h
    tablecellcheckbox.cpp
    tablecellcheckbox.h
    tablecellconnector.cpp
    tablecellconnector.h
    undo_commands.cpp
    undo_commands.h
    vendor_list.cpp
    vendor_list.h
    xlet_info.cpp
    xlet_info.h
    xlet_view_index.cpp
    xlet_view_index.h
    xlets_logic_view_data.cpp
    xlets_logic_view_data.h
    xlets_logic_view_editor.cpp
    xlets_logic_view_editor.h
    xlets_logic_view_editor.ui
    xlets_user_view.cpp
    xlets_user_view.h
    xlets_user_view_data.cpp
    xlets_user_view_data.h
    xlets_user_view_editor.cpp
    xlets_user_view_editor.h
    xlets_user_view_editor.ui
    xlets_view.cpp
    xlets_view.h
)

if(WIN32)
    configure_file(patch_scene_win.rc ${PROJECT_BINARY_DIR}/patch_scene_win.rc @ONLY)
endif()

if(APPLE)
    list(APPEND PROJECT_SOURCES macos_utils.h macos_utils.mm)
    set(MACOSX_BUNDLE_ICON_FILE patch_scene.icns)
    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/resources/patch_scene.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1. If you are developing
    # for iOS or macOS you should consider setting an explicit, fixed bundle identifier manually
    # though.
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

add_subdirectory(3rd-party/QSimpleUpdater)
target_compile_definitions(QSimpleUpdater PUBLIC QSU_INCLUDE_MOC)

add_library(patch_scene_lib STATIC ${PROJECT_SOURCES})
target_include_directories(patch_scene_lib PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(
    patch_scene_lib PRIVATE QSimpleUpdater Qt${QT_VERSION_MAJOR}::Widgets
                            Qt${QT_VERSION_MAJOR}::PrintSupport Qt${QT_VERSION_MAJOR}::SvgWidgets)

qt_add_executable(patch_scene MANUAL_FINALIZATION MACOSX_BUNDLE ${app_icon_macos} main.cpp
                  resources/patch_scene_resources.qrc)

add_dependencies(patch_scene_lib git_version)

target_link_libraries(
    patch_scene PRIVATE patch_scene_lib Qt${QT_VERSION_MAJOR}::Widgets
                        Qt${QT_VERSION_MAJOR}::PrintSupport Qt${QT_VERSION_MAJOR}::SvgWidgets)

set_target_properties(
    patch_scene
    PROPERTIES ${BUNDLE_ID_OPTION} MACOSX_BUNDLE_BUNDLE_VERSION
               ${PROJECT_VERSION} MACOSX_BUNDLE_SHORT_VERSION_STRING
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} MACOSX_BUNDLE
               TRUE WIN32_EXECUTABLE
               TRUE)

add_custom_target(translations SOURCES patch_scene_ru.ts)
add_custom_target(misc SOURCES CHANGELOG.md resources/updates.in.json tests/test_updates.json)

include(GNUInstallDirs)
install(
    TARGETS patch_scene
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(patch_scene)
    # qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ru)
    set(LUPDATE_OPTIONS "")
    # set(LUPDATE_OPTIONS "-no-obsolete")

    qt_add_translations(
        patch_scene
        TARGETS
        patch_scene
        patch_scene_lib
        SOURCE_TARGETS
        patch_scene
        patch_scene_lib
        SOURCES
        ${PROJECT_SOURCES}
        3rd-party/QSimpleUpdater/src/Downloader.cpp
        3rd-party/QSimpleUpdater/src/Downloader.ui
        3rd-party/QSimpleUpdater/src/QSimpleUpdater.cpp
        3rd-party/QSimpleUpdater/src/Updater.cpp
        TS_FILES
        patch_scene_ru.ts
        LUPDATE_OPTIONS
        ${LUPDATE_OPTIONS})
endif()

include(icon_tools)
include(jq_format)
include(summary)
include(InstallRequiredSystemLibraries)

add_subdirectory(tests)

find_program(GITHUB_CLI gh gh.exe)
if(GITHUB_CLI)
    message(STATUS "GitHub command line found: ${GITHUB_CLI}")
endif()

# package
set(INNO_INSTALLER_NAME "PatchScene-win-v${PROJECT_VERSION}-setup")
set(WIN_INSTALLER_EXE "${INNO_INSTALLER_NAME}.exe")
set(DMG_FILE "PatchScene-mac-v${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.dmg")
set(MACOS_DMG_ARM64 "PatchScene-mac-v${PROJECT_VERSION}-arm64.dmg")
set(MACOS_DMG_X86_64 "PatchScene-mac-v${PROJECT_VERSION}-x86_64.dmg")

configure_file(resources/updates.in.json ${PROJECT_SOURCE_DIR}/resources/updates.json @ONLY)

if(APPLE)
    find_program(PLIST_BUDDY PlistBuddy PATHS /usr/libexec)
    find_program(DMGBUILD dmgbuild PATHS $ENV["PATH"])

    set(FILE_ICNS "${PROJECT_SOURCE_DIR}/resources/patch_scene_file.icns")
    set(APP_ZIP_FILE "PatchScene-mac-v${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.app.zip")

    find_program(NINJA ninja)
    if(NINJA)
        configure_file(scripts/macos_build_ninja.sh ${PROJECT_BINARY_DIR}/build.sh @ONLY)
    else()
        configure_file(scripts/macos_build_make.sh ${PROJECT_BINARY_DIR}/build.sh @ONLY)
    endif()

    configure_file(scripts/macos_deploy.sh ${PROJECT_BINARY_DIR}/deploy.sh @ONLY)
    configure_file(scripts/macos_make_dmg.sh ${PROJECT_BINARY_DIR}/macos_make_dmg.sh @ONLY)
    configure_file(resources/dmg_settings.json ${PROJECT_BINARY_DIR}/dmg_settings.json @ONLY)
    add_custom_target(
        dmg
        COMMAND ${PROJECT_BINARY_DIR}/macos_make_dmg.sh
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS patch_scene
        USES_TERMINAL)
    add_custom_target(
        app-zip
        COMMAND zip -r ${APP_ZIP_FILE} "PatchScene.app/"
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/dist
        USES_TERMINAL)
    add_custom_target(run COMMAND open ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.app)

    if(GITHUB_CLI)
        add_custom_target(
            dev_new_release
            COMMAND git push --tags
            COMMAND ${GITHUB_CLI} release create "v${PROJECT_VERSION}"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            USES_TERMINAL)
        add_custom_target(
            dev_upload
            COMMAND ${GITHUB_CLI} release upload --clobber "v${PROJECT_VERSION}" ${APP_ZIP_FILE}
            COMMAND ${GITHUB_CLI} release upload --clobber "v${PROJECT_VERSION}" ${DMG_FILE}
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/dist
            USES_TERMINAL)
    endif()
endif()

if(WIN32)
    target_sources(patch_scene PRIVATE ${PROJECT_BINARY_DIR}/patch_scene_win.rc)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/dist)

    install(
        TARGETS patch_scene
        BUNDLE DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    find_program(
        7Z_EXE
        NAMES "7z" "7z.exe"
        PATH_SUFFIXES "7-Zip")
    if(7Z_EXE)
        message(STATUS "7z found at: ${7Z_EXE}")
        message(STATUS "app-zip is target available")
        set(APP_ZIP_NAME "PatchScene-win-v${PROJECT_VERSION}.zip")

        add_custom_target(
            app-zip
            COMMAND ${7Z_EXE} a ${APP_ZIP_NAME} "${CMAKE_INSTALL_PREFIX}/bin/*"
            USES_TERMINAL)
    endif()

    configure_file(${PROJECT_SOURCE_DIR}/patch_scene.iss ${PROJECT_BINARY_DIR}/patch_scene.iss
                   @ONLY)

    if(GITHUB_CLI)
        add_custom_target(
            dev_upload
            COMMAND ${GITHUB_CLI} release upload --clobber "v${PROJECT_VERSION}" ${APP_ZIP_NAME}
            COMMAND ${GITHUB_CLI} release upload --clobber "v${PROJECT_VERSION}"
                    ${WIN_INSTALLER_EXE}
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            USES_TERMINAL)
    endif()

    find_program(
        ISCC_PATH
        NAMES ISCC.exe
        PATHS "$ENV{PROGRAMFILES}/Inno Setup 5" "C:/Program Files (x86)/Inno Setup 5"
              "$ENV{PROGRAMFILES}/Inno Setup 6" "C:/Program Files (x86)/Inno Setup 6")

    if(ISCC_PATH)
        message(STATUS "Inno Setup Installed found: ${ISCC_PATH}")
        message(
            STATUS "     You could type `make inno` after `make install` to build installer package"
        )
        add_custom_target(
            inno
            COMMAND ${ISCC_PATH} "${PROJECT_BINARY_DIR}/patch_scene.iss"
            USES_TERMINAL)
    endif()

    find_program(NINJA ninja)
    if(NINJA)
        configure_file(scripts/mingw_build_ninja.sh ${PROJECT_BINARY_DIR}/build.sh @ONLY)
    else()
        configure_file(scripts/mingw_build_make.sh ${PROJECT_BINARY_DIR}/build.sh @ONLY)
    endif()

    configure_file(cmake/mingw_deploy.cmake.in ${PROJECT_BINARY_DIR}/mingw_deploy.cmake @ONLY)
    configure_file(scripts/mingw_deploy.sh ${PROJECT_BINARY_DIR}/mingw_deploy.sh @ONLY)
    configure_file(scripts/mingw_deploy_all.sh ${PROJECT_BINARY_DIR}/deploy.sh @ONLY)
    install(SCRIPT ${PROJECT_BINARY_DIR}/mingw_deploy.cmake)
endif()
