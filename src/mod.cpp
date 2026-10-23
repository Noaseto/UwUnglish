#include <charconv>

#include <nlohmann/json.hpp>
#include <fmt/format.h>

#include "mods/service.hpp"
#include "mods/svc/log.h"
#include "mods/svc/flow.hpp"
#include "mods/svc/config.h"
#include "mods/svc/message.h"
#include "mods/svc/resource.h"
#include "mods/svc/ui.h"
#include "mods/svc/hook.hpp"

#include <cstdio>
#include <span>
#include <string>
#include <utility>
#include <vector>

DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(MessageService, svc_message);
IMPORT_SERVICE(ResourceService, svc_resource);

using json = nlohmann::json;

namespace {
std::vector<mods::flow::MessageOverride> overridenMessage;
uint16_t kGroupCount = 9;

uint16_t computeId(int leftId, int rightId) {
    uint16_t id = leftId * 256 + rightId;
    return id;
}


/*
 * Fully copied from the randomizer
 */
std::string UTF8ToCP1252(const std::string& utf8Str) {
    std::string cp1252Str;
    cp1252Str.reserve(utf8Str.length());

    size_t readPos = 0;
    size_t len = utf8Str.length();

    while (readPos < len) {
        unsigned char c = utf8Str[readPos];

        if (c < 0x80) {
            // Standard ASCII (0x00 - 0x7F)
            cp1252Str.push_back(c);
            ++readPos;
        } else if ((c & 0xE0) == 0xC0 && (readPos + 1 < len)) {
            // 2-byte UTF-8 sequence (0xC2 - 0xDF)
            unsigned char nextByte = utf8Str[readPos + 1];

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
                    utf8Str));
            }

            readPos += 2;
        } else if ((c & 0xF0) == 0xE0 && (readPos + 2 < len)) {
            // 3-byte UTF-8 sequence
            unsigned char b2 = utf8Str[readPos + 1];
            unsigned char b3 = utf8Str[readPos + 2];

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
                    utf8Str));
            }
            readPos += 3;
        } else {
            // Unsupported sequence, out of CP1252 range, or malformed UTF-8
            throw std::runtime_error(
                fmt::format("Invalid bytes when converting to CP1252 with \"{}\"", utf8Str));
        }
    }

    return cp1252Str;
}

// maybe in some world where I'm not exhausted I split this function into several parts
// like, read json, compute ID, extract text, then the call to the flow
void overrideMessages(const MessageLanguage language) {
    int overridenMessages = 0;
    std::string languageFolder = "english/";
    for (uint16_t group = 0; group < kGroupCount; ++group) {
        std::string filePath = languageFolder +
                               "zel_0" +
                               std::to_string(group) +
                               ".bmg.json";
        if (svc_resource->file_exists(mod_ctx, filePath.c_str())) {
            ResourceBuffer buffer = RESOURCE_BUFFER_INIT;
            svc_resource->load(mod_ctx, filePath.c_str(), &buffer);
            const char* start = static_cast<const char*>(buffer.data);
            const char* end = start + buffer.size;
            json elements = json::parse(start, end, nullptr, false);
            svc_resource->free(mod_ctx, &buffer);

            for (const auto& element : elements) {
                // id of the message to replace
                std::string fullId = element.at("ID").get<std::string>();
                int leftId, rightId;
                std::sscanf(fullId.c_str(), "%d , %d", &leftId, &rightId);
                uint16_t id = computeId(leftId, rightId);

                // now we need the bytes to write
                std::vector<uint8_t> text;
                auto lines = element.at("text");

                bool isFirstLine = true;
                for (const auto& jsonLine : lines) {
                    if (!isFirstLine) {
                        text.push_back('\n');
                    }
                    isFirstLine = false;

                    std::string utf8Line = jsonLine.get<std::string>();
                    std::string line = UTF8ToCP1252(utf8Line);

                    for (size_t i = 0; i < line.size(); i++) {
                        char currentChar = line[i];
                        if (currentChar == '{') {
                            // here we have to convert stuff between curly brackets
                            // {1a05000000} is the code for player name
                            // they do not have a fixed size :< but its even
                            // and then we must find the closing one
                            // There is no dialog in game with curly brackets, so it is safe to
                            // assume that there will be a closing one for every open one
                            size_t closingCurlyBracket = line.find('}', i);
                            std::string hexTag = line.substr(i + 1,
                                closingCurlyBracket - i - 1);
                            for (size_t j = 0; j < hexTag.size(); j += 2) {
                                uint8_t byte;
                                std::from_chars(hexTag.data() + j, hexTag.data() + (j + 2),
                                    byte, 16);
                                text.push_back(byte);
                            }

                            // move the cursor to the end of the tag
                            i = closingCurlyBracket;
                        } else {
                            text.push_back(currentChar);
                        }
                    }
                }
                text.push_back(0);

                auto messageOverride = mods::flow::override_message(group, id, language,
                    std::span{text});
                overridenMessage.push_back(std::move(messageOverride));
                overridenMessages++;
            }
        }
    }
    svc_log->debug(mod_ctx, fmt::format("replaced {} messages for {}", overridenMessages,
        languageFolder).c_str());
}
} // namespace

extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError*) {
    overrideMessages(MESSAGE_LANGUAGE_ENGLISH);
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    overridenMessage.clear();
    return MOD_OK;
}
}