#pragma once

#include <array>
#include <filesystem>
#include <string>

namespace UpgradeValue
{
    struct Settings
    {
        std::string apiKey;
        int thresholdSilver = 50;
        bool useNetListing = false;
        bool includeInfusions = false;
        bool showWindow = true;
        bool chineseUi = true;
    };

    class SettingsStore
    {
    public:
        explicit SettingsStore(std::filesystem::path directory);

        Settings Load() const;
        bool Save(const Settings& settings, std::string& error) const;
        const std::filesystem::path& Directory() const { return directory_; }

    private:
        std::filesystem::path directory_;
        std::filesystem::path file_;
    };
}
