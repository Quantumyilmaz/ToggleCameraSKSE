#include "Utils.h"

bool Utilities::Menu::IsOpen(const std::string_view menuname) {
    if (auto ui = RE::UI::GetSingleton()) {
        if (ui->IsMenuOpen(menuname)) return true;
    }
    return false;
}

std::filesystem::path Utilities::GetLogPath() {
    auto logsFolder = SKSE::log::log_directory();
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

std::string Utilities::kDelivery2Char(const int delivery) { 
    switch (delivery) {
		case 0: return "Self";
		case 1: return "Touch";
		case 2: return "Aimed";
		case 3: return "Target";
		case 4: return "Target Location";
		default: return "Unknown";
	}
}
