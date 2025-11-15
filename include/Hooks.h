#pragma once
#include "Serialization.h"

namespace Combat {
    using namespace Modules::Combat;
    using namespace Utilities;

    inline int spell_delivery_L=-1;
    inline int spell_delivery_R=-1;

    bool IsMagicEquipped();

    bool IsCasting();

	namespace OnActorUpdate {
        bool OnKillmove(const RE::Actor* a_actor);
        bool OnWeaponDraw(RE::Actor* a_actor);
        bool OnSneak(const RE::Actor* a_actor);
        bool OnBowDraw(RE::Actor* a_actor);
        bool OnMagicDraw(RE::Actor* a_actor);
        bool OnMagicCast(RE::Actor* a_actor);

        inline bool sneaked = false;
	};
};

namespace Hooks {

    struct PlayerUpdateHook
    {
        static void Install();
        static void Update(RE::PlayerCharacter* a_this, float a_delta);
        static inline REL::Relocation<decltype(Update)> _Update;
    };

    void Install();
};