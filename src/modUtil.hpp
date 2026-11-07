#pragma once
#include "../dusklight/src/dusk/config.hpp"


/**
 *
 * Thanks to Fimmel for this part
 *
 */
namespace uwunglish::modUtil {
namespace {
using GetConfigVarFn = dusk::config::ConfigVarBase* (*)(std::string_view);

// converts id "com.ditrey.linkle" + "enabled" -> "mod.com_ditrey_linkle.enabled" to check the config key later
std::string mod_cvar_name(std::string_view id, std::string_view var) {
    std::string name = "mod.";
    for (const char c : id) {
        if (c == '.') {
            name.push_back('_');
        } else if (c == '_') {
            name.append("__");
        } else {
            name.push_back(c);
        }
    }
    name.push_back('.');
    name.append(var);
    return name;
}


// get function ptr with HookService (result will be cached)
GetConfigVarFn host_get_config_var() {
    static GetConfigVarFn fn = nullptr;
    static bool resolved = false;
    if (!resolved) {
        resolved = true;
        void* addr = nullptr;
        if (svc_hook != nullptr && mod_ctx != nullptr &&
            svc_hook->resolve(mod_ctx, "dusk::config::GetConfigVar", &addr, nullptr) == MOD_OK) {
            fn = reinterpret_cast<GetConfigVarFn>(addr);
        }
    }
    return fn;
}

// reads a bool config var from host, returns fallback if missing
bool host_config_bool(std::string_view name, bool fallback) {
    const auto getVar = host_get_config_var();
    if (getVar == nullptr)
        return fallback;
    const auto* var = getVar(name);
    return var != nullptr ?
               static_cast<const dusk::config::ConfigVar<bool>*>(var)->getValue() :
               fallback;
}

// builds the full cvar name
bool mod_config_bool(std::string_view modId, std::string_view name, bool fallback) {
    return host_config_bool(mod_cvar_name(modId, name), fallback);
}
}

/**
 * Uses the hook service in a hacky way to get the method that retrieves a config var and
 * applies it to the provided mod id.
 *
 * @param id a mod id
 * @return true if the mod is active, else false
 */
inline bool is_mod_enabled(std::string_view id) {
    return mod_config_bool(id, "enabled", false);
}
}