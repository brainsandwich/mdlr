include(cmake/CPM.cmake)

CPMAddPackage(
    NAME stb
    GIT_REPOSITORY "https://github.com/nothings/stb.git"
    GIT_TAG "5736b15f7ea0ffb08dd38af21067c314d6a3aae9"
    DOWNLOAD_ONLY ON
)
if (stb_ADDED)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")

    add_library(stb_vorbis INTERFACE)
    target_link_libraries(stb_vorbis INTERFACE stb)
    set_target_properties(stb_vorbis PROPERTIES VERSION 1.22)
    add_library(stb::vorbis ALIAS stb_vorbis)

    add_library(stb_truetype INTERFACE)
    target_link_libraries(stb_truetype INTERFACE stb)
    set_target_properties(stb_truetype PROPERTIES VERSION 1.26)
    add_library(stb::truetype ALIAS stb_truetype)
endif()