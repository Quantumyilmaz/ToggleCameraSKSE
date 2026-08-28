#pragma once

#include "Serialization.h"


namespace Dialogue {
    constexpr uint8_t n_hooks = 1;
    constexpr size_t trampoline_size = n_hooks * 14;

    namespace OnCameraUpdate {
        static void thunk(RE::TESCamera* a_camera);
        static inline REL::Relocation<decltype(thunk)> func;
    };

    void InstallHooks();
};

namespace Combat {
    using namespace Modules::Combat;
    using namespace Utilities;

    constexpr uint8_t n_hooks = 1;
    constexpr size_t trampoline_size = n_hooks * 14;
    inline int spell_delivery_L = -1;
    inline int spell_delivery_R = -1;

    bool IsMagicEquipped();

    bool IsCasting();

    namespace OnActorUpdate {
        static void thunk(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell);
        static inline REL::Relocation<decltype(thunk)> func;

        bool __Killmove(RE::Actor* a_actor);
        bool __WeaponDraw(RE::Actor* a_actor);
        bool __Sneak(RE::Actor* a_actor);
        bool __BowDraw(RE::Actor* a_actor);
        bool __MagicDraw(RE::Actor* a_actor);
        bool __MagicCast(RE::Actor* a_actor);

        inline bool sneaked = false;
    };

    void InstallHooks();
};

namespace Hooks {
    void Install();
};