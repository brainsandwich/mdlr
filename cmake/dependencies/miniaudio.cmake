include(cmake/CPM.cmake)

CPMAddPackage(
    NAME miniaudio
    VERSION 0.11.17
    GIT_REPOSITORY "https://github.com/mackron/miniaudio.git"
    GIT_TAG "0.11.17"
    DOWNLOAD_ONLY ON
)
if (miniaudio_ADDED)
    add_library(miniaudio INTERFACE)
    add_library(miniaudio::miniaudio ALIAS miniaudio)
    target_include_directories(miniaudio INTERFACE "${miniaudio_SOURCE_DIR}")
    target_compile_definitions(miniaudio
        INTERFACE
            MA_NO_DECODING
            MA_NO_ENCODING
            MA_NO_WAV
            MA_NO_FLAC
            MA_NO_MP3
            MA_NO_GENERATION
            MA_DEBUG_OUTPUT
    )
endif()