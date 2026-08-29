#include "Events.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kPostLoad) {
        MCP::Register();
        logger::info("MCP registered.");
    }
    if (message->type == SKSE::MessagingInterface::kInputLoaded) {
        logger::info("Adding input event sink.");
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(OurEventSink::GetSingleton());
    }
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        auto* eventsink = OurEventSink::GetSingleton();
        if (auto* ui = RE::UI::GetSingleton(); ui) {
            logger::info("Adding event sink for dialogue menu auto zoom.");
            ui->AddEventSink<RE::MenuOpenCloseEvent>(eventsink);
        }
        RE::PlayerCharacter::GetSingleton()->AsBGSActorCellEventSource()->AddEventSink(eventsink);
        SKSE::GetCameraEventSource()->AddEventSink(eventsink);
        logger::info("Gameplay event sinks added.");
    }
}

static void SetupLog() {
    const auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    const auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    #ifndef NDEBUG
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
    #else
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
    #endif
    logger::info("Name of the plugin is {}.", pluginName);
    logger::info("Version of the plugin is {}.", SKSE::PluginDeclaration::GetSingleton()->GetVersion());
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SetupLog();
    logger::info("Plugin loaded");
    SKSE::Init(skse);
    Settings::LoadSettings();
    InitializeSerialization();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    Hooks::Install();
    return true;
}
