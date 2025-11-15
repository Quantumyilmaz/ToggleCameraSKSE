#include "Events.h"
#include "Logger.h"
#include "UI.h"

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        auto* eventsink = EventSink::GetSingleton();
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(eventsink);
        RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(eventsink);
        RE::PlayerCharacter::GetSingleton()->AsBGSActorCellEventSource()->AddEventSink(eventsink);
        SKSE::GetCameraEventSource()->AddEventSink(eventsink);

        MCP::Register();
        logger::info("MCP registered.");
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    logger::info("Plugin loaded");
    SKSE::Init(skse);
    SetupLog();
    Settings::LoadSettings();
    InitializeSerialization();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    Hooks::Install();
    return true;
}