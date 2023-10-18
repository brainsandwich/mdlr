#include "mdlr/module.h"

#include <fmt/format.h>

namespace mdlr
{
    Slot& Module::addInput(std::string_view name, float defaultValue) { return ins.emplace_back(Slot { .name = name.data(), .signal = defaultValue }); }
    void Module::addInputs(std::string_view basename, int count, float defaultValue)
    {
        for (int i = 0; i < count; i++)
            addInput(fmt::format("{}-{}", basename, i), defaultValue);
    }
    Slot& Module::addOutput(std::string_view name, float defaultValue) { return outs.emplace_back(Slot { .name = name.data(), .signal = defaultValue }); }
    void Module::addOutputs(std::string_view basename, int count, float defaultValue)
    {
        for (int i = 0; i < count; i++)
            addOutput(fmt::format("{}-{}", basename, i), defaultValue);
    }
    Parameter& Module::addParameter(std::string_view name, Parameter::Setter&& setter, Parameter::Getter&& getter)
    {
        return parameters.emplace_back(Parameter(this, name, std::move(setter), std::move(getter)));
    }

    Parameter* Module::findParameter(std::string_view path)
    {
        auto dotpos = path.find(".");
        auto stub = path.substr(0, dotpos);
        auto ext = path.substr(dotpos + 1);
        if (dotpos == std::string::npos)
        {
            for (auto& p: parameters)
                if (p.name == stub)
                    return &p;
        } else {
            auto mod = findModule(stub);
            if (mod)
                return mod->findParameter(ext);
        }
        return nullptr;
    }

    Slot* Module::findInput(std::string_view path)
    {
        auto dotpos = path.find(".");
        auto stub = path.substr(0, dotpos);
        auto ext = path.substr(dotpos + 1);
        if (dotpos == std::string::npos)
        {
            for (auto& i: ins)
                if (i.name == stub)
                    return &i;
        } else {
            auto mod = findModule(stub);
            if (mod)
                return mod->findInput(ext);
        }
        return nullptr;
    }

    Slot* Module::findOutput(std::string_view path)
    {
        auto dotpos = path.find(".");
        auto stub = path.substr(0, dotpos);
        auto ext = path.substr(dotpos + 1);
        if (dotpos == std::string::npos)
        {
            for (auto& o: outs)
                if (o.name == stub)
                    return &o;
        } else {
            auto mod = findModule(stub);
            if (mod)
                return mod->findOutput(ext);
        }
        return nullptr;
    }

    Module* Module::findModule(std::string_view path) { return nullptr; }

    std::string Module::string() const
    {
        std::string result;
        result += fmt::format("{}:\n", name);
        
        if (!ins.empty())
        {
            result += fmt::format("- inputs:\n");
            for (const auto& in: ins)
                result += fmt::format("    - {} -> {:.2f}\n", in.name, in.name);
        }
        
        if (!outs.empty())
        {
            result += fmt::format("- outputs:\n");
            for (const auto& out: outs)
                result += fmt::format("    - {} -> {:.2f}\n", out.name, out.name);
        }

        if (!parameters.empty())
        {
            result += fmt::format("- parameters:\n");
            for (const auto& prm: parameters)
                result += fmt::format("    - {}\n", prm.name);
        }

        return result;
    }
}