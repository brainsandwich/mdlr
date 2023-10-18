include(CPM)
# find_package(ZLIB)
# if (NOT ZLIB_FOUND)
    CPMAddPackage(
        NAME zlib
        GIT_REPOSITORY "git@github.com:madler/zlib.git"
        GIT_TAG "v1.2.13"
        VERSION "1.2.13"
    )
# endif()