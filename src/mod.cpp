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

#include "TE_localization.h"

#include <cstdio>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <map>

#include "hooks.hpp"
#include "util.hpp"
#include "modUtil.hpp"

DEFINE_MOD();

IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(MessageService, svc_message);
IMPORT_SERVICE(ResourceService, svc_resource);
IMPORT_SERVICE(HookService, svc_hook);

IMPORT_OPTIONAL_SERVICE(TwilitEssentialsLocalizationService, svc_te_localization);
static TwilitEssentialsLanguageHandle s_language = 0;

using json = nlohmann::json;

namespace {
std::vector<mods::flow::MessageOverride> overridenMessage;
uint16_t kGroupCount = 9;
std::map<std::pair<uint16_t, uint16_t>, std::vector<uint8_t> > zelBmgMap;

void loadFolder(const std::string& languageFolder,
    std::map<std::pair<uint16_t, uint16_t>, std::vector<uint8_t> >& map) {
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
                uint16_t id = uwunglish::util::computeId(leftId, rightId);
                if (map.contains(std::pair(group, id))) {
                    // map has already been filled there, we skip
                    continue;
                }

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
                    std::string line = uwunglish::util::UTF8ToCP1252(utf8Line);

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
                map[std::pair(group, id)] = text;
            }
        }
    }
}

void overrideMessage(const std::map<std::pair<uint16_t, uint16_t>, std::vector<uint8_t> >& map,
    MessageLanguage language) {
    uint16_t overridenMessages = 0;
    for (auto& [key, text] : map) {
        auto [group, id] = key;
        auto messageOverride = mods::flow::override_message(group, id, language,
            std::span{text});
        overridenMessage.push_back(std::move(messageOverride));
        overridenMessages++;
    }
    svc_log->debug(mod_ctx, fmt::format("replaced {} messages for {}", overridenMessages,
        "english").c_str());
}

bool isLinkleModActive;

void overrideMessages(bool isLinkleModActive) {
    // load the override map
    if (isLinkleModActive) {
        loadFolder("linkle/english/", zelBmgMap);
    }
    loadFolder("english/", zelBmgMap);
    overrideMessage(zelBmgMap, MESSAGE_LANGUAGE_ENGLISH);
}
} // namespace

extern "C" {
MOD_EXPORT ModResult mod_initialize(ModError*) {
    isLinkleModActive = uwunglish::modUtil::is_mod_enabled("com.ditrey.linkle");
    overrideMessages(isLinkleModActive);

    ModResult result = uwunglish::hooks::initialize(isLinkleModActive);
    if (result != MOD_OK) {
        return result;
    }

    // Add a language option inside Twilit Essentials if it exists
    if (svc_te_localization) {
        TwilitEssentialsLanguageDesc desc = TWILIT_ESSENTIALS_LANGUAGE_DESC_INIT;
        desc.code = "en_uwu";
        desc.name = "UwUnglish";
        desc.fallback = "en";
        svc_te_localization->add_language(mod_ctx, &desc, &s_language);
    }

    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    bool isEnabled = uwunglish::modUtil::is_mod_enabled("com.ditrey.linkle");
    if (isLinkleModActive != isEnabled) {
        isLinkleModActive = isEnabled;
        zelBmgMap.clear();
        overridenMessage.clear();
        uwunglish::hooks::uninstall();

        overrideMessages(isLinkleModActive);
        uwunglish::hooks::initialize(isLinkleModActive);
    }
    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    overridenMessage.clear();
    uwunglish::hooks::uninstall();

    if (svc_te_localization && s_language != 0) {
        svc_te_localization->remove_language(mod_ctx, s_language);
        s_language = 0;
    }

    return MOD_OK;
}
}