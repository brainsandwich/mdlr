#include "mdlr/driver.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <fmt/format.h>

#include <CoreAudio/CoreAudio.h>

namespace mdlr
{
    struct MiniaudioDriver: Driver
    {
        ma_context context;
        ma_device device;

        MiniaudioDriver()
        {
            ma_context_config config = ma_context_config_init();
            if (ma_context_init(nullptr, 0, &config, &context) != MA_SUCCESS)
            {
                assert(false);
                return;
            }
        }

        ~MiniaudioDriver()
        {
            if (ma_device_is_started(&device))
                ma_device_stop(&device);

            if (ma_device_get_state(&device) != ma_device_state_uninitialized)
                ma_device_uninit(&device);

            ma_context_uninit(&context);
        }

        virtual bool configure(const DriverConfiguration& configuration) override
        {
            if (ma_device_is_started(&device))
            {
                ma_device_stop(&device);
                ma_device_uninit(&device);
            }

            ma_device_info* playback_device_infos;
            ma_device_info* capture_device_infos;
            uint32_t playback_devices_count;
            uint32_t capture_devices_count;
            ma_context_get_devices(&context
                , &playback_device_infos, &playback_devices_count
                , &capture_device_infos, &capture_devices_count);

            ma_device_id playback_id = {};
            ma_device_id capture_id = {};
            fmt::println("Available playback devices");
            for (uint32_t i = 0; i < playback_devices_count; i++)
            {
                const auto& pdi = playback_device_infos[i];
                ma_context_get_device_info(&context, ma_device_type_playback, &pdi.id, &playback_device_infos[i]);
                fmt::println("- {}", pdi.name);
                if (std::string_view(pdi.name).find("MOTU") != std::string_view::npos)
                    playback_id = pdi.id;
                for (int f = 0; f < pdi.nativeDataFormatCount; f++)
                {
                    fmt::println("    - {}Hz, {} channels"
                        , pdi.nativeDataFormats[f].sampleRate
                        , pdi.nativeDataFormats[f].channels);
                }
            }

            fmt::println("Available capture devices");
            for (uint32_t i = 0; i < capture_devices_count; i++)
            {
                const auto& cdi = capture_device_infos[i];
                ma_context_get_device_info(&context, ma_device_type_capture, &cdi.id, &capture_device_infos[i]);
                fmt::println("- {}", cdi.name);
                if (std::string_view(cdi.name).find("MOTU") != std::string_view::npos)
                    capture_id = cdi.id;
                for (int f = 0; f < cdi.nativeDataFormatCount; f++)
                {
                    fmt::println("    - {}Hz, {} channels"
                        , cdi.nativeDataFormats[f].sampleRate
                        , cdi.nativeDataFormats[f].channels);
                }
            }

            ma_device_config device_config = ma_device_config_init(ma_device_type_duplex);
            device_config.dataCallback = &MiniaudioDriver::datacallback;
            device_config.pUserData = this;
            device_config.periodSizeInFrames = configuration.buffersize;
            device_config.sampleRate = configuration.samplerate;

            // device_config.playback.channelMixMode = ma_channel_mix_mode_simple;
            device_config.playback.channels = 8;
            device_config.playback.format = ma_format_f32;
            device_config.playback.shareMode = ma_share_mode_shared;
            device_config.playback.pDeviceID = &playback_id;
            // device_config.capture.channelMixMode = ma_channel_mix_mode_simple
            device_config.capture.channels = 8;
            device_config.capture.format = ma_format_f32;
            device_config.capture.shareMode = ma_share_mode_shared;
            device_config.capture.pDeviceID = &capture_id;

            if (ma_device_init(&context, &device_config, &device) != MA_SUCCESS)
                return false;
            
            samplerate = device.sampleRate;
            buffersize = device.playback.internalPeriodSizeInFrames;
            playback.channels = device.playback.channels;
            memcpy(&playback.deviceid, &device.playback.id, sizeof(playback.deviceid));
            capture.channels = device.capture.channels;
            memcpy(&capture.deviceid, &device.capture.id, sizeof(capture.deviceid));

            return true;
        }

        virtual void start() override
        {
            if (!ma_device_is_started(&device))
                ma_device_start(&device);
        }
        
        virtual void stop() override
        {
            if (ma_device_is_started(&device))
                ma_device_stop(&device);
        }

        static void datacallback(
              ma_device* pDevice
            , void* pOutput
            , const void* pInput
            , ma_uint32 frameCount)
        {
            auto driver = (MiniaudioDriver*) pDevice->pUserData;
            driver->callback((const float*) pInput, (float*) pOutput, frameCount);
        }
    };

}