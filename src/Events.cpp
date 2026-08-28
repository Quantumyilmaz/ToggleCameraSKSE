#include "Events.h"


void OurEventSink::HandleDialogueInputs(RE::InputEvent* const* evns) {
    if (!Utilities::Menu::IsOpen(RE::DialogueMenu::MENU_NAME)) return;
    for (RE::InputEvent* e = *evns; e; e = e->next) {
        if (!e) continue;
        if (e->eventType.get() != RE::INPUT_EVENT_TYPE::kButton) continue;
        _HandleDialogueInputs(e->AsButtonEvent());
    }
}

void OurEventSink::_HandleDialogueInputs(RE::ButtonEvent* a_event) {
    const uint32_t keyMask = a_event->GetIDCode();
    const auto _device = a_event->GetDevice();
    // check if _device is supported
    if (std::ranges::find(SupportedDevices, _device) == SupportedDevices.end()) return;

    //logger::trace("Device: {}, KeyMask: {}", _device, keyMask);

    const auto purpose = Modules::Dialogue::GetPurpose(_device, keyMask);
    if (purpose == kNone) return;

    const float duration = a_event->HeldDuration();
    const bool isPressed = a_event->Value() != 0 && duration >= 0;
    const bool isReleased = a_event->Value() == 0 && duration != 0;
    bool _toggle = false; // switch for 1st/3rd person

    if (isPressed) {
        if (purpose == kZoomEnable)
            Modules::Dialogue::zoom_enabled = true;
        else if (purpose == kZoomIn && (Modules::Dialogue::zoom_enabled || !Modules::Dialogue::ZoomEnable)) {
            Modules::Dialogue::funcZoom(_device, true);
        } else if (purpose == kZoomOut && (Modules::Dialogue::zoom_enabled || !Modules::Dialogue::ZoomEnable)) {
            Modules::Dialogue::funcZoom(_device, false);
        }
    } else if (isReleased) {
        if (purpose == kZoomEnable)
            Modules::Dialogue::zoom_enabled = false;
        else if (purpose == kToggle && Modules::Dialogue::Toggle)
            _toggle = true;
    }
    if (_toggle) {
        Modules::Dialogue::funcToggle();
    }
}

RE::BSEventNotifyControl OurEventSink::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                                    RE::BSTEventSource<RE::MenuOpenCloseEvent>*) {
    if (!event) return RE::BSEventNotifyControl::kContinue;
    if (event->menuName != RE::DialogueMenu::MENU_NAME) return RE::BSEventNotifyControl::kContinue;

    const auto playerCamera = RE::PlayerCamera::GetSingleton();
    const auto thirdPersonState = static_cast<RE::ThirdPersonState*>(playerCamera->GetRuntimeData().cameraStates[
        RE::CameraState::kThirdPerson].get());

    if (event->opening && Modules::Dialogue::Toggle.fix_zoom.enabled && thirdPersonState) {
        Modules::Dialogue::preDialogueZoomOffset = playerCamera->IsInThirdPerson()
                                                        ? thirdPersonState->currentZoomOffset
                                                        : thirdPersonState->savedZoomOffset;
        if (playerCamera->IsInThirdPerson()) {
            thirdPersonState->targetZoomOffset = Modules::Dialogue::Toggle.fix_zoom.zoom_lvl;
        }
    }

    if (Modules::Dialogue::listen_auto_zoom && Modules::Dialogue::AutoToggle) {
        if (event->opening) {
            if (playerCamera->IsInThirdPerson() && !Modules::Dialogue::AutoToggle.invert) {
                Modules::Dialogue::funcToggle();
            } else if (playerCamera->IsInFirstPerson() && Modules::Dialogue::AutoToggle.invert) {
                Modules::Dialogue::funcToggle();
            }
        } else if (Modules::Dialogue::AutoToggle.revert) {
            if (playerCamera->IsInFirstPerson() && !Modules::Dialogue::AutoToggle.invert) {
                Modules::Dialogue::funcToggle();
            } else if (playerCamera->IsInThirdPerson() && Modules::Dialogue::AutoToggle.invert) {
                Modules::Dialogue::funcToggle();
            }
        }
    }

    if (!event->opening) {
        if (Modules::Dialogue::Toggle.revert && Modules::Dialogue::preDialogueZoomOffset && thirdPersonState) {
            thirdPersonState->targetZoomOffset = *Modules::Dialogue::preDialogueZoomOffset;
        }
        Modules::Dialogue::preDialogueZoomOffset.reset();
    }

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl OurEventSink::ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) {
    if (!evns) return RE::BSEventNotifyControl::kContinue;
    if (!*evns) return RE::BSEventNotifyControl::kContinue;

    if (MCP::listen_key) {
        for (RE::InputEvent* e = *evns; e; e = e->next) {
            if (!e) continue;
            if (e->eventType.get() != RE::INPUT_EVENT_TYPE::kButton) continue;
            const RE::ButtonEvent* a_event = e->AsButtonEvent();
            if (a_event->IsHeld()) continue;
            uint32_t keyMask = a_event->GetIDCode();
            auto _device = a_event->GetDevice();
            if (std::ranges::find(SupportedDevices, _device) == SupportedDevices.end()) continue;

            if (const float duration = a_event->HeldDuration(); a_event->Value() != 0 && duration >= 0) {
                MCP::listen_key = false;
                MCP::detected_device = _device;
                MCP::detected_key = keyMask;
                logger::info("Input Key Detection -> Device: {}, KeyMask: {}", device_names[_device], keyMask);
                break;
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }

    HandleDialogueInputs(evns);

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl OurEventSink::ProcessEvent(const RE::BGSActorCellEvent* a_event,
                                                    RE::BSTEventSource<RE::BGSActorCellEvent>*) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;

    //logger::trace("ActorCellEvent: {}", a_event->cellID);
    const RE::TESObjectCELL* cell = RE::TESForm::LookupByID<RE::TESObjectCELL>(a_event->cellID);
    if (!cell) return RE::BSEventNotifyControl::kContinue;
    const bool is_interior = cell->IsInteriorCell();
    const bool is_exterior = cell->IsExteriorCell();
    if (!is_interior && !is_exterior || is_exterior && is_interior) return RE::BSEventNotifyControl::kContinue;
    //logger::trace("Cell: {} is interior: {}, is exterior: {}", cell->GetFullName(), is_interior, is_exterior);

    if (is_exterior == Modules::Other::is_exterior) return RE::BSEventNotifyControl::kContinue;
    Modules::Other::is_exterior = is_exterior;
    if (is_exterior && !Modules::Other::ToggleCellChangeExterior) return RE::BSEventNotifyControl::kContinue;
    if (is_interior && !Modules::Other::ToggleCellChangeInterior) return RE::BSEventNotifyControl::kContinue;
    const auto is3rdP = RE::PlayerCamera::GetSingleton()->IsInThirdPerson();
    bool player_is_in_toggled_cam = true;
    if (is_exterior) player_is_in_toggled_cam = Modules::Other::ToggleCellChangeExterior.invert ? !is3rdP : is3rdP;
    else if (is_interior) player_is_in_toggled_cam = Modules::Other::ToggleCellChangeInterior.invert ? is3rdP : !is3rdP;

    if (player_is_in_toggled_cam) return RE::BSEventNotifyControl::kContinue;

    //logger::trace("Cell change detected. Toggled.");
    Modules::Other::funcToggle(is3rdP);

    return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl OurEventSink::ProcessEvent(const SKSE::CameraEvent* a_event,
                                                    RE::BSTEventSource<SKSE::CameraEvent>*) {
    if (!a_event) return RE::BSEventNotifyControl::kContinue;
    const auto& dialogueFixZoom = Modules::Dialogue::Toggle.fix_zoom;
    const auto& fixZoom = Utilities::Menu::IsOpen(RE::DialogueMenu::MENU_NAME) && dialogueFixZoom.enabled
                              ? dialogueFixZoom
                              : Modules::Other::FixZoom.fix_zoom;
    if (!fixZoom.enabled) return RE::BSEventNotifyControl::kContinue;
    if (!RE::PlayerCamera::GetSingleton()->IsInThirdPerson()) return RE::BSEventNotifyControl::kContinue;
    const auto thirdPersonState = static_cast<RE::ThirdPersonState*>(RE::PlayerCamera::GetSingleton()->GetRuntimeData().
        cameraStates[RE::CameraState::kThirdPerson].get());
    if (!thirdPersonState) return RE::BSEventNotifyControl::kContinue;
    //if (std::abs(thirdPersonState->currentZoomOffset - thirdPersonState->targetZoomOffset)>0.01){
    //    return RE::BSEventNotifyControl::kContinue;
    //}
    if (thirdPersonState->currentZoomOffset == fixZoom.zoom_lvl) return RE::BSEventNotifyControl::kContinue;
    logger::trace("CameraEvent: FixZoom");
    thirdPersonState->targetZoomOffset = fixZoom.zoom_lvl;

    return RE::BSEventNotifyControl::kContinue;
}
