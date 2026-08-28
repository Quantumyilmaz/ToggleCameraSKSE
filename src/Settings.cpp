#include "Settings.h"

void Settings::LoadDefaults() {
    Modules::Dialogue::LoadFeatures();
    Modules::Combat::LoadFeatures();
    Modules::Other::LoadFeatures();
}

void Settings::LoadSettings() {
    LoadDefaults();

    std::string filename = path;

    if (!std::filesystem::exists(filename)) {
        logger::info("Settings file does not exist. Creating default settings.");
        SaveSettings();
        return;
    }

    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        logger::error("Failed to open file for reading: {}", filename);
        return;
    }

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);
    if (doc.HasParseError()) {
        logger::error("Failed to parse JSON settings file: {}", filename);
        return;
    }
    if (doc.HasMember("dialogue")) Modules::Dialogue::from_json(doc["dialogue"]);
    if (doc.HasMember("combat")) Modules::Combat::from_json(doc["combat"]);
    if (doc.HasMember("other")) Modules::Other::from_json(doc["other"]);

    ifs.close();

    logger::info("Settings loaded from file: {}", filename);
}

void Settings::SaveSettings() {
    rapidjson::Document doc;
    doc.SetObject();

    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value version(rapidjson::kObjectType);
    version.AddMember("major", Utilities::plugin_version.major(), allocator);
    version.AddMember("minor", Utilities::plugin_version.minor(), allocator);
    version.AddMember("patch", Utilities::plugin_version.patch(), allocator);
    version.AddMember("build", Utilities::plugin_version.build(), allocator);

    doc.AddMember("plugin_version", version, allocator);

    doc.AddMember("dialogue", Modules::Dialogue::to_json(allocator), allocator);
    doc.AddMember("combat", Modules::Combat::to_json(allocator), allocator);
    doc.AddMember("other", Modules::Other::to_json(allocator), allocator);

    // Convert JSON document to string
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    // Write JSON to file
    std::string filename = path;
    std::filesystem::create_directories(std::filesystem::path(filename).parent_path());
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        logger::error("Failed to open file for writing: {}", filename);
        return;
    }
    ofs << buffer.GetString() << std::endl;
    ofs.close();
};

void Modules::Dialogue::funcToggle() {
    listen_gradual_zoom = false;
    listen_auto_zoom = false;

    const auto plyr_c = RE::PlayerCamera::GetSingleton();
    const auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(plyr_c->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].get());
    if (!thirdPersonState) {
        logger::error("ThirdPersonState is null.");
        return;
    }
    if (plyr_c->IsInFirstPerson()) {
        plyr_c->ForceThirdPerson();
        thirdPersonState->targetZoomOffset = Toggle.fix_zoom.enabled
                                                 ? Toggle.fix_zoom.zoom_lvl
                                                 : thirdPersonState->savedZoomOffset;
    } else if (plyr_c->IsInThirdPerson()) {
        thirdPersonState->savedZoomOffset = thirdPersonState->currentZoomOffset;
        if (!Toggle.instant) {
            listen_gradual_zoom = true;
            thirdPersonState->targetZoomOffset = -0.2f;
            return;
        }
        plyr_c->ForceFirstPerson();
    } else logger::error("Player is in neither 1st nor 3rd person.");

    listen_auto_zoom = true;
}

void Modules::Dialogue::funcZoom(const int a_device, const bool _in) {
    listen_gradual_zoom = false;
    listen_auto_zoom = false;

    const auto player_cam = RE::PlayerCamera::GetSingleton();
    const auto is_in_first = player_cam->IsInFirstPerson();

    const auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(player_cam->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].
            get());
    if (!thirdPersonState) {
        logger::error("ThirdPersonState is null.");
        return;
    }
    const float amount = (a_device % 2) ? 0.1f : 0.025f;
    if (_in) {
        if (is_in_first) {
        } else if (thirdPersonState->currentZoomOffset < -0.19f && !DisallowZoomPOVSwitch.enabled) player_cam->
            ForceFirstPerson();
        else thirdPersonState->targetZoomOffset = std::max(thirdPersonState->targetZoomOffset - amount, -0.2f);
    } else if (is_in_first) player_cam->ForceThirdPerson();
    else thirdPersonState->targetZoomOffset = std::min(thirdPersonState->targetZoomOffset + amount, 1.0f);

    listen_auto_zoom = true;
}

Purpose Modules::Dialogue::GetPurpose(const int a_device, const int keyMask) {
    constexpr auto failed = kNone;
    for (const auto& device : SupportedDevices) {
        if (a_device != device) continue;
        if (Toggle && keyMask == Toggle.keymap[device]) return kToggle;
        if (keyMask == ZoomEnable.keymap[device]) return kZoomEnable;
        if (ZoomIn && keyMask == ZoomIn.keymap[device]) return kZoomIn;
        if (ZoomOut && keyMask == ZoomOut.keymap[device]) return kZoomOut;
    }
    return failed;
}

rapidjson::Value Modules::Dialogue::to_json(Document::AllocatorType& a) {
    Value dialogue(kObjectType);

    Value toggle(kObjectType);
    Toggle.to_json(toggle, a);
    dialogue.AddMember("Toggle", toggle, a);

    Value zoom_enable(kObjectType);
    ZoomEnable.to_json(zoom_enable, a);
    dialogue.AddMember("ZoomEnable", zoom_enable, a);

    Value zoom_in(kObjectType);
    ZoomIn.to_json(zoom_in, a);
    dialogue.AddMember("ZoomIn", zoom_in, a);

    Value zoom_out(kObjectType);
    ZoomOut.to_json(zoom_out, a);
    dialogue.AddMember("ZoomOut", zoom_out, a);

    Value auto_toggle(kObjectType);
    AutoToggle.to_json(auto_toggle, a);
    dialogue.AddMember("AutoToggle", auto_toggle, a);

    Value disallow_zoom_pov_switch(kObjectType);
    DisallowZoomPOVSwitch.to_json(disallow_zoom_pov_switch, a);
    dialogue.AddMember("DisallowZoomPOVSwitch", disallow_zoom_pov_switch, a);

    return dialogue;
}

void Modules::Dialogue::from_json(const Value& j) {
    if (j.HasMember("Toggle")) Toggle.from_json(j["Toggle"]);
    if (j.HasMember("ZoomEnable")) ZoomEnable.from_json(j["ZoomEnable"]);
    if (j.HasMember("ZoomIn")) ZoomIn.from_json(j["ZoomIn"]);
    if (j.HasMember("ZoomOut")) ZoomOut.from_json(j["ZoomOut"]);
    if (j.HasMember("AutoToggle")) AutoToggle.from_json(j["AutoToggle"]);
    if (j.HasMember("DisallowZoomPOVSwitch")) DisallowZoomPOVSwitch.from_json(j["DisallowZoomPOVSwitch"]);
}

void Modules::Dialogue::LoadFeatures() {
    Toggle.enabled = true;
    Toggle.instant = false;

    ZoomEnable.enabled = true;
    ZoomIn.enabled = true;
    ZoomOut.enabled = true;

    DisallowZoomPOVSwitch.enabled = false;

    Toggle.keymap = {{0, 33}, {1, -1}, {2, 128}};
    ZoomEnable.keymap = {{0, 29}, {1, -1}, {2, 64}};
    ZoomIn.keymap = {{0, -1}, {1, 8}, {2, 10}};
    ZoomOut.keymap = {{0, -1}, {1, 9}, {2, 512}};
};

bool Modules::Combat::Is3rdP() {
    const auto plyr_c = RE::PlayerCamera::GetSingleton();
    if (!plyr_c) {
        logger::error("PlayerCamera is null.");
        return false;
    }
    if (plyr_c->IsInFirstPerson()) return false;
    const auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(plyr_c->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].get());
    if (thirdPersonState->targetZoomOffset != thirdPersonState->currentZoomOffset &&
        thirdPersonState->targetZoomOffset == -0.2f && listen_gradual_zoom) {
        return false;
    }
    return plyr_c->IsInThirdPerson();
}

void Modules::Combat::funcToggle(Feature& feat) {
    const auto plyr_c = RE::PlayerCamera::GetSingleton();
    if (!plyr_c) {
        logger::error("PlayerCamera is null.");
        return;
    }
    const bool is3rdP = Is3rdP();
    listen_gradual_zoom = false;
    const auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(plyr_c->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].get());
    if (!thirdPersonState) {
        logger::error("ThirdPersonState is null.");
        return;
    }
    if (!is3rdP) {
        plyr_c->ForceThirdPerson();
        thirdPersonState->targetZoomOffset = feat.fix_zoom.enabled ? feat.fix_zoom.zoom_lvl : savedZoomOffset;
    } else if (!feat.instant) {
        listen_gradual_zoom = true;
        thirdPersonState->targetZoomOffset = -0.2f;
    } else plyr_c->ForceFirstPerson();
}

uint32_t Modules::Combat::CamSwitchHandling(const uint32_t newstate, const bool third2first, const bool switch_back) {
    // Toggle i call lamali miyiz ona bakiyoruz
    const bool is_3rd_p = Is3rdP();
    const bool player_is_in_toggled_cam = third2first ? !is_3rd_p : is_3rd_p;

    if (newstate) {
        if (player_is_in_toggled_cam) {
            return 0;
        }
    } else {
        if (!player_is_in_toggled_cam) {
            return 0;
        }
        if (!switch_back) {
            return 0;
        }
    }
    return 1;
}

rapidjson::Value Modules::Combat::to_json(Document::AllocatorType& a) {
    Value combat(kObjectType);

    Value toggle_combat(kObjectType);
    ToggleCombat.to_json(toggle_combat, a);
    combat.AddMember("ToggleCombat", toggle_combat, a);

    Value toggle_weapon(kObjectType);
    ToggleWeapon.to_json(toggle_weapon, a);
    combat.AddMember("ToggleWeapon", toggle_weapon, a);

    Value toggle_bow_draw(kObjectType);
    ToggleBowDraw.to_json(toggle_bow_draw, a);
    combat.AddMember("ToggleBowDraw", toggle_bow_draw, a);

    Value toggle_magic_wield(kObjectType);
    ToggleMagicWield.to_json(toggle_magic_wield, a);
    combat.AddMember("ToggleMagicWield", toggle_magic_wield, a);

    Value toggle_magic_cast(kObjectType);
    ToggleMagicCast.to_json(toggle_magic_cast, a);
    combat.AddMember("ToggleMagicCast", toggle_magic_cast, a);

    Value toggle_sneak(kObjectType);
    ToggleSneak.to_json(toggle_sneak, a);
    combat.AddMember("ToggleSneak", toggle_sneak, a);

    return combat;
}

void Modules::Combat::from_json(const Value& j) {
    if (j.HasMember("ToggleCombat")) ToggleCombat.from_json(j["ToggleCombat"]);
    if (j.HasMember("ToggleWeapon")) ToggleWeapon.from_json(j["ToggleWeapon"]);
    if (j.HasMember("ToggleBowDraw")) ToggleBowDraw.from_json(j["ToggleBowDraw"]);
    if (j.HasMember("ToggleMagicWield")) ToggleMagicWield.from_json(j["ToggleMagicWield"]);
    if (j.HasMember("ToggleMagicCast")) ToggleMagicCast.from_json(j["ToggleMagicCast"]);
    if (j.HasMember("ToggleSneak")) ToggleSneak.from_json(j["ToggleSneak"]);
}

void Modules::Combat::LoadFeatures() {
    ToggleCombat.enabled = true;
    ToggleWeapon.enabled = true;
    ToggleBowDraw.enabled = true;
    ToggleMagicWield.enabled = true;
    ToggleMagicCast.enabled = true;
    ToggleSneak.enabled = true;

    // 0: do not ignore, 1: ignore if 1+ hand, 2: ignore if 2 hands
    ToggleMagicWield.keymap[-1] = 0; // None
    ToggleMagicWield.keymap[0] = 0; // Self
    ToggleMagicWield.keymap[1] = 0; // Touch
    ToggleMagicCast.keymap[2] = 0; // Aimed
    ToggleMagicCast.keymap[3] = 0; // Target Actor
    ToggleMagicCast.keymap[4] = 0; // Target Location
}

void Modules::Other::funcToggle(const bool is3rdP, const float extra_offset) {
    const auto plyr_c = RE::PlayerCamera::GetSingleton();
    if (!plyr_c) {
        logger::error("PlayerCamera is null.");
        return;
    }
    const auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(plyr_c->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].get());
    if (!thirdPersonState) {
        logger::error("ThirdPersonState is null.");
        return;
    }
    if (!is3rdP) {
        const auto savedZoomOffset = thirdPersonState->savedZoomOffset;
        plyr_c->ForceThirdPerson();
        thirdPersonState->targetZoomOffset = savedZoomOffset + extra_offset;
    } else plyr_c->ForceFirstPerson();
};

rapidjson::Value Modules::Other::to_json(Document::AllocatorType& a) {
    Value other(kObjectType);

    Value toggle_cell_change_exterior(kObjectType);
    ToggleCellChangeExterior.to_json(toggle_cell_change_exterior, a);
    other.AddMember("ToggleCellChangeExterior", toggle_cell_change_exterior, a);

    Value toggle_cell_change_interior(kObjectType);
    ToggleCellChangeInterior.to_json(toggle_cell_change_interior, a);
    other.AddMember("ToggleCellChangeInterior", toggle_cell_change_interior, a);

    Value fix_zoom_(kObjectType);
    FixZoom.to_json(fix_zoom_, a);
    other.AddMember("FixZoom", fix_zoom_, a);

    return other;
}

void Modules::Other::from_json(const Value& j) {
    if (j.HasMember("ToggleCellChangeExterior")) ToggleCellChangeExterior.from_json(j["ToggleCellChangeExterior"]);
    if (j.HasMember("ToggleCellChangeInterior")) ToggleCellChangeInterior.from_json(j["ToggleCellChangeInterior"]);
    if (j.HasMember("FixZoom")) FixZoom.from_json(j["FixZoom"]);
}

void Modules::Other::LoadFeatures() {
    ToggleCellChangeExterior.enabled = false;
    ToggleCellChangeExterior.instant = false;

    ToggleCellChangeInterior.enabled = false;
    ToggleCellChangeInterior.instant = false;
}

void Feature::to_json(rapidjson::Value& j, rapidjson::Document::AllocatorType& a) const {
    j.AddMember("enabled", enabled, a);
    j.AddMember("instant", instant, a);
    j.AddMember("invert", invert, a);
    j.AddMember("revert", revert, a);
    rapidjson::Value keymap_array(rapidjson::kArrayType);
    for (const auto& [device, key] : keymap) {
        rapidjson::Value keymap_obj(rapidjson::kObjectType);
        keymap_obj.AddMember("device", device, a);
        keymap_obj.AddMember("key", key, a);
        keymap_array.PushBack(keymap_obj, a);
    }
    j.AddMember("keymap", keymap_array, a);
    j.AddMember("fixed_zoom_enabled", fix_zoom.enabled, a);
    j.AddMember("fixed_zoom_lvl", fix_zoom.zoom_lvl, a);
}

void Feature::from_json(const rapidjson::Value& j) {
    enabled = j["enabled"].GetBool();
    instant = j["instant"].GetBool();
    invert = j["invert"].GetBool();
    revert = j["revert"].GetBool();
    const rapidjson::Value& keymap_array = j["keymap"];
    for (const auto& keymap_obj : keymap_array.GetArray()) {
        keymap[keymap_obj["device"].GetInt()] = keymap_obj["key"].GetInt();
    }
    fix_zoom.enabled = j["fixed_zoom_enabled"].GetBool();
    fix_zoom.zoom_lvl = j["fixed_zoom_lvl"].GetFloat();
}