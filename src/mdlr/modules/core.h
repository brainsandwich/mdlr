#pragma once

#include "mdlr/module.h"
#include "mdlr/util.h"

#include <cmath>

namespace mdlr
{
    struct Attenuator: Module
    {
        enum {
            slot_input,
            slot_gain,
            slot_offset,
        };

        enum {
            slot_output,
        };

        float gain = 1.f;
        float offset = 0.f;

        Attenuator()
        {
            ins = {
                { "input" },
                { "gain", .signal = 0.5f },
                { "offset", .signal = 0.f }
            };
            outs = {
                { "output" }
            };
        }

        virtual void process(float samplerate) override
        {
            const float lerpfac = 1000.f / samplerate;
            gain = (1. - lerpfac) * gain + lerpfac * ins[slot_gain];
            offset = (1. - lerpfac) * offset + lerpfac * ins[slot_offset];

            outs[slot_output] = ins[slot_input] * gain + offset;
        }
    };

    struct Oscillator: Module
    {
        enum {
            slot_frequency,
        };

        enum {
            slot_output,
        };

        float phase = 0.f;

        Oscillator()
        {
            ins = {
                { "frequency", .signal = 120.f }
            };
            outs = {
                { "output" }
            };
        }

        virtual void process(float samplerate) override
        {
            outs[slot_output] = std::sin(phase);
            phase = fmod(phase + 2.f * M_PI * ins[slot_frequency] / samplerate, 2.f * M_PI);
        }
    };

    struct Sigmoid: Module
    {
        enum {
            slot_input,
            slot_k,
        };

        enum {
            slot_output,
        };

        Sigmoid()
        {
            ins = {
                { "input" },
                { "k", .signal = 3.f },
            };
            outs = {
                { "output" }
            };
        }

        virtual void process(float samplerate) override
        {
            float x = ins[slot_input];
            float k = ins[slot_k];
            outs[slot_output] = clamp(1.f / (1.f + exp(-k*x)), -1.f, 1.f);
        }
    };

    struct ClockDivider: Module
    {
        enum {
            slot_clock
        };
        enum {
            slot_1_2,
            slot_1_3,
            slot_1_4,
            slot_1_6,
            slot_1_8,
            slot_1_12,
            slot_1_16,
        };

        int counts[7] = {};
        int divs[7] = { 2, 3, 4, 6, 8, 12, 16 };
        RisingEdgeDetector clktrig;

        ClockDivider()
        {
            ins = {
                { "clock" }
            };
            outs = {
                { "1/2" },
                { "1/3" },
                { "1/4" },
                { "1/6" },
                { "1/8" },
                { "1/12" },
                { "1/16" },
            };

            for (int i = slot_1_2; i <= slot_1_16; i++)
                counts[i] = 123;
        }

        virtual void process(float samplerate)
        {
            for (int i = slot_1_2; i <= slot_1_16; i++)
                outs[i] = 0.f;

            if (clktrig.process(ins[slot_clock]))
            {
                for (int i = slot_1_2; i <= slot_1_16; i++)
                {
                    if (counts[i] >= divs[i])
                    {
                        counts[i] = 0;
                        outs[i] = 1.f;    
                    }
                    counts[i]++;
                }
            }
        }
    };

    struct Mixer8: Module
    {
        enum {
            slot_in0,
            slot_in1,
            slot_in2,
            slot_in3,
            slot_in4,
            slot_in5,
            slot_in6,
            slot_in7,

            slot_volume0,
            slot_volume1,
            slot_volume2,
            slot_volume3,
            slot_volume4,
            slot_volume5,
            slot_volume6,
            slot_volume7,
        };

        enum {
            slot_output,
        };

        Mixer8() {
            ins = {
                { "in0" },
                { "in1" },
                { "in2" },
                { "in3" },
                { "in4" },
                { "in5" },
                { "in6" },
                { "in7" },

                { "volume0" },
                { "volume1" },
                { "volume2" },
                { "volume3" },
                { "volume4" },
                { "volume5" },
                { "volume6" },
                { "volume7" },
            };
            outs = {
                { "output" }
            };
        }

        virtual void process(float) override
        {
            float sum = 0.f;
            for (int i = 0; i < 8; i++)
                sum += ins[slot_in0 + i] * ins[slot_volume0 + i];

            outs[slot_output] = clamp(sum, -1.f, 1.f);
        }
    };
}