#pragma once
#include <Utils.h>


//kKeyboard = 0, 
//kMouse=1, 
//kGamepad=2,

constexpr std::array<int, 3> SupportedDevices{0, 1, 2};
const std::array<std::string, 3> device_names{"Keyboard", "Mouse", "Gamepad"};

enum Purpose {
    kNone,

    // Dialogue
    kToggle,
    kZoomEnable,
    kZoomIn,
    kZoomOut,
};

struct FixedZoom {
    float zoom_lvl = 0.2f;
    bool enabled = false;
    bool instant = false;
};

struct Feature {
    //std::string name;
    bool enabled = false;
    bool instant = false;
    bool invert = false;
    bool revert = true;

    FixedZoom fix_zoom;

    Purpose purpose = kNone;
    std::map<int, int> keymap;

    Feature() {
        for (int i = 0; i < SupportedDevices.size(); ++i) {
            keymap[i] = -1;
        }
    };

    operator bool() const {
        return enabled;
    };

    void to_json(rapidjson::Value& j, rapidjson::Document::AllocatorType& a) const;

    void from_json(const rapidjson::Value& j);
};

namespace Modules {
    using namespace rapidjson;

    namespace Dialogue {
        // Features
        inline Feature Toggle;
        inline Feature ZoomEnable;
        inline Feature ZoomIn;
        inline Feature ZoomOut;
        inline Feature AutoToggle;
        inline Feature DisallowZoomPOVSwitch;

        inline bool listen_gradual_zoom = false;
        inline bool listen_auto_zoom = true;
        inline bool zoom_enabled = false;
        inline std::optional<float> preDialogueZoomOffset;

        void funcToggle();
        void funcZoom(int a_device, bool _in);

        Purpose GetPurpose(int a_device, int keyMask);

        Value to_json(Document::AllocatorType& a);
        void from_json(const Value& j);
        void LoadFeatures();
    };

    namespace Combat {
        // Features
        inline Feature ToggleCombat;
        inline Feature ToggleWeapon;
        inline Feature ToggleBowDraw;
        inline Feature ToggleMagicWield;
        inline Feature ToggleMagicCast;
        inline Feature ToggleSneak;

        inline bool listen_gradual_zoom = false;
        inline float savedZoomOffset = 0.2f;

        // combat trigger stuff
        inline uint32_t oldstate_c = 0;
        // weapon draw stuff
        inline uint32_t oldstate_w = 0;
        // magic stuff
        inline uint32_t oldstate_m = 0;

        // bunlara priority veriyom
        inline bool bow_cam_switched = false;
        inline bool casting_switched = false;

        bool Is3rdP();

        void funcToggle(Feature& feat);

        uint32_t CamSwitchHandling(uint32_t newstate, bool third2first, bool switch_back);

        Value to_json(Document::AllocatorType& a);
        void from_json(const Value& j);
        void LoadFeatures();
    };

    namespace Other {
        // Features
        inline Feature ToggleCellChangeExterior;
        inline Feature ToggleCellChangeInterior;
        inline Feature FixZoom;
        //inline Feature FixZoomHardcore;

        void funcToggle(bool is3rdP, float extra_offset = 0.f);

        inline bool is_exterior = false;

        Value to_json(Document::AllocatorType& a);
        void from_json(const Value& j);
        void LoadFeatures();
    };
};


namespace Settings {
    constexpr std::uint32_t kDataKey = 'TCSE';
    const std::string path = std::format("Data/SKSE/Plugins/{}/Settings.json", Utilities::mod_name);
    void LoadDefaults();
    void LoadSettings();
    void SaveSettings();
};


namespace LogSettings {
    inline bool log_trace = true;
    inline bool log_info = true;
    inline bool log_warning = true;
    inline bool log_error = true;
};
