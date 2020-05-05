#pragma once

#include "rapidjson/pointer.h"

#include "collect_paths_struct.hpp"
#include "collect_paths_json_doc.hpp"
#include "json_doc_converter.hpp"
#include "json_doc_io.hpp"
#include "value_printer.hpp"

#include "fs_ns_workaround/fs_ns_workaround.hpp"

#include <map>
#include <boost/optional.hpp>

class ConfigLoader
{
public:
    static std::string findConfigFile(const std::string& name)
    {
        using namespace STD_FILESYSTEM_NAMESPACE;
        if (exists(name))
            return current_path() / name;
        else {
            auto searchConfig = [&] (const char *env) {
                auto dir = getenv(env);
                if (dir) {
                    auto result = std::string(path(dir) / name);
                    if (exists(result))
                        return result;
                }
                return std::string();
            };
            auto result = searchConfig("HOME");
            if (result.empty())
                result = searchConfig("USERPROFILE");
            return result;
        }
    }

    template<class Overrider>
    ConfigLoader(
            std::istream& s,
            const std::string& origin,
            Overrider configOverrider)
    {
        load(s, origin);
        configOverrider(*this);
    }

    template<class Overrider>
    ConfigLoader(
            const std::string& fileName,
            Overrider configOverrider)
    {
        load(fileName);
        configOverrider(*this);
    }

    ConfigLoader(const std::string& fileName)
    {
        load(fileName);
    }

    template<class T>
    ConfigLoader& setValue(const T& value, const std::string& path, const std::string& origin)
    {
        auto val = iterate_struct::to_json(value, m_config.GetAllocator());
        rapidjson::Pointer(path.c_str()).Create(m_config).Swap(val);
        m_configParamOrigin[path] = origin;
        return *this;
    }

    template<class T>
    T value() const {
        return iterate_struct::from_json_doc<T>(m_config);
    }

    template<class T>
    T valueAt(const std::string& path, const T& defaultValue) const
    {
        auto val = rapidjson::Pointer(path.c_str()).Get(m_config);
        return val? iterate_struct::from_json<T>(*val): defaultValue;
    }

    template<class T>
    T valueAt(const std::string& path) const
    {
        auto val = rapidjson::Pointer(path.c_str()).Get(m_config);
        if (!val)
            throw std::runtime_error(std::string("Value at path '") + path + "' is not found");
        return iterate_struct::from_json<T>(*val);
    }

    ConfigLoader& setOptionalString(const boost::optional<std::string>& maybe, const std::string& path, const std::string& origin)
    {
        if (maybe)
            setValue(maybe.get().c_str(), path, origin);
        return *this;
    }

    const rapidjson::Document& config() const {
        return m_config;
    }

    const std::map<std::string, std::string>& configParamOrigin() const {
        return m_configParamOrigin;
    }

private:
    rapidjson::Document m_config;
    std::map<std::string, std::string> m_configParamOrigin;

    void load(const std::string& fileName)
    {
        std::ifstream s(fileName);
        if (!s.is_open())
            throw std::runtime_error(std::string("Failed to open input file '")+fileName+"'");
        load(s, fileName);
    }

    void load(std::istream& s, const std::string& origin)
    {
        m_config = read_json_doc(s);
        for (auto& path : iterate_struct::collect_paths(m_config, true))
            m_configParamOrigin[path] = origin;
    }
};