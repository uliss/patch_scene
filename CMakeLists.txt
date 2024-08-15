# cmake_minimum_required(VERSION 3.5)
cmake_minimum_required(VERSION 3.21.1)

if(APPLE)
    set(LANGS CXX OBJCXX)
else()
    set(LANGS CXX)
endif()

project(patch_scene VERSION 0.2.0 LANGUAGES ${LANGS})
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
    diagram.h diagram.cpp
    device.h device.cpp
    connection.h connection.cpp
    deviceproperties.h deviceproperties.cpp deviceproperties.ui
    socket.h socket.cpp
    tablecellcheckbox.h tablecellcheckbox.cpp
    tablecellconnector.h tablecellconnector.cpp
    device_xlet.h device_xlet.cpp
    device_pixmap.h device_pixmap.cpp device_pixmap.ui
    about_window.h about_window.cpp about_window.ui
    preferences_dialog.h preferences_dialog.cpp preferences_dialog.ui
    device_library.h device_library.cpp
    undo_commands.h undo_commands.cpp
    device_common.h device_common.cpp
    export_document.h export_document.cpp
    diagram_image.h diagram_image.cpp
    resources/patch_scene_resources.qrc
    diagram_meta.h diagram_meta.cpp
    diagram_meta_dialog.h diagram_meta_dialog.cpp diagram_meta_dialog.ui
    diagram_item_model.h diagram_item_model.cpp
    favorites_widget.h favorites_widget.cpp
    diagram_state_machine.h diagram_state_machine.cpp
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

include(summary)
include(InstallRequiredSystemLibraries)

if(APPLE)
    configure_file(scripts/macos_make_dmg.sh ${PROJECT_BINARY_DIR}/macos_make_dmg.sh @ONLY)
    add_custom_target(dmg
        COMMAND ${PROJECT_BINARY_DIR}/macos_make_dmg.sh
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS patch_scene
    )
    add_custom_target(app-zip
        COMMAND zip -r
            "PatchScene-v${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.app.zip"
            "PatchScene.app/"
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/dist
    )
    add_custom_target(run
        COMMAND open ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.app
    )
endif()
