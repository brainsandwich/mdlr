include(CPM)

set(LIBREMIDI_TESTS OFF)
CPMAddPackage(
    NAME libremidi
    GIT_REPOSITORY "https://github.com/jcelerier/libremidi.git"
    GIT_TAG "v3.0"
    VERSION 3.0
)