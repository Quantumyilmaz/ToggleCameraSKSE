#include "UI.h"
#include "SKSEMCP/SKSEMenuFramework.hpp"

void __stdcall MCP::RenderLog() {
    // add checkboxes to filter log levels
    ImGui::Checkbox("Trace", &LogSettings::log_trace);
    ImGui::SameLine();
    ImGui::Checkbox("Info", &LogSettings::log_info);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &LogSettings::log_warning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &LogSettings::log_error);

    // if "Generate Log" button is pressed, read the log file
    if (ImGui::Button("Generate Log")) logLines = Utilities::ReadLogFile();

    // Display each line in a new ImGui::Text() element
    for (const auto& line : logLines) {
        if (line.find("trace") != std::string::npos && !LogSettings::log_trace) continue;
        if (line.find("info") != std::string::npos && !LogSettings::log_info) continue;
        if (line.find("warning") != std::string::npos && !LogSettings::log_warning) continue;
        if (line.find("error") != std::string::npos && !LogSettings::log_error) continue;
        ImGui::Text(line.c_str());
    }
}

void MCP::RenderCheckBox(const std::string& title, const std::string& label, bool& enabled) {
    ImGui::Checkbox((label + "##" + title).c_str(), &enabled);
}

void MCP::RenderDeviceKeyCombo(const std::string& title, const std::string& label, bool& enabled, const int& selected_device,
                               std::map<int, int>& keymap) {
    
    
    ImGui::Checkbox((label + ":##" + title).c_str(), &enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(170);
    ImGui::Text("");
    ImGui::SameLine();
    ImGui::Text(device_names[selected_device].c_str());
    ImGui::SameLine();
    ImGui::Text("Key");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
        
    std::string combo_label = "##key_combo_" + std::to_string(selected_device) + label;  // Ensure unique ID for each combo
    if (ImGui::BeginCombo(combo_label.c_str(), std::to_string(keymap[selected_device]).c_str())) {
        for (int n = -1; n < 600; ++n) {
            const bool is_selected = keymap[selected_device] == n;
            if (ImGui::Selectable(std::to_string(n).c_str(), is_selected)) keymap[selected_device] = n;
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}

void MCP::RenderZoomLvL(const std::string& title, const std::string& label, Feature& feat) {
    RenderCheckBox(title + label, "FixZoom", feat.fix_zoom.enabled);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    float& zoom_lvl = feat.fix_zoom.zoom_lvl;
    if (ImGui::BeginCombo(("##FixZoomValue" + label).c_str(), Utilities::formatFloatToString(zoom_lvl, 1).c_str())) {
        for (int n = -1; n < 11; ++n) {
            const bool is_selected = std::abs(zoom_lvl - n / 10.f) < 0.0000001f;
            if (ImGui::Selectable(Utilities::formatFloatToString(n / 10.f, 1).c_str(), is_selected)) zoom_lvl = n / 10.f;
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
};

void MCP::Register(){
    if (!SKSEMenuFramework::IsInstalled()) {
        logger::critical("SKSE Menu Framework is not installed. Cannot register menu.");
        return;
    }
    SKSEMenuFramework::SetSection(Utilities::mod_name);
    SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);
    //SKSEMenuFramework::AddSectionItem("Status", RenderStatus);
    SKSEMenuFramework::AddSectionItem("Log", RenderLog);
}

void __stdcall MCP::RenderSettings(){
    // add a save button
    if (ImGui::Button("Save Settings")) {
		Settings::SaveSettings();
	}

    ImGui::SameLine();

    if (ImGui::Button("Load Settings")) {
        Settings::LoadSettings();
    }

    ImGui::SameLine();

    // Key Detection
    if (ImGui::Button("Start Key Detection")) {
        detected_key = -1;
        detected_device = -1;
        listen_key = true;
    }
    if (listen_key) {
        ImGui::SameLine();
        ImGui::Text("Listening for key press...");
    }
    if (detected_key >= 0) {
        ImGui::SameLine();
        ImGui::Text("Detected Key: %d, Device: %s", detected_key, device_names[detected_device].c_str());
    }
    // help marker
    ImGui::SameLine();
    HelpMarker("Click 'Start',close this menu and press a key to detect it. You can view the detected key here.");
    Dialogue::Render();
    Combat::Render();
    Other::Render();
};

void MCP::Dialogue::Render(){

    std::string title = "Dialogue";
    if (ImGui::CollapsingHeader((title + "##Settings").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderEnableDisableAll();
        ImGui::SameLine();
        ImGui::Text("Device Selection: ");
        ImGui::SameLine();
        auto& selected_device = MCP::Dialogue::Toggle::selected_device;
        std::string label = "Toggle";
        ImGui::SetNextItemWidth(200);
        if (ImGui::BeginCombo(("##device_combo" + title + label).c_str(), device_names[selected_device].c_str(),
                              ImGuiComboFlags_HeightSmall)) {
            for (int n = 0; n < device_names.size(); ++n) {
                const bool is_selected = (selected_device == n);
                if (ImGui::Selectable(device_names[n].c_str(), is_selected)) {
                    selected_device = n;
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        RenderDeviceKeyCombo(title,"Toggle",Modules::Dialogue::Toggle.enabled,Dialogue::Toggle::selected_device,Modules::Dialogue::Toggle.keymap);
        ImGui::SameLine();
        ImGui::Checkbox((std::string("InstantZoom") + "##" + title).c_str(), &Modules::Dialogue::Toggle.instant);
        RenderDeviceKeyCombo(title, "ZoomEnabler", Modules::Dialogue::ZoomEnable.enabled,Dialogue::Toggle::selected_device, Modules::Dialogue::ZoomEnable.keymap);
        RenderDeviceKeyCombo(title, "ZoomIn", Modules::Dialogue::ZoomIn.enabled, Dialogue::Toggle::selected_device,Modules::Dialogue::ZoomIn.keymap);
        RenderDeviceKeyCombo(title, "ZoomOut", Modules::Dialogue::ZoomOut.enabled, Dialogue::Toggle::selected_device,
                             Modules::Dialogue::ZoomOut.keymap);
        ImGui::Checkbox((std::string("AutoToggle") + "##" + title).c_str(), &Modules::Dialogue::AutoToggle.enabled);
        ImGui::SameLine();
        ImGui::SetCursorPosX(170);
        ImGui::Text("");
        ImGui::SameLine();
        ImGui::Checkbox((std::string("Invert") + "##" + title).c_str(), &Modules::Dialogue::AutoToggle.invert);
        ImGui::SameLine();
        ImGui::Checkbox((std::string("Revert") + "##" + title).c_str(), &Modules::Dialogue::AutoToggle.revert);
        ImGui::SameLine();
        HelpMarker("Default is from 3rd to 1st.");

        ImGui::Checkbox(std::format("DisallowZoomPOVSwitch##{}",title).c_str(), &Modules::Dialogue::DisallowZoomPOVSwitch.enabled);

        RenderZoomLvL("Dialogue", "Toggle", Modules::Dialogue::Toggle);
    }
    
}
void MCP::Dialogue::RenderEnableDisableAll(){
    if (ImGui::Button("Enable All##Dialogue")) {
		Modules::Dialogue::Toggle.enabled = true;
		Modules::Dialogue::ZoomEnable.enabled = true;
		Modules::Dialogue::ZoomIn.enabled = true;
		Modules::Dialogue::ZoomOut.enabled = true;
		Modules::Dialogue::AutoToggle.enabled = true;
        Modules::Dialogue::DisallowZoomPOVSwitch.enabled = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Disable All##Dialogue")) {
		Modules::Dialogue::Toggle.enabled = false;
		Modules::Dialogue::ZoomEnable.enabled = false;
		Modules::Dialogue::ZoomIn.enabled = false;
		Modules::Dialogue::ZoomOut.enabled = false;
		Modules::Dialogue::AutoToggle.enabled = false;
        Modules::Dialogue::DisallowZoomPOVSwitch.enabled = false;
	}
};

void MCP::Combat::Render() {

    std::string title = "Combat";
    if (ImGui::CollapsingHeader((title + "##Settings").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderEnableDisableAll();
        HelpMarker("Default is from 1st to 3rd.");
        ImGui::SameLine();
        __Render(ToggleCombat,title,"ToggleCombatEnter");
        HelpMarker("Default is from 1st to 3rd.");
        ImGui::SameLine();
        __Render(ToggleWeapon,title,"ToggleWeaponDraw");
        HelpMarker("Default is from 3rd to 1st.");
        ImGui::SameLine();
        __Render(ToggleBowDraw, title,"ToggleBowDraw");
        HelpMarker("Default is from 3rd to 1st.");
        ImGui::SameLine();
        __Render(ToggleMagicWield,title,"ToggleMagicWield");
        ImGui::SameLine();
        __RenderIgnoreSpell(ToggleMagicWield, "ToggleMagicWield",selected_delivery_wield);
        HelpMarker("Default is from 3rd to 1st.");
        ImGui::SameLine();
        __Render(ToggleMagicCast,title,"ToggleMagicCast");
        ImGui::SameLine();
        __RenderIgnoreSpell(ToggleMagicCast, "ToggleMagicCast",selected_delivery_cast);
        HelpMarker("Default is from 1st to 3rd.");
        ImGui::SameLine();
        __Render(ToggleSneak,title,"ToggleSneak");
    }
}

void MCP::Combat::__Render(Feature& feat, const std::string& title, const std::string& label) {
    ImGui::Checkbox((label + "##" + title).c_str(), &feat.enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(250);
    ImGui::Text("");
    ImGui::SameLine();
    ImGui::Checkbox((std::string("Invert") + "##" + title + label).c_str(), &feat.invert);
    ImGui::SameLine();
    ImGui::Checkbox((std::string("Revert") + "##" + title + label).c_str(), &feat.revert);
    ImGui::SameLine();
    ImGui::Checkbox((std::string("Instant") + "##" + title + label).c_str(), &feat.instant);
    ImGui::SameLine();
    RenderZoomLvL(title, label, feat);
}

void MCP::Combat::__RenderIgnoreSpell(Feature& combat_magic_feat, const std::string& label, int& selected_delivery) {
    bool ignore = combat_magic_feat.keymap[selected_delivery] > 0;
    bool both = combat_magic_feat.keymap[selected_delivery] == 2;
    ImGui::Checkbox(("Ignore##SpellWithDelivery" + label).c_str(), &ignore);
    ImGui::SameLine();
    const std::string& temp_str = Utilities::kDelivery2Char(selected_delivery);
    ImVec2 textSize;
    ImGui::CalcTextSize(&textSize, temp_str.c_str(), nullptr, false, -1.0f);
    ImGui::SetNextItemWidth(textSize.x+55);
    int new_delivery = selected_delivery;
    if (ImGui::BeginCombo(("##SpellDelivery" + label).c_str(), temp_str.c_str())) {
        for (int n = 0; n < 5; ++n) {
            const bool is_selected = selected_delivery == n;
            if (ImGui::Selectable((Utilities::kDelivery2Char(n) + "##" + label).c_str(), is_selected)) {
                new_delivery = n;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (ignore) {
        ImGui::SameLine();
        ImGui::Checkbox(("Both##Hands" + label).c_str(), &both);
    }
    ImGui::SameLine();
    HelpMarker("Spell with the selected delivery will be ignored, optionally only when equipped on both hands. To see the both hands option, enable Ignore.");
    combat_magic_feat.keymap[selected_delivery] = ignore ? both ? 2 : 1  : 0;
    selected_delivery = new_delivery;

}

void MCP::Combat::RenderEnableDisableAll() {
    if (ImGui::Button("Enable All##Combat")) {
		ToggleCombat.enabled = true;
		ToggleWeapon.enabled = true;
		ToggleBowDraw.enabled = true;
		ToggleMagicWield.enabled = true;
		ToggleMagicCast.enabled = true;
		ToggleSneak.enabled = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Disable All##Combat")) {
		ToggleCombat.enabled = false;
		ToggleWeapon.enabled = false;
		ToggleBowDraw.enabled = false;
		ToggleMagicWield.enabled = false;
		ToggleMagicCast.enabled = false;
		ToggleSneak.enabled = false;
	}
};

void MCP::Other::Render(){
    std::string title = "Other";
	if (ImGui::CollapsingHeader((title + "##Settings").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		RenderEnableDisableAll();
        
        __Render(ToggleCellChangeExterior.enabled, ToggleCellChangeExterior.invert, title, "ToggleCellChangeExterior");
        ImGui::SameLine();
        HelpMarker("Default is from 1st to 3rd.");
        
        __Render(ToggleCellChangeInterior.enabled, ToggleCellChangeInterior.invert, title, "ToggleCellChangeInterior");
        ImGui::SameLine();
        HelpMarker("Default is from 3rd to 1st.");

        RenderZoomLvL("Other", "FixZoom", FixZoom);
        ImGui::SameLine();
        HelpMarker("Upon transitioning into 3rd person, the selected zoom level will be applied.");
	}
}

void MCP::Other::__Render(bool& enabled, bool& invert, const std::string& title,
                          const std::string& label){
    RenderCheckBox(title, label, enabled);
    ImGui::SameLine();
    ImGui::SetCursorPosX(270);
    ImGui::Text("");
    ImGui::SameLine();
    ImGui::Checkbox((std::string("Invert") + "##" + title+label).c_str(), &invert);
    /*ImGui::SameLine();
    ImGui::Checkbox((std::string("Revert") + "##" + title + label).c_str(), &revert);*/
    //ImGui::SameLine();
    //ImGui::Checkbox((std::string("Instant") + "##" + title + label).c_str(), &instant);

}
void MCP::Other::RenderEnableDisableAll(){
    if (ImGui::Button("Enable All##Other")) {
        ToggleCellChangeExterior.enabled = true;
        ToggleCellChangeInterior.enabled = true;
        FixZoom.fix_zoom.enabled = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Disable All##Other")) {
        ToggleCellChangeExterior.enabled = false;
        ToggleCellChangeInterior.enabled = false;
        FixZoom.fix_zoom.enabled = false;
	}
};

void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
