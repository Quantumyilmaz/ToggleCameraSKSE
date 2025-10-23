#pragma once
#include "Serialization.h"


namespace Dialogue {
    const uint8_t n_hooks = 1;
    const size_t trampoline_size = n_hooks * 14;

    namespace OnCameraUpdate {
        static void thunk(RE::TESCamera* a_camera);
        static inline REL::Relocation<decltype(thunk)> func;
    };

    void InstallHooks();
};

namespace Combat {
    using namespace Modules::Combat;
    using namespace Utilities;

	const uint8_t n_hooks = 1;
	const size_t trampoline_size = n_hooks * 14;
    inline int spell_delivery_L=-1;
    inline int spell_delivery_R=-1;

    bool IsMagicEquipped();

    bool IsCasting();

	namespace OnActorUpdate {
        static void thunk(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell);
		static inline REL::Relocation<decltype(thunk)> func;

        bool OnKillmove(const RE::Actor* a_actor);
        bool OnWeaponDraw(RE::Actor* a_actor);
        bool OnSneak(const RE::Actor* a_actor);
        bool OnBowDraw(RE::Actor* a_actor);
        bool OnMagicDraw(RE::Actor* a_actor);
        bool OnMagicCast(RE::Actor* a_actor);

        inline bool sneaked = false;
	};

	void InstallHooks();
};

namespace Hooks {
    void Install();
};