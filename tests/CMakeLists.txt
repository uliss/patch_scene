qt_add_executable(psc_tests
    test_suite.h test_suite.cpp
    test_main.cpp
    test_connection.h test_connection.cpp
)

target_include_directories(psc_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)
target_link_libraries(psc_tests
    PRIVATE
        Qt::Test
        Qt6::Widgets Qt6::PrintSupport Qt6::SvgWidgets
)

add_test(NAME psc_tests COMMAND psc_tests)
