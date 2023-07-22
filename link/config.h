#ifndef __LINKS_CONFIG_H__
#define __LINKS_CONFIG_H__

#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <boost/lexical_cast.hpp>
#include "yaml-cpp/yaml.h"
#include "log.h"

namespace links {

class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "") 
        : m_name(name) 
        , m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}

    const std::string getName() { return m_name; }
    const std::string getDescription() { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
private:
    std::string m_name;
    std::string m_description;
};

template<class T>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        : ConfigVarBase(name, description)
        , m_val(default_value) {
    }

    std::string toString() override {
        try {
            return boost::lexical_cast<std::string>(m_val);
        } catch (std::exception& e) {
            LINK_LOG_ERROR(LINK_GET_ROOT()) << "ConfigVar::toString exception" 
                << e.what() << " convert: " << typeid(m_val).name() << "to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try {
            m_val =  boost::lexical_cast<T>(val);
        } catch (std::exception& e) {
            LINK_LOG_ERROR(LINK_GET_ROOT()) << "ConfigVar::toString exception" 
                << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() { return m_val; }
    void setValue(const T val) { m_val = val; }

private:
    T m_val;
};

class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, 
            const T& default_value, const std::string& description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            LINK_LOG_INFO(LINK_GET_ROOT()) << "Lookup name=" << name << " exists";
            return tmp;
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") 
                != std::string::npos) {
            LINK_LOG_ERROR(LINK_GET_ROOT()) << "Lookup name invalid:" << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        s_datas[name] = v;
        return v;
    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto it = s_datas.find(name);
        if (it == s_datas.end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const std::string& name);
private:
    static ConfigVarMap s_datas;
};
}

#endif
