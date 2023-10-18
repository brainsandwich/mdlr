include(CPM)

CPMAddPackage(
    NAME sokol
    GIT_REPOSITORY "https://github.com/floooh/sokol.git"
    GIT_TAG "00b2083a1876e0190e6c1915f3aa3edb68b75b46"
    VERSION "1.0.0"
    DOWNLOAD_ONLY ON
)
if (sokol_ADDED)
    message("Sokol added")
    add_library(sokol INTERFACE)
    add_library(sokol::sokol ALIAS sokol)
    target_include_directories(sokol INTERFACE "${sokol_SOURCE_DIR}" "${sokol_SOURCE_DIR}/util")
endif()