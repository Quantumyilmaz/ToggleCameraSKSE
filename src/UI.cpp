#include "UI.h"

void __stdcall MCP::RenderLog() {
    // add checkboxes to filter log levels
    ImGuiMCP::Checkbox("Trace", &LogSettings::log_trace);
    ImGuiMCP::SameLine();
    ImGuiMCP::Checkbox("Info", &LogSettings::log_info);
    ImGuiMCP::SameLine();
    ImGuiMCP::Checkbox("Warning", &LogSettings::log_warning);
    ImGuiMCP::SameLine();
    ImGuiMCP::Checkbox("Error", &LogSettings::log_error);

    // if "Generate Log" button is pressed, read the log file
    if (ImGuiMCP::Button("Generate Log")) logLines = Utilities::ReadLogFile();

    // Display each line in a new ImGui::Text() element
    for (const auto& line : logLines) {
        if (line.find("trace") != std::string::npos && !LogSettings::log_trace) continue;
        if (line.find("info") != std::string::npos && !LogSettings::log_info) continue;
        if (line.find("warning") != std::string::npos && !LogSettings::log_warning) continue;
        if (line.find("error") != std::string::npos && !LogSettings::log_error) continue;
        ImGuiMCP::Text(line.c_str());
    }
}

void MCP::RenderCheckBox(const std::string& title, const std::string& label, bool& enabled) {
    ImGuiMCP::Checkbox((label + "##" + title).c_str(), &enabled);
}

void MCP::RenderDeviceKeyCombo(const std::string& title, const std::string& label, bool& enabled, int& selected_device,
                               std::map<int, int>& keymap) {
    ImGuiMCP::Checkbox((label + ":##" + title).c_str(), &enabled);
    ImGuiMCP::SameLine();
    ImGuiMCP::SetCursorPosX(170);
    ImGuiMCP::Text("");
    ImGuiMCP::SameLine();
    ImGuiMCP::Text(device_names[selected_device].c_str());
    ImGuiMCP::SameLine();
    ImGuiMCP::Text("Key");
    ImGuiMCP::SameLine();
    ImGuiMCP::SetNextItemWidth(100);

    const std::string combo_label = "##key_combo_" + std::to_string(selected_device) + label;
    // Ensure unique ID for each combo
    if (ImGuiMCP::BeginCombo(combo_label.c_str(), std::to_string(keymap[selected_device]).c_str())) {
        for (int n = -1; n < 600; ++n) {
            const bool is_selected = keymap[selected_device] == n;
            if (ImGuiMCP::Selectable(std::to_string(n).c_str(), is_selected)) keymap[selected_device] = n;
            if (is_selected) {
                ImGuiMCP::SetItemDefaultFocus();
            }
        }
        ImGuiMCP::EndCombo();
    }
}

void MCP::RenderZoomLvL(const std::string& title, const std::string& label, Feature& feat, bool show_instant) {
    RenderCheckBox(title + label, "FixZoom", feat.fix_zoom.enabled);
    if (show_instant) {
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox((std::string("Instant Zoom") + "##" + title + label).c_str(), &feat.fix_zoom.instant);
    }
    ImGuiMCP::SameLine();
    ImGuiMCP::SetNextItemWidth(100);
    float& zoom_lvl = feat.fix_zoom.zoom_lvl;
    if (ImGuiMCP::BeginCombo(("##FixZoomValue" + label).c_str(), Utilities::formatFloatToString(zoom_lvl, 1).c_str())) {
        for (int n = -10; n < 11; ++n) {
            const bool is_selected = std::abs(zoom_lvl - n / 10.f) < 0.0000001f;
            if (ImGuiMCP::Selectable(Utilities::formatFloatToString(n / 10.f, 1).c_str(), is_selected))
                zoom_lvl = n / 10.f;
            if (is_selected) ImGuiMCP::SetItemDefaultFocus();
        }
        ImGuiMCP::EndCombo();
    }
};

void MCP::Register() {
    if (!SKSEMenuFramework::IsInstalled()) {
        logger::critical("SKSE Menu Framework is not installed. Cannot register menu.");
        return;
    }
    SKSEMenuFramework::SetSection(Utilities::mod_name);
    SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);
    //SKSEMenuFramework::AddSectionItem("Status", RenderStatus);
    SKSEMenuFramework::AddSectionItem("Log", RenderLog);
}

void __stdcall MCP::RenderSettings() {
    // add a save button
    if (ImGuiMCP::Button("Save Settings")) {
        Settings::SaveSettings();
    }

    ImGuiMCP::SameLine();

    if (ImGuiMCP::Button("Load Settings")) {
        Settings::LoadSettings();
    }

    ImGuiMCP::SameLine();

    // Key Detection
    if (ImGuiMCP::Button("Start Key Detection")) {
        detected_key = -1;
        detected_device = -1;
        listen_key = true;
    }
    if (listen_key) {
        ImGuiMCP::SameLine();
        ImGuiMCP::Text("Listening for key press...");
    }
    if (detected_key >= 0) {
        ImGuiMCP::SameLine();
        ImGuiMCP::Text("Detected Key: %d, Device: %s", detected_key, device_names[detected_device].c_str());
    }
    // help marker
    ImGuiMCP::SameLine();
    HelpMarker("Click 'Start',close this menu and press a key to detect it. You can view the detected key here.");
    Dialogue::Render();
    Combat::Render();
    Other::Render();
};

void MCP::Dialogue::Render() {
    std::string title = "Dialogue";
    if (ImGuiMCP::CollapsingHeader((title + "##Settings").c_str(), ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderEnableDisableAll();
        ImGuiMCP::SameLine();
        ImGuiMCP::Text("Device Selection: ");
        ImGuiMCP::SameLine();
        auto& selected_device = Toggle::selected_device;
        const std::string label = "Toggle";
        ImGuiMCP::SetNextItemWidth(200);
        if (ImGuiMCP::BeginCombo(("##device_combo" + title + label).c_str(), device_names[selected_device].c_str(),
                                 ImGuiMCP::ImGuiComboFlags_HeightSmall)) {
            for (int n = 0; n < device_names.size(); ++n) {
                const bool is_selected = (selected_device == n);
                if (ImGuiMCP::Selectable(device_names[n].c_str(), is_selected)) {
                    selected_device = n;
                }
                if (is_selected) ImGuiMCP::SetItemDefaultFocus();
            }
            ImGuiMCP::EndCombo();
        }
        RenderDeviceKeyCombo(title, "Toggle", Modules::Dialogue::Toggle.enabled, Toggle::selected_device,
                             Modules::Dialogue::Toggle.keymap);
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox((std::string("InstantZoom") + "##" + title).c_str(), &Modules::Dialogue::Toggle.instant);
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox((std::string("Revert") + "##" + title + label).c_str(),
                           &Modules::Dialogue::Toggle.revert);
        RenderDeviceKeyCombo(title, "ZoomEnabler", Modules::Dialogue::ZoomEnable.enabled, Toggle::selected_device,
                             Modules::Dialogue::ZoomEnable.keymap);
        RenderDeviceKeyCombo(title, "ZoomIn", Modules::Dialogue::ZoomIn.enabled, Toggle::selected_device,
                             Modules::Dialogue::ZoomIn.keymap);
        RenderDeviceKeyCombo(title, "ZoomOut", Modules::Dialogue::ZoomOut.enabled, Toggle::selected_device,
                             Modules::Dialogue::ZoomOut.keymap);
        ImGuiMCP::Checkbox((std::string("AutoToggle") + "##" + title).c_str(), &Modules::Dialogue::AutoToggle.enabled);
        ImGuiMCP::SameLine();
        ImGuiMCP::SetCursorPosX(170);
        ImGuiMCP::Text("");
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox((std::string("Invert") + "##" + title).c_str(), &Modules::Dialogue::AutoToggle.invert);
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox((std::string("Revert") + "##" + title).c_str(), &Modules::Dialogue::AutoToggle.revert);
        ImGuiMCP::SameLine();
        HelpMarker("Default is from 3rd to 1st.");

        ImGuiMCP::Checkbox(std::format("DisallowZoomPOVSwitch##{}", title).c_str(),
                           &Modules::Dialogue::DisallowZoomPOVSwitch.enabled);

        RenderZoomLvL("Dialogue", "Toggle", Modules::Dialogue::Toggle, true);
    }
}

void MCP::Dialogue::RenderEnableDisableAll() {
    if (ImGuiMCP::Button("Enable All##Dialogue")) {
        Modules::Dialogue::Toggle.enabled = true;
        Modules::Dialogue::ZoomEnable.enabled = true;
        Modules::Dialogue::ZoomIn.enabled = true;
        Modules::Dialogue::ZoomOut.enabled = true;
        Modules::Dialogue::AutoToggle.enabled = true;
        Modules::Dialogue::DisallowZoomPOVSwitch.enabled = true;
    }
    ImGuiMCP::SameLine();
    if (ImGuiMCP::Button("Disable All##Dialogue")) {
        Modules::Dialogue::Toggle.enabled = false;
        Modules::Dialogue::ZoomEnable.enabled = false;
        Modules::Dialogue::ZoomIn.enabled = false;
        Modules::Dialogue::ZoomOut.enabled = false;
        Modules::Dialogue::AutoToggle.enabled = false;
        Modules::Dialogue::DisallowZoomPOVSwitch.enabled = false;
    }
};

void MCP::Combat::Render() {
    const std::string title = "Combat";
    if (ImGuiMCP::CollapsingHeader((title + "##Settings").c_str(), ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderEnableDisableAll();
        HelpMarker("Default is from 1st to 3rd.");
        ImGuiMCP::SameLine();
        __Render(ToggleCombat, title, "ToggleCombatEnter");
        HelpMarker("Default is from 1st to 3rd.");
        ImGuiMCP::SameLine();
        __Render(ToggleWeapon, title, "ToggleWeaponDraw");
        HelpMarker("Default is from 3rd to 1st.");
        ImGuiMCP::SameLine();
        __Render(ToggleBowDraw, title, "ToggleBowDraw");
        HelpMarker("Default is from 3rd to 1st.");
        ImGuiMCP::SameLine();
        __Render(ToggleMagicWield, title, "ToggleMagicWield");
        ImGuiMCP::SameLine();
        __RenderIgnoreSpell(ToggleMagicWield, "ToggleMagicWield", selected_delivery_wield);
        HelpMarker("Default is from 3rd to 1st.");
        ImGuiMCP::SameLine();
        __Render(ToggleMagicCast, title, "ToggleMagicCast");
        ImGuiMCP::SameLine();
        __RenderIgnoreSpell(ToggleMagicCast, "ToggleMagicCast", selected_delivery_cast);
        HelpMarker("Default is from 1st to 3rd.");
        ImGuiMCP::SameLine();
        __Render(ToggleSneak, title, "ToggleSneak");
    }
}

void MCP::Combat::__Render(Feature& feat, const std::string& title, const std::string& label) {
    ImGuiMCP::Checkbox((label + "##" + title).c_str(), &feat.enabled);
    ImGuiMCP::SameLine();
    ImGuiMCP::SetCursorPosX(250);
    ImGuiMCP::Text("");
    ImGuiMCP::SameLine();
    ImGuiMCP::Checkbox((std::string("Invert") + "##" + title + label).c_str(), &feat.invert);
    ImGuiMCP::SameLine();
    ImGuiMCP::Checkbox((std::string("Revert") + "##" + title + label).c_str(), &feat.revert);
    ImGuiMCP::SameLine();
    ImGuiMCP::Checkbox((std::string("Instant") + "##" + title + label).c_str(), &feat.instant);
    ImGuiMCP::SameLine();
    RenderZoomLvL(title, label, feat, false);
}

void MCP::Combat::__RenderIgnoreSpell(Feature& combat_magic_feat, const std::string& label, int& selected_delivery) {
    bool ignore = combat_magic_feat.keymap[selected_delivery] > 0;
    bool both = combat_magic_feat.keymap[selected_delivery] == 2;
    ImGuiMCP::Checkbox(("Ignore##SpellWithDelivery" + label).c_str(), &ignore);
    ImGuiMCP::SameLine();
    const std::string& temp_str = Utilities::kDelivery2Char(selected_delivery);
    const ImGuiMCP::ImVec2 textSize = ImGuiMCP::CalcTextSize(temp_str.c_str(), nullptr, false, -1.0f);
    ImGuiMCP::SetNextItemWidth(textSize.x + 55);
    int new_delivery = selected_delivery;
    if (ImGuiMCP::BeginCombo(("##SpellDelivery" + label).c_str(), temp_str.c_str())) {
        for (int n = 0; n < 5; ++n) {
            const bool is_selected = selected_delivery == n;
            if (ImGuiMCP::Selectable((Utilities::kDelivery2Char(n) + "##" + label).c_str(), is_selected)) {
                new_delivery = n;
            }
            if (is_selected) ImGuiMCP::SetItemDefaultFocus();
        }
        ImGuiMCP::EndCombo();
    }
    if (ignore) {
        ImGuiMCP::SameLine();
        ImGuiMCP::Checkbox(("Both##Hands" + label).c_str(), &both);
    }
    ImGuiMCP::SameLine();
    HelpMarker(
        "Spell with the selected delivery will be ignored, optionally only when equipped on both hands. To see the both hands option, enable Ignore.");
    combat_magic_feat.keymap[selected_delivery] = ignore ? both ? 2 : 1 : 0;
    selected_delivery = new_delivery;
}

void MCP::Combat::RenderEnableDisableAll() {
    if (ImGuiMCP::Button("Enable All##Combat")) {
        ToggleCombat.enabled = true;
        ToggleWeapon.enabled = true;
        ToggleBowDraw.enabled = true;
        ToggleMagicWield.enabled = true;
        ToggleMagicCast.enabled = true;
        ToggleSneak.enabled = true;
    }
    ImGuiMCP::SameLine();
    if (ImGuiMCP::Button("Disable All##Combat")) {
        ToggleCombat.enabled = false;
        ToggleWeapon.enabled = false;
        ToggleBowDraw.enabled = false;
        ToggleMagicWield.enabled = false;
        ToggleMagicCast.enabled = false;
        ToggleSneak.enabled = false;
    }
};

void MCP::Other::Render() {
    const std::string title = "Other";
    if (ImGuiMCP::CollapsingHeader((title + "##Settings").c_str(), ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderEnableDisableAll();

        __Render(ToggleCellChangeExterior.enabled, ToggleCellChangeExterior.invert, title, "ToggleCellChangeExterior");
        ImGuiMCP::SameLine();
        HelpMarker("Default is from 1st to 3rd.");

        __Render(ToggleCellChangeInterior.enabled, ToggleCellChangeInterior.invert, title, "ToggleCellChangeInterior");
        ImGuiMCP::SameLine();
        HelpMarker("Default is from 3rd to 1st.");

        RenderZoomLvL("Other", "FixZoom", FixZoom, false);
        ImGuiMCP::SameLine();
        HelpMarker("Upon transitioning into 3rd person, the selected zoom level will be applied.");
    }
}

void MCP::Other::__Render(bool& enabled, bool& invert, const std::string& title,
                          const std::string& label) {
    RenderCheckBox(title, label, enabled);
    ImGuiMCP::SameLine();
    ImGuiMCP::SetCursorPosX(270);
    ImGuiMCP::Text("");
    ImGuiMCP::SameLine();
    ImGuiMCP::Checkbox((std::string("Invert") + "##" + title + label).c_str(), &invert);
    /*ImGui::SameLine();
    ImGui::Checkbox((std::string("Revert") + "##" + title + label).c_str(), &revert);*/
    //ImGui::SameLine();
    //ImGui::Checkbox((std::string("Instant") + "##" + title + label).c_str(), &instant);
}

void MCP::Other::RenderEnableDisableAll() {
    if (ImGuiMCP::Button("Enable All##Other")) {
        ToggleCellChangeExterior.enabled = true;
        ToggleCellChangeInterior.enabled = true;
        FixZoom.fix_zoom.enabled = true;
    }
    ImGuiMCP::SameLine();
    if (ImGuiMCP::Button("Disable All##Other")) {
        ToggleCellChangeExterior.enabled = false;
        ToggleCellChangeInterior.enabled = false;
        FixZoom.fix_zoom.enabled = false;
    }
};

void HelpMarker(const char* desc) {
    ImGuiMCP::TextDisabled("(?)");
    if (ImGuiMCP::BeginItemTooltip()) {
        ImGuiMCP::PushTextWrapPos(ImGuiMCP::GetFontSize() * 35.0f);
        ImGuiMCP::TextUnformatted(desc);
        ImGuiMCP::PopTextWrapPos();
        ImGuiMCP::EndTooltip();
    }
}
