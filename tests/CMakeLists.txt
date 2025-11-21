qt_add_executable(
    psc_tests
    json_common.h
    json_common.cpp
    test_main.cpp
    ../resources/patch_scene_resources.qrc
    # tests
    test_connection.h
    test_connection.cpp
    test_connector_type.h
    test_connector_type.cpp
    test_device_library.h
    test_device_library.cpp
    test_diagram.h
    test_diagram.cpp
    test_scene_devices.h
    test_scene_devices.cpp
    test_xlet.h
    test_xlet.cpp
    test_scene_connections.h
    test_scene_connections.cpp
    test_device_data.h
    test_device_data.cpp
    test_connection_database.h test_connection_database.cpp
    test_scene_background.h test_scene_background.cpp
    test_device_xlet_view.h test_device_xlet_view.cpp
    test_device.h test_device.cpp
    test_subcategory.h test_subcategory.cpp
    test_segment_points.h test_segment_points.cpp
    test_xlets_user_view.h test_xlets_user_view.cpp
    test_connection_view_data.h test_connection_view_data.cpp
    test_common.h test_common.cpp
    test_comment.h test_comment.cpp)

add_custom_target(
    test_src
    SOURCES test_device_library_1.json test_device_library_2.json test_device_library_3.json
            test_device_library_lib1.json test_scene_devices_1.json)

target_compile_definitions(psc_tests PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(psc_tests PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(psc_tests PRIVATE patch_scene_lib Qt::Test Qt6::Widgets Qt6::PrintSupport
                                        Qt6::SvgWidgets)

add_test(NAME psc_tests COMMAND psc_tests)
