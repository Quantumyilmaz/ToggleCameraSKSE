#pragma once
#include "UI.h"

class OurEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                     public RE::BSTEventSink<RE::InputEvent*>,
                     public RE::BSTEventSink<RE::BGSActorCellEvent>,
                     public RE::BSTEventSink<SKSE::CameraEvent> {
    using InputEvents = RE::InputEvent*;

    OurEventSink() = default;
    OurEventSink(const OurEventSink&) = delete;
    OurEventSink(OurEventSink&&) = delete;
    OurEventSink& operator=(const OurEventSink&) = delete;
    OurEventSink& operator=(OurEventSink&&) = delete;

    RE::UI* ui = RE::UI::GetSingleton();

    void HandleDialogueInputs(RE::InputEvent* const* evns);
    void _HandleDialogueInputs(RE::ButtonEvent* a_event);

    void InputKeyDetection(RE::ButtonEvent* a_event);

public:
    static OurEventSink* GetSingleton() {
        static OurEventSink singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::BGSActorCellEvent* a_event,
                                          RE::BSTEventSource<RE::BGSActorCellEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const SKSE::CameraEvent* a_event,
                                          RE::BSTEventSource<SKSE::CameraEvent>*) override;
};