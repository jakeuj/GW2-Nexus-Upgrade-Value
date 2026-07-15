#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Models.h"
#include "Settings.h"
#include "nexus/Nexus.h"

namespace UpgradeValue
{
    class App
    {
    public:
        explicit App(AddonAPI* api);
        ~App();

        void Initialize();
        void Shutdown();
        void Render();
        void RenderOptions();
        void ToggleWindow();
        static void ReceiveCjkFont(const char* identifier, void* font);

        bool* WindowVisiblePointer() { return &settings_.showWindow; }

    private:
        void RefreshAsync();
        void SaveSettings(bool refreshAfterSave);
        void JoinWorker();
        void RenderRowTooltip(const ResultRow& row, int selectedValue) const;

        const char* T(const char* chinese, const char* english) const;
        static std::string Coins(int copper);
        static bool ContainsInsensitive(const std::string& value, const char* query);

        AddonAPI* api_ = nullptr;
        std::unique_ptr<SettingsStore> store_;
        Settings settings_;
        std::array<char, 256> apiKeyBuffer_{};
        std::array<char, 128> searchBuffer_{};
        bool revealApiKey_ = false;

        std::atomic<bool> refreshing_{false};
        std::atomic<bool> stopRequested_{false};
        std::thread worker_;
        mutable std::mutex resultMutex_;
        std::vector<ResultRow> rows_;
        std::string accountName_;
        std::string status_;
        std::string error_;
        std::string settingsMessage_;
        static void* cjkFont_;
    };
}
