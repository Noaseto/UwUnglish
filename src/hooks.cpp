#include "mods/svc/hook.hpp"

#include <nlohmann/json.hpp>
#include "fmt/format.h"

#include "hooks.hpp"

#include "util.hpp"
#include "helpers/string.hpp"
#include "mods/svc/log.h"
#include "mods/svc/resource.h"

#include "d/d_meter2_info.h"
#include "d/d_msg_scrn_light.h"
#include "d/d_msg_object.h"
#include "d/d_msg_unit.h"

namespace uwunglish::hooks {
namespace {
using json = nlohmann::json;

DEFINE_HOOK(&dMeter2Info_c::getStringKanji, dMeter2Info_c__GetStringKanji);

DEFINE_HOOK(&dMeter2Info_c::getStringKana, dMeter2Info_c__GetStringKana);

DEFINE_HOOK(&dMeter2Info_c::getString, dMeter2Info_c__GetString);

// Portal names are retrieved via a pointer inside the bmg, to rebuild the string
/*
 * "Want to twavew to {1a06ff000001}{1a0500002b}{1a06ff000000}?"
 * 1a06ff000001 : red
 * 1a0500002b : portal name
 * 1a06ff000000 : white
 */
DEFINE_HOOK(&dMsgObject_c::getPortalName, dMsgObject_c__GetPortalName);

// override the max arrow tag
DEFINE_HOOK(&dMsgUnit_c::setTag, dMsgOUnit_c__setTag);

// little trick because there are 2 functions called draw
using LightDrawFn = void (dMsgScrnLight_c::*)(f32*, f32, f32, f32, f32, f32, f32, JUtility::TColor,
    JUtility::TColor);
DEFINE_HOOK(static_cast<LightDrawFn>(&dMsgScrnLight_c::draw), dMsgScrnLight_c__draw);


std::unordered_map<uint32_t, std::string> hookMap = {
    // Plumm mini-game : score
    {0x53e, ""},

    //// Collection
    /* collection title*/{0x3E1, ""},
    /* nothing */
    // Swords
    /* Ordon Sword */{0x18d, ""},/* Wooden Sword */{0x1a4, ""},
    /* Master Sword (infused) */{0x1ae, ""},/* Master Sword */{0x18e, ""},
    // Shields
    /* Ordon Shield */{0x18f, ""},/* Wooden Shield */{0x190, ""},/* Hylian Shield */{0x191, ""},
    // Clothes
    /* Hero's Clothes */ {0x194, ""},/* Zora Armor */ {0x196, ""},/* Magic Armor */ {0x195, ""},
    // Heart
    /* Piece of heart*/{0x186, ""},
    // Key items
    /* Mirror shards (x3)*/{0x20a, ""}, {0x20b, ""}, {0x20c, ""},
    // Wallet
    /* Base */ {0x199, ""}, /* Big */ {0x19a, ""}, /* Giant */ {0x19b, ""},
    // Quiver
    /* Base */ {0x1b9, ""}, /* Big */ {0x1ba, ""}, /* Giant */ {0x1bb, ""},
    // Golden bugs
    /* Details */{0x5b8, ""},
    /* NO, when giving bug to agitha */{0x4be, ""},
    // Hidden skills
    {0x5b0, ""},
    // scent
    /* Ilia */{0x215, ""},
    /* Poe */ {0x217, ""},
    /* Reekfish */{0x218, ""},
    /* Youths */ {0x219, ""},
    /* Medicine */{0x21a, ""},
    // Poe souls
    {0x245, ""},
    // Fish journal
    {0x5a1, ""},
    // fish names
    {0x59E, ""}, {0x59D, ""}, {0x59B, ""}, {0x599, ""}, {0x59A, ""}, {0x59C, ""},
    // Largest, number caught
    {0x59F, ""}, {0x5A0, ""},
    // Letters
    {0x4c8, ""},

    //// Start menu, probably already overriden with message services
    /* Choose a quest log, file selection */ {0x43, ""},
    /* Copy to where */{0x44, ""},
    /* New quest log */{0x4d, ""},
    /* Press start */ {0x64, ""},
    /* Confirm */ {0x55, ""},
    /* Erase */{0x57, ""},
    /* Start */{0x58, ""},
    /* Total play time */{0x3d1, ""},
    /* Link */{0x382, ""},
    /* Epona */{0x383, ""},
    /* Enter horse name */{0x384, ""},
    /* Enter name */{0x385, ""},
    /* Quest log 1 */{0x40, ""},
    /* Quest log 2 */{0x41, ""},
    /* Quest log 3 */{0x42, ""},

    //// Game over
    /* game over */ {0x381, ""},

    //// Minigames
    /* start */{0x3e3, ""},
    /* ready */{0x515, ""},
    /* lose */{0x539, ""},

    ////Map
    // world
    /* Hyrule Kingdom */ {0x579, ""}, {0x583, ""},
    // provinces
    /* Ordona */{0x57d, ""}, {0x57a, ""}, {0x57b, ""}, {0x57c, ""},
    /* Faron */{0x57e, ""}, {0x580, ""}, {0x58d, ""},
    /* Eldin */{0x584, ""}, {0x581, ""}, {0x582, ""}, {0x589, ""}, {0x58f, ""},
    /* Lanayru */{0x588, ""}, {0x585, ""}, {0x586, ""}, {0x587, ""}, {0x58c, ""}, {0x58e, ""},
    /* Peak */{0x590, ""}, {0x58a, ""},
    /* Desert */{0x591, ""}, {0x58b, ""},
    // buttons
    /* Z  */ {0x529, ""}, {0x52a, ""},
    /* Stick */{0x524, ""}, {0x525, ""}, {0x526, ""}, {0x528, ""},
    // portals
    /* Ordon Spring */{0x5cd, ""},
    /* S. Faron Woods */{0x5ce, ""},
    /* N. Faron Woods */{0x5cf, ""},
    /* Kakariko village */{0x5d0, ""},
    /* Death Mountain */{0x5d1, ""},
    /* Castle Town */{0x5d2, ""},
    /* Zora's Domain */{0x5d3, ""},
    /* Lake Hylia */{0x5d4, ""},
    /* Kakariko Gorge */{0x5d5, ""},
    /* Bridge of Eldin */{0x5d6, ""},
    /* Snowpeak Top */{0x5d7, ""},
    /* Gerudo Mesa */{0x5d8, ""},
    /* Upper Zora's River */{0x5d9, ""},
    /* Sacred Grove */{0x5da, ""},
    /* Mirror Chamber */{0x5db, ""},

    //// NO, I don't know where they are all displayed
    /* No */ {0x8, ""}, // Generic no
    /* NO */{0x519, ""},
    /* NO */{0x51C, ""},
    /* NO */{0x4b1, ""}, // should be portal
    /* NO */{0x520, ""}, // should be portal

    //// Letters
    /* Turn page */ {0x4d7, ""},
    /* Next */{0x408, ""},

    //// Hidden skills
    {1708, ""}, {1709, ""}, {1710, ""}, {1711, ""}, {1712, ""}, {1713, ""}, {1714, ""},

    //// action strings (from getActionString)
    // roll, throw, etc...
    {891, ""},   // Move (in dungeon'a map)
    {0x3d6, ""}, // Roll
    {981, ""},   // clear
    {0x368, ""}, // details
    {983, ""},
    {1001, ""},
    {1005, ""},
    {1007, ""},
    {1010, ""},
    {1015, ""},
    {1018, ""},
    {1019, ""},
    {1020, ""},
    {1024, ""},
    {1026, ""},
    {1028, ""},
    {1029, ""},
    {1032, ""},
    {1033, ""},
    {1035, ""},
    {1036, ""},
    {1039, ""},
    {1042, ""},
    {1043, ""},
    {1046, ""},
    {1049, ""},
    {1050, ""},
    {1053, ""},
    {1054, ""},
    {1055, ""},
    {1057, ""},
    {1059, ""},
    {1060, ""},
    {1062, ""}, // Slap
    {1063, ""},
    {0x42b, ""}, // Roll
    {1068, ""},
    {1071, ""},
    {1074, ""},
    {1075, ""},
    {1077, ""},
    {1079, ""},
    {1080, ""},
    {1081, ""},
    {1085, ""},
    {1086, ""},
    {1087, ""},
    {1092, ""},
    {1093, ""},
    {1094, ""},
    {1096, ""},
    {1097, ""},
    {1144, ""},
    {1145, ""},
    {1146, ""},
    {1148, ""},
    {1160, ""},
    {1163, ""},
    {1164, ""},
    {1165, ""},
    {1166, ""},
    {1204, ""},
    {1237, ""}, // howl
    {1377, ""},
    {1399, ""},
    {1904, ""},
};

ModResult loadFile(const std::string& filePath, std::unordered_map<uint32_t, std::string>& map) {
    ResourceBuffer buffer = RESOURCE_BUFFER_INIT;
    ModResult result = svc_resource->load(mod_ctx, filePath.c_str(), &buffer);
    if (result != MOD_OK) {
        return result;
    }
    const char* start = static_cast<const char*>(buffer.data);
    const char* end = start + buffer.size;
    json elements = json::parse(start, end, nullptr, false);
    svc_resource->free(mod_ctx, &buffer);

    for (const auto& element : elements) {
        std::string fullId = element.at("ID").get<std::string>();
        int leftId;
        int rightId;
        std::sscanf(fullId.c_str(), "%d , %d", &leftId, &rightId);
        // game uses uint32 despite our ID staying on 16 bits
        uint32_t id = util::computeId(leftId, rightId);

        if (!map.contains(id)) {
            svc_log->error(mod_ctx,
                fmt::format("found unexpected {} ID, inside {} file", id, filePath).c_str());
            return MOD_ERROR;
        }

        std::string text;
        for (const auto& jsonLine : element.at("text")) {
            text += util::UTF8ToCP1252(jsonLine.get<std::string>());
        }
        map[id] = std::move(text);
    }
    svc_log->info(mod_ctx, fmt::format("{} has been loaded", filePath).c_str());
    return MOD_OK;
}

void hookPost_GetStringKanji_uwufied(ModContext*, void* args, void* retval, void* userdata) {
    const uint32_t idFetched = mods::arg<uint32_t>(args, 1); // arg 0 is `this`
    if (!hookMap.contains(idFetched))
        return;

    const TextSpan o_string = mods::arg<TextSpan>(args, 2);
    if (o_string.buffer == nullptr || o_string.size == 0)
        return;
    std::string text = hookMap[idFetched];

    std::memcpy(o_string.buffer, text.data(), text.size());
    o_string.buffer[text.size()] = '\0';
}

// this is needed for the text displayed on the A button
void hookPost_GetStringKana_uwufied(ModContext*, void* args, void* retval, void* userdata) {
    const uint32_t idFetched = mods::arg<uint32_t>(args, 1); // arg 0 is `this`
    if (!hookMap.contains(idFetched))
        return;

    const TextSpan o_string = mods::arg<TextSpan>(args, 2);
    if (o_string.buffer == nullptr || o_string.size == 0)
        return;
    std::string text = hookMap[idFetched];

    std::memcpy(o_string.buffer, text.data(), text.size());
    o_string.buffer[text.size()] = '\0';
}

// used for the Game over texts, maybe others too
void hookPost_GetString_uwufied(ModContext*, void* args, void* retval, void* userdata) {
    const uint32_t idFetched = mods::arg<uint32_t>(args, 1); // arg 0 is `this`
    if (!hookMap.contains(idFetched))
        return;

    const TextSpan o_string = mods::arg<TextSpan>(args, 2);
    if (o_string.buffer == nullptr || o_string.size == 0)
        return;
    std::string text = hookMap[idFetched];

    std::memcpy(o_string.buffer, text.data(), text.size());
    o_string.buffer[text.size()] = '\0';
}

// when confirming the warping action
void hookPost_GetPortalName_uwufied(ModContext*, void* args, void* retval, void* userdata) {
    auto* self = mods::arg<dMsgObject_c*>(args, 0); // arg 0 is `this`
    // portalID is set in the void dMenu_Fmap_c::portal_warp_map_proc() which eventually calls
    // the getPortalName
    const uint32_t id = self->getPortalMessageID();
    // min and max id for the 15 portals
    if (id < 0x5cd || id > 0x5db)
        return;

    const std::string& text = hookMap[id];
    *static_cast<const char**>(retval) = text.c_str();
}

// original Gameover golden shadows
f32 originalOffsetX[8] = {-138.0f, -96.0f, -56.0f, -18.0f, 42.0f, 75.0f, 110.0f, 143.0f};
// eyeballed offset, maybe I could compute each symbol size but... that works too
f32 uwuOffsetX[7] = {-117.0f, -76.0f, -36.0f, 0.0f, 50.0f, 85.0f, 124.0f};
constexpr f32 baseWidthCenter = 608.0f / 2.0f;
constexpr f32 wideWidthCenter = 640.0f / 2.0f;

HookAction hookPre_draw(ModContext*, void* args, void* retval, void* userdata) {
    f32& xOffset = mods::arg_ref<f32>(args, 2); // arg 0 is `this`
    if (std::fabs(xOffset - originalOffsetX[7] - baseWidthCenter) < 0.01f ||
        std::fabs(xOffset - originalOffsetX[7] - wideWidthCenter) < 0.01f) {
        return HOOK_SKIP_ORIGINAL;
    }
    for (size_t i = 0; i < 7; i++) {
        if (std::fabs(xOffset - originalOffsetX[i] - baseWidthCenter) < 0.01f) {
            xOffset = uwuOffsetX[i] + baseWidthCenter;
            break;
        }
        if (std::fabs(xOffset - originalOffsetX[i] - wideWidthCenter) < 0.01f) {
            xOffset = uwuOffsetX[i] + wideWidthCenter;
            break;
        }
    }
    return HOOK_CONTINUE;
}

// the hook works, but I don't see where the buffer is set with the "30|60|100 arrows" value :/
void hookPost_setTag(ModContext*, void* args, void* retval, void* userdata) {
    char* buffer = mods::arg_ref<char*>(args, 3);
    if (buffer == nullptr)
        return;

    for (char* p = buffer; *p != '\0'; p++) {
        if ((*p == 'a' || *p == 'A') && std::strncmp(p + 1, "rrow", 4) == 0) {
            std::memcpy(p + 1, "wwow", 4);
            p += 4;
        }
    }
}
}

ModResult initialize() {
    ModResult result = loadFile("english/hookTexts.json", hookMap);
    if (result != MOD_OK) {
        return result;
    }

    if (mods::hook::add_post<dMeter2Info_c__GetStringKanji>(hookPost_GetStringKanji_uwufied) !=
        MOD_OK) {
        svc_log->error(mod_ctx, fmt::format("Failed to add post-hook for {}",
            "dMeter2Info_c__GetStringKanji").c_str());
        return MOD_ERROR;
    }
    if (mods::hook::add_post<dMeter2Info_c__GetStringKana>(hookPost_GetStringKana_uwufied) !=
        MOD_OK) {
        svc_log->error(mod_ctx, fmt::format("Failed to add post-hook for {}",
            "dMeter2Info_c__GetStringKana").c_str());
        return MOD_ERROR;
    }
    if (mods::hook::add_post<dMeter2Info_c__GetString>(hookPost_GetString_uwufied) !=
        MOD_OK) {
        svc_log->error(mod_ctx, fmt::format("Failed to add post-hook for {}",
            "dMeter2Info_c__GetString").c_str());
        return MOD_ERROR;
    }
    if (mods::hook::add_post<dMsgObject_c__GetPortalName>(hookPost_GetPortalName_uwufied) !=
        MOD_OK) {
        svc_log->error(mod_ctx, fmt::format("Failed to add post-hook for {}",
            "dMsgObject_c__GetPortalName").c_str());
        return MOD_ERROR;
    }
    if (mods::hook::add_pre<dMsgScrnLight_c__draw>(hookPre_draw) !=
        MOD_OK) {
        svc_log->error(mod_ctx, fmt::format("Failed to add pre-hook for {}",
            "dMsgScrnLight_c__draw").c_str());
        return MOD_ERROR;
    }
    if (mods::hook::add_post<dMsgOUnit_c__setTag>(hookPost_setTag) !=
        MOD_OK) {
        svc_log->error(mod_ctx, fmt::format("Failed to add pre-hook for {}",
            "dMsgScrnLight_c__draw").c_str());
        return MOD_ERROR;
    }

    return MOD_OK;
}

ModResult uninstall() {
    mods::hook::uninstall<dMeter2Info_c__GetStringKanji>(svc_hook);
    mods::hook::uninstall<dMeter2Info_c__GetStringKana>(svc_hook);
    mods::hook::uninstall<dMeter2Info_c__GetString>(svc_hook);
    mods::hook::uninstall<dMsgObject_c__GetPortalName>(svc_hook);
    mods::hook::uninstall<dMsgScrnLight_c__draw>(svc_hook);
    mods::hook::uninstall<dMsgOUnit_c__setTag>(svc_hook);
    return MOD_OK;
}
}