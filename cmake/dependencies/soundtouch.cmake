include(cmake/CPM.cmake)

set(SOUNDSTRETCH OFF)
set(SOUNDTOUCH_DLL OFF)
set(OPENMP ON)
set(BUILD_SHARED_LIBS OFF)
set(INTEGER_SAMPLES OFF)
CPMAddPackage(
    NAME soundtouch
    GIT_REPOSITORY "https://codeberg.org/soundtouch/soundtouch.git"
    GIT_TAG "2.3.1"
    VERSION "2.3.1"
)

if (soundtouch_ADDED)
    add_library(soundtouch::soundtouch ALIAS SoundTouch)
endif()