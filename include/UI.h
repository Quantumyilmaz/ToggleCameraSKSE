#pragma once

#include "Hooks.h"

void HelpMarker(const char* desc);

namespace MCP {
    inline std::string log_path = Utilities::GetLogPath().string();
    inline std::vector<std::string> logLines;

    inline int detected_key = -1;
    inline int detected_device = -1;
    inline bool listen_key = false;

    void Register();

    void __stdcall RenderSettings();
    //void __stdcall RenderStatus();
    void __stdcall RenderLog();

    void RenderCheckBox(const std::string& title, const std::string& label, bool& enabled);
    void RenderDeviceKeyCombo(const std::string& title, const std::string& label, bool& enabled, int& selected_device,
                              std::map<int, int>& keymap);
    void RenderZoomLvL(const std::string& title, const std::string& label, Feature& feat, bool show_instant = false);

    namespace Dialogue {
        void Render();

        namespace Toggle {
            inline int selected_device = 0;
        };

        void RenderEnableDisableAll();
    };

    namespace Combat {
        using namespace Modules::Combat;

        inline int selected_delivery_wield = 0;
        inline int selected_delivery_cast = 0;

        void Render();
        void __Render(Feature& feat, const std::string& title, const std::string& label);
        void __RenderIgnoreSpell(Feature& combat_magic_feat, const std::string& label, int& selected_delivery);
        void RenderEnableDisableAll();
    };

    namespace Other {
        using namespace Modules::Other;
        void Render();
        void __Render(bool& enabled, bool& invert, const std::string& title,
                      const std::string& label);
        void RenderEnableDisableAll();
    };
};