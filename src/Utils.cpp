#include "Utils.h"

//#ifdef GetObject
//#undef GetObject
//#endif

bool Utilities::Menu::IsOpen(const std::string_view menuname) {
    if (const auto ui = RE::UI::GetSingleton()) {
        if (ui->IsMenuOpen(menuname)) return true;
    }
    return false;
}

std::filesystem::path Utilities::GetLogPath() {
    const auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    return logFilePath;
}

std::vector<std::string> Utilities::ReadLogFile() {
    std::vector<std::string> logLines;

    // Open the log file
    std::ifstream file(GetLogPath().c_str());
    if (!file.is_open()) {
        // Handle error
        return logLines;
    }

    // Read and store each line from the file
    std::string line;
    while (std::getline(file, line)) {
        logLines.push_back(line);
    }

    file.close();

    return logLines;
}

//const bool Utilities::IsVampireLord(const RE::Actor* player) { 
//    const auto race = player->GetRace();
//    if (!race) return false;
//    RE::TESForm** beastform = RE::BGSDefaultObjectManager::GetSingleton()->GetObject(RE::DefaultObjectID::kVampireRace);
//    return beastform && *beastform ? (*beastform)->GetFormID() == race->GetFormID() : false;
//}
//
//
//const bool Utilities::IsWerewolf(const RE::Actor* player) {
//    const auto race = player->GetRace();
//    if (!race) return false;
//    RE::TESForm** beastform = RE::BGSDefaultObjectManager::GetSingleton()->GetObject(RE::DefaultObjectID::kWerewolfRace);
//    return beastform && *beastform ? (*beastform)->GetFormID() == race->GetFormID() : false;
//}


bool Utilities::IsWerewolf(const RE::Actor* player) {
    const auto race = player->GetRace();
    if (!race) return false;
    return race->GetFormID() == 0x000CDD84;
}

std::string Utilities::kDelivery2Char(const int delivery) {
    switch (delivery) {
        case 0:
            return "Self";
        case 1:
            return "Touch";
        case 2:
            return "Aimed";
        case 3:
            return "Target";
        case 4:
            return "Target Location";
        default:
            return "Unknown";
    }
}