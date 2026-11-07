#include "util.hpp"
#include "../dusklight/src/dusk/config.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <fmt/format.h>

#include "mods/svc/hook.h"
#include "mods/svc/message.h"

namespace uwunglish::util {
uint16_t computeId(uint8_t leftId, uint8_t rightId) {
    return static_cast<uint16_t>(leftId << 8 | rightId);
}

/*
 * Fully copied from the randomizer
 */
std::string UTF8ToCP1252(const std::string& utf8String) {
    std::string cp1252Str;
    cp1252Str.reserve(utf8String.length());

    size_t readPos = 0;
    size_t len = utf8String.length();

    while (readPos < len) {
        unsigned char c = utf8String[readPos];

        if (c < 0x80) {
            // Standard ASCII (0x00 - 0x7F)
            cp1252Str.push_back(c);
            ++readPos;
        } else if ((c & 0xE0) == 0xC0 && (readPos + 1 < len)) {
            // 2-byte UTF-8 sequence (0xC2 - 0xDF)
            unsigned char nextByte = utf8String[readPos + 1];

            // Reconstruct code point for U+0080 to U+07FF
            uint32_t codePoint = ((c & 0x1F) << 6) | (nextByte & 0x3F);

            static std::unordered_map<uint32_t, char> twoByteMap = {
                {0x0152, 0x8C}, // Œ
                {0x0153, 0x9C}, // œ
                {0x0160, 0x8A}, // Š
                {0x0161, 0x9A}, // š
                {0x0178, 0x9F}, // Ÿ
                {0x017D, 0x8E}, // Ž
                {0x017E, 0x9E}, // ž
            };

            if (twoByteMap.contains(codePoint)) {
                cp1252Str.push_back(twoByteMap.at(codePoint));
            } else if (codePoint <= 0xFF) {
                cp1252Str.push_back(static_cast<char>(codePoint));
            } else {
                throw std::runtime_error(fmt::format(
                    "Invalid character U+{:04X} when converting to CP1252 in \"{}\"", codePoint,
                    utf8String));
            }

            readPos += 2;
        } else if ((c & 0xF0) == 0xE0 && (readPos + 2 < len)) {
            // 3-byte UTF-8 sequence
            unsigned char b2 = utf8String[readPos + 1];
            unsigned char b3 = utf8String[readPos + 2];

            uint32_t codePoint = ((c & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);

            static std::unordered_map<uint32_t, char> threeByteMap = {
                {0x20AC, 0x80}, // €
                {0x201A, 0x82}, // ‚
                {0x0192, 0x83}, // ƒ
                {0x201E, 0x84}, // „
                {0x2026, 0x85}, // …
                {0x2020, 0x86}, // †
                {0x2021, 0x87}, // ‡
                {0x02C6, 0x88}, // ˆ
                {0x2030, 0x89}, // ‰
                {0x2039, 0x8B}, // ‹
                {0x2018, 0x91}, // ‘
                {0x2019, 0x92}, // ’
                {0x201C, 0x93}, // “
                {0x201D, 0x94}, // ”
                {0x2022, 0x95}, // •
                {0x2013, 0x96}, // –
                {0x2014, 0x97}, // —
                {0x02DC, 0x98}, // ˜
                {0x2122, 0x99}, // ™
                {0x203A, 0x9B}, // ›
            };

            if (threeByteMap.contains(codePoint)) {
                cp1252Str.push_back(threeByteMap.at(codePoint));
            } else {
                throw std::runtime_error(fmt::format(
                    "Invalid character U+{:04X} when converting to CP1252 in \"{}\"", codePoint,
                    utf8String));
            }
            readPos += 3;
        } else {
            // Unsupported sequence, out of CP1252 range, or malformed UTF-8
            throw std::runtime_error(
                fmt::format("Invalid bytes when converting to CP1252 with \"{}\"", utf8String));
        }
    }

    return cp1252Str;
}

/**
 *
 * Thanks to Fimmel for that last part
 *
 */

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

// check if target mod is active by querying its interal 'enabled' bool
bool is_mod_enabled(std::string_view id) {
    return mod_config_bool(id, "enabled", false);
}
}