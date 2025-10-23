#pragma once
#include "ClibUtil/singleton.hpp"

class EventSink : public clib_util::singleton::ISingleton<EventSink>,
    public RE::BSTEventSink<RE::MenuOpenCloseEvent>, 
    public RE::BSTEventSink<RE::InputEvent*>, 
    public RE::BSTEventSink<RE::BGSActorCellEvent>,
    public RE::BSTEventSink<SKSE::CameraEvent> {
    
    using InputEvents = RE::InputEvent*;

    RE::UI* ui = RE::UI::GetSingleton();

    void HandleDialogueInputs(RE::InputEvent* const* evns);
    void _HandleDialogueInputs(const RE::ButtonEvent* a_event);

    void InputKeyDetection(RE::ButtonEvent* a_event);

public:

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::BGSActorCellEvent* a_event,
										  RE::BSTEventSource<RE::BGSActorCellEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const SKSE::CameraEvent* a_event, 
										  RE::BSTEventSource<SKSE::CameraEvent>*) override;
};