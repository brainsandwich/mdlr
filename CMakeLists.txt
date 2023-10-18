cmake_minimum_required(VERSION 3.22)
project(mdlr)

# ------------------ DEFAULT COMPILATION FLAGS

set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/ext" CACHE STRING "")
set(CPM_USE_LOCAL_PACKAGES ON CACHE BOOL "")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies)

# ------------------ OPTIONS

option(MDLR_COPY_ASSETS "Copy assets near app folder" OFF)
option(MDLR_BUILD_TESTS "Build unit tests" OFF)
option(MDLR_BUILD_WALL "Hard warnings" OFF)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    option(MDLR_BUILD_ASAN "Compile with Asan (clang)" OFF)
endif()

# ------------------ SOURCES

file(GLOB_RECURSE mdlr_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mdlr/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mdlr/*.h"
)
add_library(mdlr ${mdlr_sources})
add_library(mdlr::mdlr ALIAS mdlr)
target_include_directories(mdlr PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(mdlr PUBLIC cxx_std_23)
set_target_properties(mdlr PROPERTIES FOLDER "mdlr")

add_executable(mdlr_app "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cc")
target_link_libraries(mdlr_app PRIVATE mdlr)
set_target_properties(mdlr_app PROPERTIES FOLDER "mdlr")

# ------------------ BASE CONFIG

if (MDLR_BUILD_WALL)
    if (MSVC)
        target_compile_options(mdlr PUBLIC /W4)
    else()
        target_compile_options(mdlr PUBLIC -Wall -Wextra -Wpedantic)
    endif()
endif()

if (MDLR_BUILD_ASAN)
    if (MSVC)
        message(AUTHOR_WARNING "is ASAN a valid option on MSVC ??")
    else()
        target_compile_options(mdlr PUBLIC -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(mdlr PUBLIC -fsanitize=address)
    endif()
endif()

# ------------------ DEPENDENCIES

include(fmt)
include(miniaudio)
include(stb)
include(libremidi)
include(soundtouch)
include(rubberband)

target_compile_definitions(mdlr
    PUBLIC
        MDLR_USE_MINIAUDIO
        MDLR_USE_VORBIS
        MDLR_USE_LIBREMIDI
)
target_link_libraries(mdlr
    PUBLIC
        fmt::fmt
        libremidi::libremidi
    PRIVATE
        miniaudio::miniaudio
        stb::vorbis
)

if (TARGET soundtouch::soundtouch)
    target_compile_definitions(mdlr PUBLIC MDLR_USE_SOUNDTOUCH)
    target_link_libraries(mdlr PRIVATE soundtouch::soundtouch)
endif()

if (TARGET rubberband::rubberband)
    target_compile_definitions(mdlr PUBLIC MDLR_USE_RUBBERBAND)
    target_link_libraries(mdlr PRIVATE rubberband::rubberband)
endif()

# ------------------ OTHER TARGETS

if (MDLR_COPY_ASSETS)
    add_custom_target(mdlr_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:mdlr>/assets"
    )
    add_dependencies(mdlr mdlr_assets)
    set_property(TARGET mdlr_assets PROPERTY FOLDER "mdlr")
endif()

if (MDLR_BUILD_TESTS)
    enable_testing()
    add_executable(mdlr_test_example ${CMAKE_CURRENT_SOURCE_DIR}/tests/example.cc)
    target_link_libraries(mdlr_test_example
        PRIVATE
            mdlr::mdlr
            fmt::fmt
    )
    set_target_properties(mdlr_test_example
        PROPERTIES
            FOLDER "mdlr/tests"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
    add_test(NAME mdlr_test_example COMMAND mdlr_test_example)
    # .... ?
    # add_executable(mdlr_tests)
    # set_target_properties(mdlr_tests PROPERTIES FOLDER "mdlr")
endif()