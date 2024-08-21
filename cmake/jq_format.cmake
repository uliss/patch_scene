find_program(JQ_EXE jq)
if(JQ_EXE)
    add_custom_target(format_library
        COMMAND ${JQ_EXE} -e < ${PROJECT_SOURCE_DIR}/resources/library.json > ${PROJECT_SOURCE_DIR}/resources/library2.json
        COMMAND mv library2.json library.json
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/resources
    )
endif()
