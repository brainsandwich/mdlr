#pragma once

#include "mdlr/driver.h"
#include "mdlr/module.h"

#include <memory>

namespace mdlr
{
    struct Engine
    {
        std::unique_ptr<Driver> driver;
        Group system;
        struct {
            float target = 0.f;
            float current = 0.f;
        } volume;

        bool init(DriverConfiguration configuration = {})
        {
            driver = Driver::create(DriverBackend::Miniaudio);
            driver->callback = [&](const float* ins, float* outs, int frames) { this->callback(ins, outs, frames); };
            if (!driver->configure(configuration))
                return false;

            system.ins.resize(driver->capture.channels);
            system.outs.resize(driver->playback.channels);
            return true;
        }

        void start()
        {
            driver->start();
            volume.target = 1.f;
            while (std::abs(volume.target - volume.current) > 0.01f); 
        }
        void stop()
        {
            volume.target = 0.f;
            while (std::abs(volume.target - volume.current) > 0.01f);
            driver->stop();
        }
        void update() {}

        void callback(const float* ins, float* outs, int frames)
        {
            if (outs)
                memset(outs, 0, driver->playback.channels * frames * sizeof(float));

            for (int f = 0; f < frames; f++)
            {
                if (ins)
                {
                    for (int c = 0; c < driver->capture.channels; c++)
                    {
                        system.ins[c] = ins[f*driver->capture.channels + c];
                        system.ins[c].propagate();
                    }
                }
                
                system.process(driver->samplerate);

                if (outs)
                {
                    for (int c = 0; c < driver->playback.channels; c++)
                        outs[f*driver->playback.channels + c] = volume.current * system.outs[c];
                }

                const float lerpfac = 70.f / driver->samplerate;
                volume.current = volume.current * (1.f - lerpfac) + volume.target * lerpfac;
            }
        }
    };
}