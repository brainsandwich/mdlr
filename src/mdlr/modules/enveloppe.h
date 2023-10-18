#pragma once

#include "mdlr/module.h"

namespace mdlr
{
    struct EnveloppeADSR: Module
    {
        enum {
            slot_gate,
            slot_a,
            slot_d,
            slot_s,
            slot_r,
        };
        enum {
            slot_output
        };

        enum class step
        {
            attack,
            decay,
            sustain,
            release
        };

        float a, d, s, r;
        float value = 0.f;
        step step = step::attack;

        EnveloppeADSR()
        {
            ins = {
                { "gate" },
                { "a" },
                { "d" },
                { "s" },
                { "r" },
            };
            outs = {
                { "output" }
            };
        }

        float process(bool gate, float samplerate)
        {
            switch (step)
            {
                case step::attack:
                    {
                        if (gate)
                            value += 1.f / (a * samplerate);
                        else
                        {
                            value -= 1.f / (r * samplerate);
                            step = step::release;
                        }

                        if (value >= 1.f)
                            step = step::decay;
                    }
                    break;

                case step::decay:
                    {
                        if (gate)
                        {
                            value -= 1.f / (d * samplerate);
                            if (value <= s)
                            {
                                step = step::sustain;
                                value = s;
                            }
                        } else
                        {
                            value -= 1.f / (r * samplerate);
                            step = step::release;
                        }

                        if (value <= s && gate)
                            step = step::sustain;
                    }
                    break;

                case step::sustain:
                    {
                        if (!gate)
                        {
                            value -= 1.f / (r * samplerate);
                            step = step::release;
                        }
                    }
                    break;

                case step::release:
                    {
                        if (gate)
                        {
                            step = step::attack;
                            value += 1.f / (a * samplerate);
                        } else
                            value -= 1.f / (r * samplerate);
                    }
                    break;
            }

            value = clamp(value, 0.f, 1.f);
            return value;
        }

        virtual void process(float samplerate) override
        {
            a = clamp(ins[slot_a], 1.e-5f, 128.f);
            d = clamp(ins[slot_d], 1.e-5f, 128.f);
            r = clamp(ins[slot_r], 1.e-5f, 128.f);
            s = clamp(ins[slot_s], 0.f, 1.f);
            outs[slot_output] = process(ins[slot_gate] > 0.5f, samplerate);
        }
    };
}