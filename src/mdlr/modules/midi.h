#pragma once

#include "mdlr/module.h"

#include <libremidi/libremidi.hpp>
#include <fmt/format.h>

namespace mdlr
{
    struct MidiCC128: Module
    {
        libremidi::midi_in midi;
        uint16_t channel_mask = 0xFFFF;

        MidiCC128()
        {
            outs.resize(128, {});
            for (int i = 0; i < 128; i++)
                outs[i].name = fmt::format("cc.{}", i);

            midi.set_error_callback([](libremidi::midi_error type, std::string_view errorText) { fmt::println("Midi error: {}", errorText); });
            midi.set_callback([&](const libremidi::message& message) { onMidiMessage(message); });
            midi.ignore_types(true, true, true);
            
            int bspport = 0;
            for (int p = 0; p < midi.get_port_count(); p++)
            {
                std::string pname = midi.get_port_name(p);
                if (pname == "Arturia BeatStep Pro Arturia BeatStepPro")
                    bspport = p;
            }
            midi.open_port(bspport);
        }

        virtual void process(float samplerate) override {}

        void onMidiMessage(const libremidi::message& message)
        {
            if (message.get_message_type() != libremidi::message_type::CONTROL_CHANGE)
                return;

            int channel = message.get_channel();
            uint8_t control = message.bytes[1];
            uint8_t value = message.bytes[2];
            fmt::println("CC#{} => {}", control, value);
            outs[control] = float(value) / 127.f;
        }
    };

    struct MidiGate128: Module
    {
        libremidi::midi_in midi;
        int channel = -1;

        MidiGate128()
        {
            outs.resize(128, {});
            for (int i = 0; i < 128; i++)
                outs[i].name = fmt::format("cc.{}", i);

            midi.set_error_callback([](libremidi::midi_error type, std::string_view errorText) { fmt::println("Midi error: {}", errorText); });
            midi.set_callback([&](const libremidi::message& message) { onMidiMessage(message); });
            midi.ignore_types(true, true, true);

            int bspport = 0;
            for (int p = 0; p < midi.get_port_count(); p++)
            {
                std::string pname = midi.get_port_name(p);
                if (pname == "Arturia BeatStep Pro Arturia BeatStepPro")
                    bspport = p;
            }
            midi.open_port(bspport);
        }

        virtual void process(float samplerate) override {}

        void onMidiMessage(const libremidi::message& message)
        {
            if (!message.is_note_on_or_off())
                return;

            if (channel >= 0 && message.get_channel() != channel)
                return;

            bool on = message.get_message_type() == libremidi::message_type::NOTE_ON;
            uint8_t note = message.bytes[1];
            fmt::println("Pad#{} => {}", note, on ? "ON" : "OFF");
            outs[note] = on ? 1.f : 0.f;
        }
    };

    struct MidiIn: Module
    {
        enum {
            slot_pitch,
            slot_gate,
            slot_velocity,
            slot_aftertouch,
            slot_pitchbend,
            slot_modulation,

            slot_clock,
            slot_start,
            slot_cont,
            slot_stop,
        };

        libremidi::midi_in midi;
        uint16_t channel_mask = 0xFFFF;
        uint8_t lastnote = 0;

        struct {
            bool clock = false;
            bool start = false;
            bool cont = false;
            bool stop = false;
        } transport;

        MidiIn()
        {
            outs = {
                { "pitch" },
                { "gate" },
                { "velocity" },
                { "aftertouch" },
                { "pitchbend" },
                { "modulation" },
                
                { "clock" },
                { "start" },
                { "continue" },
                { "stop" },
            };

            int pcount = midi.get_port_count();
            fmt::println("{}: Available MidiIn ports ({}):"
                , libremidi::get_api_display_name(midi.get_current_api())
                , pcount);

            int bspport = 0;
            for (int p = 0; p < pcount; p++)
            {
                std::string pname = midi.get_port_name(p);
                fmt::println("   - {}", pname);
                if (pname == "Arturia BeatStep Pro Arturia BeatStepPro")
                    bspport = p;
            }

            if (pcount == 0)
                return;

            midi.set_error_callback([](libremidi::midi_error type, std::string_view errorText) { fmt::println("Midi error: {}", errorText); });
            midi.set_callback([&](const libremidi::message& message) { onMidiMessage(message); });
            midi.ignore_types(true, false, true);
            midi.open_port(bspport);
        }

        virtual void process(float samplerate) override
        {
            #define handle_transport(type)      \
                outs[slot_##type] = 0.f;        \
                if (transport.type)             \
                {                               \
                    outs[slot_##type] = 1.f;    \
                    transport.type = false;     \
                }

            handle_transport(clock);
            handle_transport(start);
            handle_transport(cont);
            handle_transport(stop);

            #undef handle_transport
        }

        static float midiToHerz(char note, float root = 440.f)
        {
            return root * std::pow(2.f, float(note - 69) / 12.f);
        }

        void onMidiMessage(const libremidi::message& message)
        {
            auto msgtype = message.get_message_type();
            auto channel = message.get_channel();

            // if ((channel_mask & (0b1 << channel)) != 0)
            // {
                switch (msgtype)
                {
                    case libremidi::message_type::NOTE_OFF:
                        outs[slot_gate] = lastnote == message[1] ? 0.f : outs[slot_gate];
                        fmt::println("channel {} --> note off ({}, {})", channel, message[1], message[2]);
                        break;
                    
                    case libremidi::message_type::NOTE_ON:
                        lastnote = message[1];
                        outs[slot_pitch] = midiToHerz(message[1]);
                        outs[slot_velocity] = float(message[2]) / 127.f;
                        outs[slot_gate] = message[2] == 0 ? 1.f : 0.f;
                        fmt::println("channel {} --> note on ({}, {})", channel, message[1], message[2]);
                        break;

                    case libremidi::message_type::POLY_PRESSURE: break;
                    case libremidi::message_type::CONTROL_CHANGE: break;
                    case libremidi::message_type::PROGRAM_CHANGE: break;
                    case libremidi::message_type::AFTERTOUCH: break;
                    case libremidi::message_type::PITCH_BEND: break;
                    default:
                        break;
                }
            // }

            switch (msgtype)
            {
                case libremidi::message_type::TIME_CLOCK:
                    transport.clock = true;
                    break;
                
                case libremidi::message_type::START:
                    fmt::println("START");
                    transport.start = true;
                    break;
                
                case libremidi::message_type::CONTINUE:
                    fmt::println("CONTINUE");
                    transport.cont = true;
                    break;
                
                case libremidi::message_type::STOP:
                    fmt::println("STOP");
                    transport.stop = true;
                    break;

                case libremidi::message_type::ACTIVE_SENSING: break;
                case libremidi::message_type::SYSTEM_RESET: break;

                default:
                    break;
            }
        }
    };
}