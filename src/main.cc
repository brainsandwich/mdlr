#include "mdlr/engine.h"
#include "mdlr/modules/core.h"
#include "mdlr/modules/enveloppe.h"
#include "mdlr/modules/midi.h"
#include "mdlr/modules/sequencer.h"

#include <iostream>
#include <fmt/format.h>

namespace mdlr
{
    struct DeeFam: Module
    {
        enum {
            slot_clock
        };
        enum {
            slot_output
        };

        RisingEdgeDetector clocktrig;
        Oscillator clock;
        Sequencer sequencer;
        Oscillator osc1;
        EnveloppeADSR enveloppe;
        EnveloppeADSR enveloppe_amp;
        Attenuator amp;

        float osc_eg_amount = 500.f;
        float osc_eg_decay = 0.01f;
        float amp_decay = 0.2f;

        DeeFam()
        {
            ins = {
                { "clock" }
            };
            outs = {
                { "output" }
            };

            ins[slot_clock].connect(sequencer.ins[Sequencer::slot_clock]);
            sequencer.randomize();

            enveloppe.ins[EnveloppeADSR::slot_a] = 0.f;
            enveloppe.ins[EnveloppeADSR::slot_d] = 0.f;
            enveloppe.ins[EnveloppeADSR::slot_s] = 1.f;
            enveloppe.ins[EnveloppeADSR::slot_r] = osc_eg_decay;

            enveloppe_amp.ins[EnveloppeADSR::slot_a] = 0.f;
            enveloppe_amp.ins[EnveloppeADSR::slot_d] = 0.f;
            enveloppe_amp.ins[EnveloppeADSR::slot_s] = 1.f;
            enveloppe_amp.ins[EnveloppeADSR::slot_r] = amp_decay;
        }

        virtual void process(float samplerate) override
        {
            ins[slot_clock].propagate();
            float trig = clocktrig.process(ins[slot_clock]) ? 1.f : 0.f;

            enveloppe_amp.ins[EnveloppeADSR::slot_gate] = trig;
            enveloppe_amp.process(samplerate);
            float env_amp = enveloppe_amp.outs[EnveloppeADSR::slot_output];

            enveloppe.ins[EnveloppeADSR::slot_gate] = trig;
            enveloppe.process(samplerate);
            float env_eg = enveloppe.outs[EnveloppeADSR::slot_output];

            sequencer.process(samplerate);
            osc1.ins[Oscillator::slot_frequency]
                = sequencer.outs[Sequencer::slot_pitch]
                + (env_eg*env_eg * osc_eg_amount);
            osc1.process(samplerate);

            // float output = osc1.outs[Oscillator::slot_output];
            float output
                = osc1.outs[Oscillator::slot_output]
                * sequencer.outs[Sequencer::slot_velocity];
                
            amp.ins[Attenuator::slot_gain] = env_amp;
            amp.ins[Attenuator::slot_input] = output;
            amp.process(samplerate);
            output = amp.outs[Attenuator::slot_output];
            outs[slot_output] = clamp(output, -1.f, 1.f);
        }

        virtual void randomize(int mode=0) override
        {
            sequencer.randomize(mode);
        }
    };
}

namespace mdlr
{
    struct Delay: Module
    {
        std::vector<float> buffer;
        int buffersize = 1024*128;
        int readpos = 0;
        int writepos = 0;
        struct
        {
            Slot* input;
            Slot* time;
            Slot* output;
        } slots;
        float time = 0.f;
        float value = 0.f;

        Delay()
        {
            buffer.resize(buffersize, 0.f);
            slots.input = &addInput("input");
            slots.time = &addInput("time");
            slots.output = &addOutput("output");
            addParameter("buffersize", &Delay::buffersize);
        }

        virtual void process(float samplerate) override
        {
            if (buffersize != buffer.size())
                buffer.resize(buffersize);

            float time_current = (*slots.time) * buffersize;
            time = time * 0.99 + time_current * 0.01;

            buffer[readpos] = *slots.input;
            readpos = (readpos + 1) % buffer.size();
            writepos = (readpos - int(time)) % buffer.size();
            value = value * 0.9 + buffer[writepos] * 0.1;
            *slots.output = clamp(value, -1., 1);
        }
    };

    template <int N>
    struct Adder: Module
    {
        Adder()
        {
            addInputs("input", N);
            addOutput("output");
        }
    };

    template <int N>
    struct Splitter: Module
    {
        Splitter()
        {
            addInput("input");
            addOutputs("output", N);
        }
    };

    template <int N>
    struct Mult: Module
    {
        
    };

    struct KellettFilter: Module
    {
        float a = 0.f;
        float b = 0.f;

        struct {
            Slot* input;
            Slot* cutoff;
            Slot* resonance;
            
            Slot* lowpass;
            Slot* bandpass;
            Slot* highpass;
        } slots;

        KellettFilter()
        {
            slots.input = &addInput("input");
            slots.cutoff = &addInput("cutoff");
            slots.resonance = &addInput("resonance");

            slots.lowpass = &addOutput("lowpass");
            slots.bandpass = &addOutput("bandpass");
            slots.highpass = &addOutput("highpass");
        }

        virtual void process(float samplerate) override
        {
            float input = *slots.input;
            float resonance = *slots.resonance;
            float cutoff = *slots.cutoff;
            float feedback = resonance + resonance / (1.0 - cutoff);
            
            a += cutoff * (input - a);
            b += cutoff * (a - b);
            
            *slots.lowpass = b;
            *slots.bandpass = input - a;
            *slots.highpass = a - b;
        }
    };
}

mdlr::Group& acidSynth(mdlr::Group& parent, std::string_view name = "acid")
{
    using namespace mdlr;
    auto& acid = parent.create<Group>(name);

    auto& seq = acid.create<MetropolisSequencer>("seq");
    auto& osc = acid.create<Oscillator>("osc");
    auto& env = acid.create<EnveloppeADSR>("env");
    auto& amp = acid.create<Attenuator>("amp");

    auto& in_clock = acid.addInput("clock");
    auto& in_reset = acid.addInput("reset");
    auto& in_rnd = acid.addInput("randomize");
    auto& out_output = acid.addOutput("output");

    in_clock.connect(*seq.findInput("clock"));
    in_reset.connect(*seq.findInput("reset"));
    in_rnd.connect(*seq.findInput("randomize"));

    seq.randomize();
    seq.gatelen = 0.02f;
    seq.slidetime = 0.5f / 1000.f;

    seq.findOutput("pitch")->connect(*osc.findInput("frequency"));
    seq.findOutput("gate")->connect(*env.findInput("gate"));
    // seq.findOutput("gate")->connect(*amp.findInput("gain"));
    // osc.findOutput("output")->connect(*amp.findInput("input"));
    env.findOutput("output")->connect(*amp.findInput("gain"));
    amp.findOutput("output")->connect(out_output);

    return acid;
}

int main(int argc, char** argv)
{
    using namespace mdlr;

    Engine engine;
    engine.init({ .samplerate = 48000, .buffersize = 128, .capture = { .selector = { .any = true } } });
    
    // Create modules
    auto& system = engine.system;
    // auto& clk = system.create<Oscillator>("clk");
    auto& midi = system.create<MidiIn>("midi");
    auto& midicc = system.create<MidiCC128>("midicc");
    auto& midigate = system.create<MidiGate128>("midigate");
    auto& midiclkdiv = system.create<ClockDivider>("mcd");
    midi.outs[MidiIn::slot_clock].connect(midiclkdiv.ins[ClockDivider::slot_clock]);
    // clk.ins[Oscillator::slot_frequency] = 4.f * 140.f / 60.f;

    // DeeFam voice
    auto& dfa = system.create<DeeFam>("dfa");
    midiclkdiv.outs[ClockDivider::slot_1_6].connect(dfa.ins[DeeFam::slot_clock]);
    midi.outs[MidiIn::slot_start].connect(dfa.sequencer.ins[Sequencer::slot_reset]);
    midigate.outs[44].connect(dfa.sequencer.ins[Sequencer::slot_randomize]);
    // clk.outs[Oscillator::slot_output].connect(dfa.ins[DeeFam::slot_clock]);
    dfa.sequencer.randomize();
    dfa.osc_eg_amount = 400.f;
    dfa.sequencer.pitch = {
        60.f, 60.f, 80.f, 60.f,
        60.f, 60.f, 60.f, 60.f,
    };
    dfa.sequencer.velocity = {
        1.f, 0.f, 0.2f, 0.f,
        1.f, 0.f, 0.f, 0.f,
    };

    // Acid voice
    auto& acid = acidSynth(system);
    midiclkdiv.outs[ClockDivider::slot_1_6].connect(*acid.findInput("clock"));
    midi.outs[MidiIn::slot_start].connect(*acid.findInput("reset"));
    midigate.outs[36].connect(*acid.findInput("randomize"));

    // DFAM voice
    dfa.outs[DeeFam::slot_output].connect(system.outs[0]);
    // dfa.outs[DeeFam::slot_output].connect(system.outs[1]);
    // dfa.outs[DeeFam::slot_output].connect(system.outs[2]);
    // dfa.outs[DeeFam::slot_output].connect(system.outs[3]);
    // dfa.outs[DeeFam::slot_output].connect(system.outs[4]);
    // dfa.outs[DeeFam::slot_output].connect(system.outs[5]);
    // dfa.outs[DeeFam::slot_output].connect(system.outs[6]);
    // dfa.outs[DeeFam::slot_output].connect(system.outs[7]);
    acid.findOutput("output")->connect(system.outs[1]);

    // FX send / return
    auto& fx1 = system.create<Delay>("fx1");
    midicc.outs[10].connect(*fx1.findInput("time"));
    // dfa.outs[DeeFam::slot_output].connect(*fx1.findInput("input"));
    system.ins[2].connect(*fx1.findInput("input"));
    fx1.findOutput("output")->connect(system.outs[2]);

    engine.start();
    while (true)
    {
        std::cout << "> ";
        std::string str;
        std::getline(std::cin, str);
        try {
            if (str == "quit") break;

            auto parenpos = str.find("()");
            if (parenpos != std::string::npos)
            {
                auto dotpos = str.find_last_of(".");
                if (dotpos != std::string::npos)
                {
                    auto path = str.substr(0, dotpos);
                    auto func = str.substr(dotpos+1, parenpos-1-dotpos);
                    auto mod = system.findModule(path);

                    if (func == "randomize" && mod)
                    {
                        fmt::println("@ Calling {}() on module {}", func, path);
                        mod->randomize();
                    }
                }
            }

            // Set a slot input or parameter value
            auto eqpos = str.find("=");
            if (eqpos != std::string::npos)
            {
                auto path = str.substr(0, eqpos);
                auto value = str.substr(eqpos+1);

                auto input = system.findInput(path);
                if (input)
                {
                    input->signal = std::strtof(value.data(), nullptr);
                    fmt::println("@ Setting signal {} to {:.2f}", path, input->signal);
                }

                auto param = system.findParameter(path);
                if (param)
                {
                    bool vset = false;
                    if (!vset)
                    {
                        try {
                            float v = std::strtof(value.data(), nullptr);
                            (*param) = v;
                            fmt::println("@ Setting parameter {} to {:.2f}", path, v);
                            vset = true;
                        } catch (...) {}
                    }

                    if (!vset)
                    {
                        try {
                            int v = std::strtol(value.data(), nullptr, 10);
                            (*param) = v;
                            fmt::println("@ Setting parameter {} to {}", path, v);
                            vset = true;
                        } catch (...) {}
                    }
                    
                    if (!vset)
                    {
                        vset = true;
                        if (value == "true") { (*param) = true; fmt::println("@ Setting parameter {} to true", path); }
                        else if (value == "false") { (*param) = false; fmt::println("@ Setting parameter {} to false", path); }
                        else { vset = false; }
                    }

                    if (!vset)
                    {
                        (*param) = value;
                        fmt::println("@ Setting parameter {} to {}", path, value);
                    }
                }
            }

            // if (str.find("dfam.") != std::string::npos)
            // {
            //     auto sub = str.substr(5);
            //     // if (sub.find("vol=") != std::string::npos) mix.ins[Mixer8::slot_volume0] = std::strtof(sub.substr(4).data(), nullptr);
            //     if (sub.find("eg=") != std::string::npos) dfa.osc_eg_amount = std::strtof(sub.substr(3).data(), nullptr);
            //     if (sub.find("decay=") != std::string::npos) dfa.enveloppe_amp.ins[EnveloppeADSR::slot_r] = std::strtof(sub.substr(6).data(), nullptr);
            //     if (sub.find("rnd.pitch") != std::string::npos) dfa.sequencer.randomize_pitch();
            //     if (sub.find("rnd.velo") != std::string::npos) dfa.sequencer.randomize_velocity();
            //     if (sub.find("rnd.all") != std::string::npos) dfa.sequencer.randomize();
            // }

            // if (str.find("acid.") != std::string::npos)
            // {
            //     auto sub = str.substr(5);
            //     // if (sub.find("vol=") != std::string::npos) mix.ins[Mixer8::slot_volume1] = std::strtof(sub.substr(4).data(), nullptr);
            //     if (sub.find("decay=") != std::string::npos) acid_env.ins[EnveloppeADSR::slot_d] = std::strtof(sub.substr(6).data(), nullptr);
            //     if (sub.find("rnd.pitch") != std::string::npos) acid_seq.randomize_pitch();
            //     if (sub.find("rnd.all") != std::string::npos) acid_seq.randomize();
            // }

            // if (str.find("mixer.") != std::string::npos)
            // {
            //     auto sub = str.substr(6);
            //     if (sub.find("drive=") != std::string::npos) sig.ins[Sigmoid::slot_k] = std::strtof(sub.substr(6).data(), nullptr);
            // }
        } catch (...)
        {}
    }
    engine.stop();

    return 0;
}