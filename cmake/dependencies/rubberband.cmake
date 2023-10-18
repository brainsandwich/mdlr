include(CPM)

CPMAddPackage(
    NAME rubberband
    GIT_REPOSITORY https://github.com/breakfastquay/rubberband.git
    VERSION 3.2.1
    DOWNLOAD_ONLY
)

if (rubberband_ADDED)
    set(rubberband_SOURCES
        ${rubberband_SOURCE_DIR}/src/rubberband-c.cpp
        ${rubberband_SOURCE_DIR}/src/RubberBandStretcher.cpp
        ${rubberband_SOURCE_DIR}/src/faster/AudioCurveCalculator.cpp
        ${rubberband_SOURCE_DIR}/src/faster/CompoundAudioCurve.cpp
        ${rubberband_SOURCE_DIR}/src/faster/HighFrequencyAudioCurve.cpp
        ${rubberband_SOURCE_DIR}/src/faster/SilentAudioCurve.cpp
        ${rubberband_SOURCE_DIR}/src/faster/PercussiveAudioCurve.cpp
        ${rubberband_SOURCE_DIR}/src/faster/R2Stretcher.cpp
        ${rubberband_SOURCE_DIR}/src/faster/StretcherChannelData.cpp
        ${rubberband_SOURCE_DIR}/src/faster/StretcherProcess.cpp
        ${rubberband_SOURCE_DIR}/src/common/Allocators.cpp
        ${rubberband_SOURCE_DIR}/src/common/FFT.cpp
        ${rubberband_SOURCE_DIR}/src/common/Log.cpp
        ${rubberband_SOURCE_DIR}/src/common/Profiler.cpp
        ${rubberband_SOURCE_DIR}/src/common/Resampler.cpp
        ${rubberband_SOURCE_DIR}/src/common/StretchCalculator.cpp
        ${rubberband_SOURCE_DIR}/src/common/sysutils.cpp
        ${rubberband_SOURCE_DIR}/src/common/mathmisc.cpp
        ${rubberband_SOURCE_DIR}/src/common/Thread.cpp
        ${rubberband_SOURCE_DIR}/src/common/BQResampler.cpp
        ${rubberband_SOURCE_DIR}/src/finer/R3Stretcher.cpp
    )

    add_library(rubberband ${rubberband_SOURCES})
    add_library(rubberband::rubberband ALIAS rubberband)
    target_include_directories(rubberband
        PUBLIC ${rubberband_SOURCE_DIR}/rubberband
        PRIVATE ${rubberband_SOURCE_DIR}/src
    )

    # Not used yet
    set(RUBBERBAND_FFT CACHE STRING builtin)
    set_property(CACHE RUBBERBAND_FFT PROPERTY STRINGS auto builtin vdsp kissfft fftw sleef ipp)
    set(RUBBERBAND_RESAMPLER CACHE STRING builtin)
    set_property(CACHE RUBBERBAND_RESAMPLER PROPERTY STRINGS auto builtin libsamplerate speex libspeexdsp ipp)
    target_compile_definitions(rubberband
        PRIVATE
            USE_BUILTIN_FFT
            USE_BQRESAMPLER
            USE_PTHREADS
            LACK_SINCOS
    )
endif()