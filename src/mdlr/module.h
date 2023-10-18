#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <variant>
#include <span>

#define FMT_CONSTEVAL
#include <fmt/format.h>

#include "mdlr/util.h"

namespace mdlr
{
    using Signal = float;
    
    struct Module;

    struct Slot
    {
        std::string name;
        std::vector<Slot*> targets;
        Signal signal = 0.f;

        void connect(Slot& other) { targets.push_back(&other); }
        void disconnect(Slot& other) { targets.erase(std::remove(targets.begin(), targets.end(), &other), targets.end()); }
        void propagate() { for (auto& t: targets) t->signal = signal; }

        Slot& operator=(const Slot&) = default;
        Slot& operator=(const Signal& sig) { signal = sig; return *this; }
        operator Signal() const { return signal; }

        static std::unique_ptr<Slot> create(std::string_view name, Signal defaultsignal = 0.f)
        {
            return std::unique_ptr<Slot>(new Slot { .name = name.data(), .signal = defaultsignal });
        }
    };

    struct Module;

    using ParameterValue
        = std::variant<
              std::monostate
            , bool
            , int
            , float
            , std::string
            , std::span<uint8_t>
        >;

    struct Parameter
    {
        using Setter = std::function<void(Module*, ParameterValue&&)>;
        using Getter = std::function<ParameterValue(Module*)>;

        Module* parent;
        std::string name;
        Setter setter;
        Getter getter;

        Parameter() = default;
        Parameter(Module* parent, std::string_view name, Setter&& setter, Getter&& getter)
            : parent(parent)
            , name(name.data())
            , setter(setter)
            , getter(getter)
        {}

        template <typename Class, std::convertible_to<ParameterValue> MemberType>
        Parameter(Module* parent, std::string_view name, MemberType Class::*member)
            : Parameter(
                  parent
                , name
                , [&](Module* m, ParameterValue&& v) { (*(Class*) m).*member = std::get<MemberType>(v); }
                , [&](Module* m) { return ParameterValue((*(Class*) m).*member); })
        {}

        Parameter& operator=(ParameterValue&& v) { setter(parent, std::move(v)); return *this; }
        operator ParameterValue() const { return getter(parent); }
    };

    struct Module
    {
        std::string name;
        stable_vector<Slot> ins;
        stable_vector<Slot> outs;
        stable_vector<Parameter> parameters;

        virtual ~Module() = default;
        virtual void process(float samplerate) = 0;

        Slot& addInput(std::string_view name, float defaultValue = 0.f);
        void addInputs(std::string_view basename, int count, float defaultValue = 0.f);
        Slot& addOutput(std::string_view name, float defaultValue = 0.f);
        void addOutputs(std::string_view basename, int count, float defaultValue = 0.f);
        Parameter& addParameter(std::string_view name, Parameter::Setter&& setter, Parameter::Getter&& getter);

        template <typename Class, std::convertible_to<ParameterValue> MemberType>
        Parameter& addParameter(std::string_view name, MemberType Class::*member)
        {
            return parameters.emplace_back(Parameter(this, name, member));
        }

        virtual Parameter* findParameter(std::string_view path);
        virtual Slot* findInput(std::string_view path);
        virtual Slot* findOutput(std::string_view path);
        virtual Module* findModule(std::string_view path);
        
        virtual std::string string() const;
        virtual void randomize(int mode=0) {}
    };

    struct Group: Module
    {
        std::vector<std::unique_ptr<Module>> modules;

        virtual void process(float samplerate) override
        {
            for (auto& sg: ins)
                sg.propagate();

            for (auto& m: modules)
            {
                m->process(samplerate);
                for (auto& s: m->outs)
                    s.propagate();
            }

            for (auto& sg: outs)
                sg.propagate();
        }

        template <typename Mod, typename ... Args>
        Mod& create(std::string_view name, Args&& ... args)
        {
            auto& mod = modules.emplace_back(std::make_unique<Mod>(std::forward<Args>(args)...));
            mod->name = name;
            return *(Mod*) mod.get();
        }

        virtual Module* findModule(std::string_view path) override
        {
            auto dotpos = path.find(".");
            auto stub = path.substr(0, dotpos);
            auto ext = path.substr(dotpos + 1);
            for (auto& m: modules)
            {
                if (m->name == stub)
                    return dotpos == std::string_view::npos
                        ? m.get()
                        : m->findModule(ext);
            }
            return nullptr;
        }

        virtual std::string string() const override
        {
            std::string result = Module::string();
            if (!modules.empty())
            {
                result += fmt::format("- modules: {\n");
                for (const auto& m: modules)
                {
                    result += m->string();
                }
                result += "}\n";
            }
            return result;
        }

        virtual void randomize(int mode=0) override { for (auto& m: modules) m->randomize(mode); }
    };
}