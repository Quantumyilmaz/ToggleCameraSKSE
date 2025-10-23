#include "Hooks.h"


void Dialogue::OnCameraUpdate::thunk(RE::TESCamera* a_camera){
	func(a_camera);
    if (!Modules::Dialogue::listen_gradual_zoom && !Modules::Combat::listen_gradual_zoom) return;
    if (auto* thirdPersonState = static_cast<RE::ThirdPersonState*>(a_camera->currentState.get());
        thirdPersonState &&
        thirdPersonState->currentZoomOffset < -0.19f) {
        Modules::Dialogue::listen_gradual_zoom = false;
        Modules::Combat::listen_gradual_zoom = false;
        RE::PlayerCamera::GetSingleton()->ForceFirstPerson();
        Modules::Dialogue::listen_auto_zoom = true;
    }
	
};

void Dialogue::InstallHooks() {
    
    auto& trampoline = SKSE::GetTrampoline();

    REL::Relocation<std::uintptr_t> camFunction{REL::RelocationID(49852, 50784)};
    OnCameraUpdate::func = trampoline.write_call<5>(camFunction.address() + REL::Relocate(0x1A6, 0x1A6), OnCameraUpdate::thunk);

};

void Hooks::Install(){

    // currently only hook is dialogue
    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(Dialogue::trampoline_size + Combat::trampoline_size);

    Dialogue::InstallHooks();
    Combat::InstallHooks();
};

void Combat::OnActorUpdate::thunk(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell) {
    if (auto* plyr_chr = RE::PlayerCharacter::GetSingleton(); 
        !a_actor || 
        plyr_chr->GetGameStatsData().byCharGenFlag.any(RE::PlayerCharacter::ByCharGenFlag::kHandsBound) ||
        plyr_chr->GetFormID() != a_actor->GetFormID() || 
        //Utilities::IsVampireLord(plyr_chr) ||
        Utilities::IsWerewolf(plyr_chr)) {
        return func(a_actor, a_zPos, a_cell);
    }
    auto plyr_c = RE::PlayerCamera::GetSingleton();
    if (!plyr_c->IsInFirstPerson() && !plyr_c->IsInThirdPerson()) return func(a_actor, a_zPos, a_cell);
    auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(plyr_c->cameraStates[RE::CameraState::kThirdPerson].get());
    if (plyr_c->IsInThirdPerson() && thirdPersonState->currentZoomOffset == thirdPersonState->targetZoomOffset &&
        savedZoomOffset != thirdPersonState->currentZoomOffset) {
        savedZoomOffset = thirdPersonState->currentZoomOffset;
        thirdPersonState->savedZoomOffset = savedZoomOffset;
    }

    //logger::trace("currentZoomOffset: {}, targetZoomOffset: {}, savedZoomOffset: {}, pitch: {}",
    //              thirdPersonState->currentZoomOffset, thirdPersonState->targetZoomOffset,
    //              thirdPersonState->savedZoomOffset, thirdPersonState->pitchZoomOffset);


    // killmove handling
    if (!OnKillmove(a_actor)) {
        logger::trace("Killmove detected.");
        return func(a_actor, a_zPos, a_cell);
    }

    int shouldToggleWeapon = 0;
    int shouldToggleCombat = 0;

    // weapon draw handling
    if (ToggleWeapon && (!IsMagicEquipped() || !ToggleMagicWield) && OnWeaponDraw(a_actor)) {
        logger::trace("Weapon draw detected. Should toggle.");
        shouldToggleWeapon += 1;
    }

    // combat handling
    if (ToggleCombat && GetCombatState() != oldstate_c && !bow_cam_switched && !casting_switched) {
        oldstate_c = oldstate_c != 0 ? 0 : 1;
        const auto temp = CamSwitchHandling(oldstate_c, ToggleCombat.invert, ToggleCombat.revert);
        if (temp > 0) {
            shouldToggleCombat += temp;
            logger::trace("Combat detected. Should toggle.");
        }
    }

    // sneak handling
    if (ToggleSneak && !OnSneak(a_actor)) {
		logger::trace("Sneak detected. Toggled.");
		return func(a_actor, a_zPos, a_cell);
	}

    // bow first person aiming handling
    if (ToggleBowDraw && !OnBowDraw(a_actor)) {
        logger::trace("Bow draw detected. Toggled.");
        return func(a_actor, a_zPos, a_cell);
    }

    // magic draw and casting handling
    if (IsMagicEquipped()) {

        
        // magic draw handling
        bool ignore_L = ToggleMagicWield.keymap[spell_delivery_L] > 0;
        bool both_hands_L = ToggleMagicWield.keymap[spell_delivery_L] == 2;
        bool ignore_R = ToggleMagicWield.keymap[spell_delivery_R] > 0;
        bool both_hands_R = ToggleMagicWield.keymap[spell_delivery_R] == 2;
        ignore_L = both_hands_L ? ignore_L && spell_delivery_L == spell_delivery_R : ignore_L;
        ignore_R = both_hands_R ? ignore_R && spell_delivery_L == spell_delivery_R : ignore_R;
        bool ignore = ignore_L || ignore_R;
        if (ToggleMagicWield && !ignore && !OnMagicDraw(a_actor)) {
            logger::trace("Magic draw detected. Toggled.");
            return func(a_actor, a_zPos, a_cell);
        }

        // magic casting handling
        ignore_L = ToggleMagicCast.keymap[spell_delivery_L] > 0;
        both_hands_L = ToggleMagicCast.keymap[spell_delivery_L] == 2;
        ignore_R = ToggleMagicCast.keymap[spell_delivery_R] > 0;
        both_hands_R = ToggleMagicCast.keymap[spell_delivery_R] == 2;
        ignore_L = both_hands_L ? ignore_L && spell_delivery_L == spell_delivery_R : ignore_L;
        ignore_R = both_hands_R ? ignore_R && spell_delivery_L == spell_delivery_R : ignore_R;
        ignore = ignore_L || ignore_R;
        if (ToggleMagicCast && !ignore  && !OnMagicCast(a_actor)) {
            logger::trace("Magic cast detected. Toggled.");
            return func(a_actor, a_zPos, a_cell);
        }
    }

    if (shouldToggleWeapon) funcToggle(ToggleWeapon);
    else if (shouldToggleCombat) funcToggle(ToggleCombat);

    return func(a_actor, a_zPos, a_cell);
}

bool Combat::OnActorUpdate::OnKillmove(const RE::Actor* a_actor) {
    if (a_actor->IsInKillMove()) {
        oldstate_c = 1;
        return false;
    } else if (RE::PlayerCamera::GetSingleton()->IsInBleedoutMode()) {
        return false;
    }
    return true;
}

bool Combat::OnActorUpdate::OnSneak(const RE::Actor* a_actor) {
    const auto is_sneaking = a_actor->IsSneaking();
    if (is_sneaking == sneaked) return true;
    sneaked = is_sneaking;
    bool is_3rd_p = Is3rdP();
    if (bool player_is_in_toggled_cam = ToggleSneak.invert ? !is_3rd_p : is_3rd_p;
		is_sneaking && !player_is_in_toggled_cam) {
		funcToggle(ToggleSneak);
		return false;
	} else if (!is_sneaking && player_is_in_toggled_cam) {
		funcToggle(ToggleSneak);
		return false;
	}
	return true;
}

bool Combat::OnActorUpdate::OnWeaponDraw(RE::Actor* a_actor) { 
    auto weapon_state = static_cast<uint32_t>(a_actor->AsActorState()->GetWeaponState());
    if ((!weapon_state || weapon_state == 3) && oldstate_w != weapon_state) {
        oldstate_w = weapon_state;
        return CamSwitchHandling(oldstate_w, ToggleWeapon.invert, ToggleWeapon.revert) > 0;
    }
    return false;
}

bool Combat::OnActorUpdate::OnBowDraw(RE::Actor* a_actor) {
    auto attack_state = static_cast<uint32_t>(a_actor->AsActorState()->GetAttackState());
    bool is_3rd_p = Is3rdP();
    if (bool player_is_in_toggled_cam = ToggleBowDraw.invert ? is_3rd_p : !is_3rd_p;
        !player_is_in_toggled_cam && attack_state == 8) {
        funcToggle(ToggleBowDraw);
        bow_cam_switched = true;
        return false;
    } else if (bow_cam_switched && (!attack_state || attack_state == 13) && player_is_in_toggled_cam && ToggleBowDraw.revert) {
        funcToggle(ToggleBowDraw);
        bow_cam_switched = false;
        return false;
    }
    return true;
}

bool Combat::OnActorUpdate::OnMagicDraw(RE::Actor* a_actor) {
    auto magic_state = static_cast<uint32_t>(a_actor->AsActorState()->GetWeaponState());
    if (oldstate_m != magic_state) {
        bool is_3rd_p = Is3rdP();
        oldstate_m = magic_state;
        if (bool player_is_in_toggled_cam = ToggleMagicWield.invert ? is_3rd_p : !is_3rd_p;
            magic_state == 5 && player_is_in_toggled_cam && ToggleMagicWield.revert) {
            funcToggle(ToggleMagicWield);
            return false;
        } else if (magic_state == 2 && !player_is_in_toggled_cam) {
            funcToggle(ToggleMagicWield);
            return false;
        }
    }
    return true;
}

bool Combat::OnActorUpdate::OnMagicCast(RE::Actor*) {
    bool is_3rd_p = Is3rdP();
    if (bool player_is_in_toggled_cam = ToggleMagicCast.invert ? is_3rd_p : !is_3rd_p;
        !IsCasting() && player_is_in_toggled_cam && ToggleMagicCast.revert && casting_switched) {
        funcToggle(ToggleMagicCast);
        casting_switched = false;
        return false;
    } else if (IsCasting() && !player_is_in_toggled_cam) {
        funcToggle(ToggleMagicCast);
        casting_switched = true;
        return false;
    }
    return true;
}

bool Combat::IsMagicEquipped() {
    auto player_char = RE::PlayerCharacter::GetSingleton();
    auto equipped_obj_L = player_char->GetEquippedObject(true);
    auto equipped_obj_R = player_char->GetEquippedObject(false);
    bool L_is_magic = equipped_obj_L ? equipped_obj_L->IsMagicItem() : false;
    bool R_is_magic = equipped_obj_R ? equipped_obj_R->IsMagicItem() : false;
    spell_delivery_L = (L_is_magic ? static_cast<int>(equipped_obj_L->As<RE::MagicItem>()->GetDelivery()) : -1);
    spell_delivery_R = (R_is_magic ? static_cast<int>(equipped_obj_R->As<RE::MagicItem>()->GetDelivery()) : -1);
    return L_is_magic || R_is_magic;
}

bool Combat::IsCasting() {
    if (!IsMagicEquipped()) return false;
    auto player_char = RE::PlayerCharacter::GetSingleton();
    auto equipped_obj_L = player_char->GetEquippedObject(true);
    auto equipped_obj_R = player_char->GetEquippedObject(false);
    RE::MagicItem* equipped_obj_L_MI = nullptr;
    RE::MagicItem* equipped_obj_R_MI = nullptr;
    if (equipped_obj_L) equipped_obj_L_MI = equipped_obj_L->As<RE::MagicItem>();
    if (equipped_obj_R) equipped_obj_R_MI = equipped_obj_R->As<RE::MagicItem>();
    bool is_casting = false;
    if (equipped_obj_L_MI && player_char->IsCasting(equipped_obj_L_MI)) is_casting = true;
    if (equipped_obj_R_MI && player_char->IsCasting(equipped_obj_R_MI)) is_casting = true;
    return is_casting;
}

void Combat::InstallHooks() {
    auto& trampoline = SKSE::GetTrampoline();

    REL::Relocation<std::uintptr_t> actorupdateFunction{REL::RelocationID(36357, 37348)};
    OnActorUpdate::func =
        trampoline.write_call<5>(actorupdateFunction.address() + REL::Relocate(0x6D3, 0x674), OnActorUpdate::thunk);
}
