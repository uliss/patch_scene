qt_add_executable(psc_tests
    test_suite.h test_suite.cpp
    test_main.cpp
    test_connection.h test_connection.cpp
    test_xlet.h test_xlet.cpp
    test_device_library.h test_device_library.cpp
    json_common.h json_common.cpp
)

add_custom_target(test_src
    SOURCES
    test_device_library_1.json
    test_device_library_2.json
    test_device_library_3.json
)

target_compile_definitions(psc_tests
    PRIVATE
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(psc_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)
target_link_libraries(psc_tests
    PRIVATE
        patch_scene_lib
        Qt::Test
        Qt6::Widgets Qt6::PrintSupport Qt6::SvgWidgets
)

add_test(NAME psc_tests COMMAND psc_tests)
