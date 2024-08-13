find_package(Git)

add_custom_target(git_version
    ${CMAKE_COMMAND}
        -D SRC=${CMAKE_SOURCE_DIR}/patch_scene_version.h.in
        -D DST=${CMAKE_BINARY_DIR}/patch_scene_version.h
        -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
        -P ${CMAKE_SOURCE_DIR}/cmake/git_version_generate_header.cmake
)

