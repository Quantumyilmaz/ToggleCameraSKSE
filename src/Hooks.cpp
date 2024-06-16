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
    trampoline.create(Dialogue::trampoline_size);

    Dialogue::InstallHooks();
};

void Combat::OnActorUpdate::thunk(RE::Actor* a_actor, float a_zPos, RE::TESObjectCELL* a_cell) {
    if (!a_actor) return func(a_actor, a_zPos, a_cell);
    if (RE::PlayerCharacter::GetSingleton()->GetGameStatsData().byCharGenFlag.any(
            RE::PlayerCharacter::ByCharGenFlag::kHandsBound))
        return func(a_actor, a_zPos, a_cell);
    if (RE::PlayerCharacter::GetSingleton()->GetFormID() != a_actor->GetFormID())
        return func(a_actor, a_zPos, a_cell);
    auto plyr_c = RE::PlayerCamera::GetSingleton();
    if (!plyr_c->IsInFirstPerson() && !plyr_c->IsInThirdPerson()) return func(a_actor, a_zPos, a_cell);
    int shouldToggle = 0;
    auto thirdPersonState =
        static_cast<RE::ThirdPersonState*>(plyr_c->cameraStates[RE::CameraState::kThirdPerson].get());
    if (plyr_c->IsInThirdPerson() && thirdPersonState->currentZoomOffset == thirdPersonState->targetZoomOffset &&
        savedZoomOffset != thirdPersonState->currentZoomOffset) {
        savedZoomOffset = thirdPersonState->currentZoomOffset;
        thirdPersonState->savedZoomOffset = savedZoomOffset;
    }

    // killmove handling
    if (a_actor->IsInKillMove()) {
        oldstate_c = 1;
        return func(a_actor, a_zPos, a_cell);
    } else if (RE::PlayerCamera::GetSingleton()->IsInBleedoutMode()) {
        return func(a_actor, a_zPos, a_cell);
    }

    // weapon draw handling
    if (ToggleWeapon && !IsMagicEquipped()) {
        auto weapon_state = static_cast<uint32_t>(a_actor->AsActorState()->GetWeaponState());
        if ((!weapon_state || weapon_state == 3) && oldstate_w != weapon_state) {
            oldstate_w = weapon_state;
            shouldToggle += CamSwitchHandling(oldstate_w, ToggleWeapon.invert, ToggleWeapon.revert);
        }
    }

    // combat handling
    if (ToggleCombat && GetCombatState() != oldstate_c && !bow_cam_switched && !casting_switched) {
        oldstate_c = oldstate_c != 0 ? 0 : 1;
        shouldToggle += CamSwitchHandling(oldstate_c, ToggleCombat.invert, ToggleCombat.revert);
    }

    // bow first person aiming handling
    if (ToggleBowDraw) {
        auto attack_state = static_cast<uint32_t>(a_actor->AsActorState()->GetAttackState());
        bool is_3rd_p = Is3rdP();
        if (bool player_is_in_toggled_cam = ToggleBowDraw.invert ? is_3rd_p : !is_3rd_p; player_is_in_toggled_cam && attack_state == 8) {
            funcToggle();
            bow_cam_switched = true;
            return func(a_actor, a_zPos, a_cell);
        } else if (bow_cam_switched && (!attack_state || attack_state == 13) && !is_3rd_p && ToggleBowDraw.revert) {
            funcToggle();
            bow_cam_switched = false;
            return func(a_actor, a_zPos, a_cell);
        }
    }

    // magic draw and casting handling
    if (IsMagicEquipped()) {
        // magic draw handling
        auto magic_state = static_cast<uint32_t>(a_actor->AsActorState()->GetWeaponState());
        if (ToggleMagicWield && oldstate_m != magic_state) {
            oldstate_m = magic_state;
            if ((!magic_state || magic_state == 5) && !Is3rdP() && magic_switched && settings->os[1].second) {
                funcToggle();
                magic_switched = false;
                return func(a_actor, a_zPos, a_cell);
            } else if ((magic_state == 2 || magic_state == 3) && Is3rdP()) {
                funcToggle();
                magic_switched = true;
                return func(a_actor, a_zPos, a_cell);
            }
        }
        // magic casting handling
        if (settings->main[5].second) {
            if (IsCasting() && Is3rdP() && (!casting_switched || !settings->os[1].second)) {
                funcToggle();
                casting_switched = true;
                return func(a_actor, a_zPos, a_cell);
            } else if (!IsCasting() && !Is3rdP() && casting_switched && settings->os[1].second) {
                funcToggle();
                casting_switched = false;
                magic_switched = false;
                return func(a_actor, a_zPos, a_cell);
            }
        }
    }

    if (shouldToggle) funcToggle();

    return func(a_actor, a_zPos, a_cell);
}

void Combat::InstallHooks() {}