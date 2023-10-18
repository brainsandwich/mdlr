#pragma once

#include "mdlr/module.h"
#include "mdlr/util.h"

#include <array>
#include <fmt/format.h>

namespace mdlr
{
    struct Sequencer: Module
    {
        enum {
            slot_clock,
            slot_reset,
            slot_randomize
        };

        enum {
            slot_pitch,
            slot_velocity
        };

        RisingEdgeDetector clktrig;
        RisingEdgeDetector rsttrig;
        RisingEdgeDetector rndtrig;
        int index = 0;
        std::array<float, 8> pitch;
        std::array<float, 8> velocity;

        Sequencer()
        {
            ins = {
                { "clock" },
                { "reset" },
                { "randomize" },
            };
            outs = {
                { "pitch" },
                { "velocity" },
            };
        }

        virtual void process(float samplerate) override
        {
            if (rndtrig.process(ins[slot_randomize]))
                randomize();

            if (rsttrig.process(ins[slot_reset]))
                index = 0;

            if (clktrig.process(ins[slot_clock]))
                index = (index + 1) % 8;

            outs[slot_pitch] = pitch[index];
            outs[slot_velocity] = velocity[index];
        }

        virtual void randomize(int mode=0) override
        {
            for (int i = 0; i < 8; i++)
            {
                if (mode == 0 || mode == 1)
                    pitch[i] = random(30.f, 120.f);
                if (mode == 0 || mode == 2)
                    velocity[i] = random(0.f, 1.f);
            }
        }
    };

    struct MetropolisSequencer: Module
    {
        enum {
            slot_clock,
            slot_reset,
            slot_randomize
        };

        enum {
            slot_pitch,
            slot_gate,
            slot_velocity,
            slot_index,
            slot_end,

            slot_step0,
            slot_step1,
            slot_step2,
            slot_step3,
            slot_step4,
            slot_step5,
            slot_step6,
            slot_step7
        };

        static constexpr int gate_off = 0;
        static constexpr int gate_short = 1;
        static constexpr int gate_long = 2;

        RisingEdgeDetector clktrig;
        RisingEdgeDetector rsttrig;
        RisingEdgeDetector rndtrig;
        int ticks = 0;

        int index = 0;
        int repcnt = 0;
        float pitch[8] = {};
        float velocity[8] = {};
        int repeat[8] = {};
        int gate[8] = {};
        bool slide[8] = {};

        struct {
            float pitch = 0.f;
            float velocity = 0.f;
        } current;
        float slidetime = 1.f / 1000.f;
        float gatelen = 1.f;

        MetropolisSequencer()
        {
            ins = {
                { "clock" },
                { "reset" },
                { "randomize" },
            };
            outs = {
                { "pitch" },
                { "gate" },
                { "velocity" },
                { "index" },
                { "end" },

                { "step0" },
                { "step1" },
                { "step2" },
                { "step3" },
                { "step4" },
                { "step5" },
                { "step6" },
                { "step7" },
            };
        }

        virtual void randomize(int mode=0) override
        {
            for (int i = 0; i < 8; i++)
            {
                if (mode == 0 || mode == 1)
                    pitch[i] = random(30.f, 500.f);
                
                if (mode == 0 || mode == 2)
                    velocity[i] = random(0.1f, 1.f);
                
                if (mode == 0 || mode == 3)
                    repeat[i] = random(1.f, 3.f);
                
                if (mode == 0 || mode == 4)
                    gate[i] = clamp(random(0.45f, 3.f), 0.f, 2.f);
                
                if (mode == 0 || mode == 5)
                    slide[i] = random(0.f, 0.8f) > 0.5f ? true : false;
            }
        }

        virtual void process(float samplerate) override
        {
            if (rndtrig.process(ins[slot_randomize]))
                randomize();

            if (rsttrig.process(ins[slot_reset]))
            {
                index = 0;
                repcnt = 0;
            }

            outs[slot_index] = float(index);
            for (int i = 0; i < 8; i++)
                outs[slot_step0 + i] = 0.f;
            outs[slot_end] = 0.f;
            if (clktrig.process(ins[slot_clock]))
            {
                ticks = 0;
                repcnt++;
                if (repcnt >= repeat[index])
                {
                    repcnt = 0;
                    index = (index + 1) % 8;
                    outs[slot_step0 + index] = 1.f;
                    if (index == 0)
                        outs[slot_end] = 1.f;
                }
            }

            if (slide[index])
            {
                const float slidespeed = 1.f / (samplerate * slidetime);
                float diff = pitch[index] - current.pitch;
                current.pitch += clamp(diff, -slidespeed, slidespeed);
            } else {
                current.pitch = pitch[index];
            }
            current.velocity = velocity[index];

            outs[slot_pitch] = current.pitch;
            outs[slot_velocity] = current.velocity;
            if (gate[index] == gate_off)
                outs[slot_gate] = 0.f;
            else if (gate[index] == gate_short)
                outs[slot_gate] = (ticks < (gatelen * samplerate)) ? 1.f : 0.f;
            else
                outs[slot_gate] = 1.f;

            ticks++;
        }
    };
}