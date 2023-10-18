#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <functional>

namespace mdlr
{
    enum class DriverBackend
    {
        Miniaudio
    };

    struct DeviceID
    {
        union {
            char name[256];
            uint32_t id;
        };
    };
    static constexpr DeviceID NullDevice = { .name = {} };

    struct DeviceSelector
    {
        DeviceID id = NullDevice;
        std::string name;
        bool any = false;
    };

    struct Driver;
    static constexpr int DefaultSampleRate = 44100;
    static constexpr int DefaultBufferSize = 1024;
    struct DriverConfiguration
    {
        struct {
            DeviceSelector selector = { .any = false };
        } capture;
        struct {
            DeviceSelector selector = { .any = true };
        } playback;
        int samplerate = DefaultSampleRate;
        int buffersize = DefaultBufferSize;
    };

    struct Driver
    {
        using Callback = std::function<void(const float*, float*, int)>;

        struct {
            DeviceID deviceid = NullDevice;
            int channels = 0;
        } capture;
        struct {
            DeviceID deviceid = NullDevice;
            int channels = 0;
        } playback;
        int samplerate = DefaultSampleRate;
        int buffersize = DefaultBufferSize;
        Callback callback;

        virtual ~Driver() = default;
        virtual bool configure(const DriverConfiguration&) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;

        static std::unique_ptr<Driver> create(DriverBackend);
    };
}