#pragma once

#include <mods/service.hpp>

#define TWILIT_ESSENTIALS_LOCALIZATION_SERVICE_ID "com.dusklight.twilit_essentials.localization"
#define TWILIT_ESSENTIALS_LOCALIZATION_SERVICE_MAJOR 1u
#define TWILIT_ESSENTIALS_LOCALIZATION_SERVICE_MINOR 0u

typedef uint64_t TwilitEssentialsLanguageHandle;

struct TwilitEssentialsLanguageDesc {
    // Size of the struct, used for version compatibility
    uint32_t struct_size;

    // Language code and display name
    const char* code;
    const char* name;

    // Language to fall back to when a translation is missing
    const char* fallback;

    // Called to translate a text using its key
    const char* (*translate)(const char* key, const char* text, void* user_data);

    // Custom data passed to the translation callback
    void* user_data;
};

#define TWILIT_ESSENTIALS_LANGUAGE_DESC_INIT \
{sizeof(TwilitEssentialsLanguageDesc), nullptr, nullptr, nullptr, nullptr, nullptr}

struct TwilitEssentialsLocalizationService {
    ServiceHeader header;

    // Registers a language and returns its handle
    ModResult (*add_language)(ModContext* caller, const TwilitEssentialsLanguageDesc* desc,
        TwilitEssentialsLanguageHandle* out_handle);

    // Removes a registered language by its handle
    ModResult (*remove_language)(ModContext* caller, TwilitEssentialsLanguageHandle handle);

    // Adds or updates a translation for the given language
    ModResult (*set_text)(ModContext* caller, TwilitEssentialsLanguageHandle handle, const char* key,
        const char* text);
};

MOD_DECLARE_SERVICE(TwilitEssentialsLocalizationService, svc_te_localization,
    TWILIT_ESSENTIALS_LOCALIZATION_SERVICE_ID, TWILIT_ESSENTIALS_LOCALIZATION_SERVICE_MAJOR,
    TWILIT_ESSENTIALS_LOCALIZATION_SERVICE_MINOR);