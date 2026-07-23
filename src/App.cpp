#include "App.h"

#include <Windows.h>
#include <shellapi.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <iterator>
#include <sstream>

#include "Gw2Api.h"
#include "imgui/imgui.h"

#pragma comment(lib, "shell32.lib")

namespace UpgradeValue
{
    namespace
    {
        constexpr const char* WindowName = "Upgrade Value##NexusUpgradeValue";
        // Nexus requests the full Chinese range for FONT_DEFAULT, but the
        // selected font file may not provide those glyphs. Verify coverage
        // before enabling the Traditional Chinese UI.
        constexpr const char* CjkFontId = "FONT_DEFAULT";
        constexpr ImGuiID LocationColumnId = 0x4C4F4341; // "LOCA"

        struct FontScope
        {
            explicit FontScope(ImFont* font) : active(font != nullptr)
            {
                if (active) ImGui::PushFont(font);
            }
            ~FontScope()
            {
                if (active) ImGui::PopFont();
            }
            bool active;
        };

        const char* KindLabel(UpgradeKind kind, bool chinese)
        {
            switch (kind)
            {
                case UpgradeKind::Rune: return chinese ? "符文" : "Rune";
                case UpgradeKind::Sigil: return chinese ? "印記" : "Sigil";
                case UpgradeKind::Infusion: return chinese ? "灌注" : "Infusion";
                default: return chinese ? "升級" : "Upgrade";
            }
        }
    }

    void* App::cjkFont_ = nullptr;

    App::App(AddonAPI* api) : api_(api)
    {
    }

    App::~App()
    {
        Shutdown();
    }

    void App::Initialize()
    {
        const char* addonDirectory = api_->Paths.GetAddonDirectory("UpgradeValue");
        const std::filesystem::path directory = addonDirectory && *addonDirectory
            ? std::filesystem::u8path(addonDirectory)
            : std::filesystem::path("addons") / "UpgradeValue";

        store_ = std::make_unique<SettingsStore>(directory);
        settings_ = store_->Load();
        strncpy_s(apiKeyBuffer_.data(), apiKeyBuffer_.size(), settings_.apiKey.c_str(), _TRUNCATE);

        api_->Fonts.Get(CjkFontId, ReceiveCjkFont);

        if (settings_.chineseUi && !FontSupportsTraditionalChinese())
        {
            settings_.chineseUi = false;
            languageWarning_ = T(
                "目前的 Nexus 字型不支援繁體中文；已切換為英文。請在 Nexus 選項 > 樣式選擇支援 CJK 的字型後再啟用繁中。",
                "Traditional Chinese is unavailable with the current Nexus font, so Upgrade Value switched to English. Choose a CJK-capable font in Nexus Options > Style to enable it.");

            std::string saveError;
            if (!store_->Save(settings_, saveError))
                settingsMessage_ = saveError;
        }

        status_ = T("尚未掃描。", "Not scanned yet.");
        if (!settings_.apiKey.empty()) RefreshAsync();
    }

    void App::Shutdown()
    {
        if (!store_) return;
        scanGeneration_.fetch_add(1);
        stopRequested_.store(true);
        JoinWorker();
        refreshPending_ = false;

        api_->Fonts.Release(CjkFontId, ReceiveCjkFont);
        cjkFont_ = nullptr;

        std::string ignored;
        store_->Save(settings_, ignored);
        store_.reset();
    }

    void App::JoinWorker()
    {
        if (worker_.joinable()) worker_.join();
        refreshing_.store(false);
    }

    void App::RefreshAsync()
    {
        if (refreshing_.load() || settings_.apiKey.empty()) return;
        JoinWorker();

        const uint64_t generation = scanGeneration_.fetch_add(1) + 1;
        const std::string apiKey = settings_.apiKey;
        const bool includeInfusions = settings_.includeInfusions;
        const bool chineseNames = settings_.chineseUi;
        stopRequested_.store(false);
        refreshing_.store(true);

        {
            std::lock_guard lock(resultMutex_);
            error_.clear();
            status_ = T("正在讀取帳號、物品與交易所價格…",
                        "Reading account, item and Trading Post data...");
        }

        worker_ = std::thread([this, apiKey, includeInfusions, chineseNames, generation]
        {
            Gw2Api gw2;
            ScanResult scan = gw2.Scan(apiKey, includeInfusions, chineseNames, stopRequested_);
            if (chineseNames && scan.error.empty())
                scan.status = "掃描完成：找到 " + std::to_string(scan.rows.size()) + " 個內嵌升級。";
            if (!stopRequested_.load() && scanGeneration_.load() == generation)
            {
                std::lock_guard lock(resultMutex_);
                rows_ = std::move(scan.rows);
                accountName_ = std::move(scan.accountName);
                status_ = std::move(scan.status);
                error_ = std::move(scan.error);
                if (status_.empty() && error_.empty())
                    status_ = T("掃描完成。", "Scan complete.");
            }
            refreshing_.store(false);
        });
    }

    void App::QueueRefresh()
    {
        if (settings_.apiKey.empty()) return;

        if (refreshing_.load())
        {
            scanGeneration_.fetch_add(1);
            stopRequested_.store(true);
            refreshPending_ = true;
            return;
        }

        RefreshAsync();
    }

    void App::ProcessPendingRefresh()
    {
        if (!refreshPending_ || refreshing_.load()) return;
        refreshPending_ = false;
        RefreshAsync();
    }

    void App::ClearRowsForLanguageRefresh()
    {
        std::lock_guard lock(resultMutex_);
        rows_.clear();
        error_.clear();
        status_ = T("語言已變更，正在重新掃描…",
                    "Language changed; refreshing data...");
    }

    void App::ApplyLanguage(bool chinese)
    {
        if (chinese && !FontSupportsTraditionalChinese())
        {
            languageWarning_ = T(
                "目前的 Nexus 字型不支援繁體中文。請在 Nexus 選項 > 樣式選擇支援 CJK 的字型後再啟用繁中。",
                "Traditional Chinese requires a CJK-capable font. Choose one in Nexus Options > Style, then enable Traditional Chinese again.");
            return;
        }

        if (settings_.chineseUi == chinese) return;

        settings_.chineseUi = chinese;
        languageWarning_.clear();
        ClearRowsForLanguageRefresh();
        SaveSettings(false);
        QueueRefresh();
    }

    void App::EnsureLanguageSupported()
    {
        if (!settings_.chineseUi || !cjkFont_ || FontSupportsTraditionalChinese()) return;

        settings_.chineseUi = false;
        languageWarning_ = T(
            "目前的 Nexus 字型不支援繁體中文；已切換為英文。請在 Nexus 選項 > 樣式選擇支援 CJK 的字型後再啟用繁中。",
            "Traditional Chinese is unavailable with the current Nexus font, so Upgrade Value switched to English. Choose a CJK-capable font in Nexus Options > Style to enable it.");
        ClearRowsForLanguageRefresh();
        SaveSettings(false);
        QueueRefresh();
    }

    const char* App::T(const char* chinese, const char* english) const
    {
        return settings_.chineseUi ? chinese : english;
    }

    bool App::FontSupportsTraditionalChinese()
    {
        const auto* font = static_cast<const ImFont*>(cjkFont_);
        if (!font) return false;

        // Representative glyphs from "繁體中文介面". A font that cannot render
        // these core labels cannot safely display the Traditional Chinese UI.
        constexpr ImWchar RequiredGlyphs[] =
        {
            0x7E41, // 繁
            0x9AD4, // 體
            0x4E2D, // 中
            0x6587, // 文
            0x4ECB, // 介
            0x9762, // 面
        };

        return std::all_of(std::begin(RequiredGlyphs), std::end(RequiredGlyphs),
            [font](ImWchar glyph)
            {
                return font->FindGlyphNoFallback(glyph) != nullptr;
            });
    }

    std::string App::Coins(int copper)
    {
        if (copper <= 0) return "—";
        const int gold = copper / 10000;
        const int silver = (copper / 100) % 100;
        const int remainder = copper % 100;
        std::ostringstream out;
        if (gold > 0) out << gold << "g ";
        if (gold > 0 || silver > 0) out << silver << "s ";
        out << remainder << "c";
        return out.str();
    }

    bool App::ContainsInsensitive(const std::string& value, const char* query)
    {
        if (!query || !*query) return true;
        std::string lhs = value;
        std::string rhs = query;
        std::transform(lhs.begin(), lhs.end(), lhs.begin(), [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });
        std::transform(rhs.begin(), rhs.end(), rhs.begin(), [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });
        return lhs.find(rhs) != std::string::npos;
    }

    void App::ToggleWindow()
    {
        settings_.showWindow = !settings_.showWindow;
    }

    void App::ReceiveCjkFont(const char*, void* font)
    {
        cjkFont_ = font;
    }

    void App::RenderRowTooltip(const ResultRow& row, int selectedValue) const
    {
        if (!ImGui::IsItemHovered()) return;
        ImGui::BeginTooltip();
        ImGui::TextUnformatted(row.upgradeName.c_str());
        ImGui::Separator();
        ImGui::Text("%s: %s", T("裝備", "Equipment"), row.gearName.c_str());
        ImGui::Text("%s: %s", T("位置", "Location"), row.location.c_str());
        ImGui::Text("%s: %s", T("立即賣出", "Instant sell"), Coins(row.instantSell).c_str());
        ImGui::Text("%s: %s", T("最低掛單", "Lowest listing"), Coins(row.listing).c_str());
        ImGui::Text("%s: %s", T("扣除 15%% 後", "After 15%% fees"), Coins(row.netListing).c_str());
        ImGui::Text("%s: %s", T("目前採用價值", "Selected value"), Coins(selectedValue).c_str());
        if (!row.binding.empty()) ImGui::Text("%s: %s", T("綁定", "Binding"), row.binding.c_str());
        ImGui::EndTooltip();
    }

    void App::Render()
    {
        EnsureLanguageSupported();
        ProcessPendingRefresh();
        if (!settings_.showWindow) return;

        ImGui::SetNextWindowSize(ImVec2(980.0f, 570.0f), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(WindowName, &settings_.showWindow))
        {
            ImGui::End();
            return;
        }

        FontScope fontScope(settings_.chineseUi ? static_cast<ImFont*>(cjkFont_) : nullptr);

        if (!languageWarning_.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.72f, 0.25f, 1.0f));
            ImGui::TextWrapped("%s", languageWarning_.c_str());
            ImGui::PopStyleColor();
            ImGui::Separator();
        }

        if (settings_.apiKey.empty())
        {
            ImGui::TextWrapped("%s", T(
                "尚未設定 GW2 API Key。請到 Nexus 的 Upgrade Value 設定頁貼上 Key。",
                "No GW2 API key is configured. Paste one in the Nexus settings for Upgrade Value."));
            ImGui::TextWrapped("%s", T(
                "必要權限：account、inventories、characters。",
                "Required permissions: account, inventories, characters."));
            ImGui::End();
            return;
        }

        if (refreshing_.load())
            ImGui::TextDisabled("%s", T("掃描中…", "Refreshing..."));
        else if (ImGui::Button(T("重新掃描", "Refresh")))
            RefreshAsync();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::InputInt(T("黑獅門檻（銀）", "Black Lion threshold (silver)"),
                            &settings_.thresholdSilver, 5, 25))
        {
            if (settings_.thresholdSilver < 0) settings_.thresholdSilver = 0;
        }
        ImGui::SameLine();
        ImGui::Checkbox(T("以扣稅掛單價判斷", "Use net listing value"), &settings_.useNetListing);

        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputTextWithHint("##UpgradeSearch", T("搜尋裝備、升級或位置…", "Search equipment, upgrade or location..."),
                                 searchBuffer_.data(), searchBuffer_.size());

        std::vector<ResultRow> rows;
        std::string status;
        std::string error;
        std::string account;
        {
            std::lock_guard lock(resultMutex_);
            rows = rows_;
            status = status_;
            error = error_;
            account = accountName_;
        }

        if (!account.empty())
        {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", account.c_str());
        }

        if (!error.empty())
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", error.c_str());
        else
            ImGui::TextDisabled("%s", status.c_str());

        ImGui::TextWrapped("%s", T(
            "橘色是特異（Exotic）；紫色才是傳奇。傳奇裝備絕對不應分解。灌注不會被黑獅分解器取回。",
            "Orange is Exotic; purple is Legendary. Never salvage Legendary equipment. Black Lion kits do not recover infusions."));

        const int thresholdCopper = settings_.thresholdSilver * 100;
        const ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
                                      ImGuiTableFlags_SizingStretchProp |
                                      ImGuiTableFlags_Sortable | ImGuiTableFlags_SortTristate;

        if (ImGui::BeginTable("##UpgradeValueRows", 7, flags, ImVec2(0.0f, -1.0f)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn(T("建議", "Recommendation"),
                ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 120.0f);
            ImGui::TableSetupColumn(T("升級", "Upgrade"),
                ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort, 1.7f);
            ImGui::TableSetupColumn(T("裝備", "Equipment"),
                ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort, 1.4f);
            ImGui::TableSetupColumn(T("位置", "Location"),
                ImGuiTableColumnFlags_WidthStretch |
                ImGuiTableColumnFlags_PreferSortAscending,
                1.8f, LocationColumnId);
            ImGui::TableSetupColumn(T("立即賣", "Instant sell"),
                ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 86.0f);
            ImGui::TableSetupColumn(T("掛單", "Listing"),
                ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 86.0f);
            ImGui::TableSetupColumn(T("扣稅掛單", "Net listing"),
                ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoSort, 86.0f);

            ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
            if (sortSpecs)
            {
                if (sortSpecs->SpecsCount == 1 &&
                    sortSpecs->Specs[0].ColumnUserID == LocationColumnId)
                {
                    const bool ascending =
                        sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
                    std::stable_sort(rows.begin(), rows.end(),
                        [ascending](const ResultRow& a, const ResultRow& b)
                        {
                            if (a.location == b.location) return false;
                            return ascending
                                ? a.location < b.location
                                : a.location > b.location;
                        });
                }
                sortSpecs->SpecsDirty = false;
            }

            ImGui::TableHeadersRow();

            for (const auto& row : rows)
            {
                if (!ContainsInsensitive(row.upgradeName, searchBuffer_.data()) &&
                    !ContainsInsensitive(row.gearName, searchBuffer_.data()) &&
                    !ContainsInsensitive(row.location, searchBuffer_.data())) continue;

                const int selectedValue = settings_.useNetListing ? row.netListing : row.instantSell;
                const char* recommendation = nullptr;
                ImVec4 recommendationColor(0.75f, 0.75f, 0.75f, 1.0f);

                if (row.gearRarity == "Legendary")
                {
                    recommendation = T("絕對不要分解", "Never salvage");
                    recommendationColor = ImVec4(0.82f, 0.42f, 1.0f, 1.0f);
                }
                else if (row.gearNoSalvage)
                {
                    recommendation = T("不可分解", "Not salvageable");
                }
                else if (row.kind == UpgradeKind::Infusion)
                {
                    recommendation = T("黑獅不取回", "BL kit won't recover");
                    recommendationColor = ImVec4(1.0f, 0.55f, 0.25f, 1.0f);
                }
                else if (!row.hasPrice || row.upgradeAccountBound)
                {
                    recommendation = T("無交易所價格", "No TP price");
                }
                else if (selectedValue >= thresholdCopper)
                {
                    recommendation = T("使用黑獅", "Use Black Lion");
                    recommendationColor = ImVec4(0.25f, 1.0f, 0.45f, 1.0f);
                }
                else
                {
                    recommendation = T("一般分解", "Normal salvage");
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(recommendationColor, "%s", recommendation);
                RenderRowTooltip(row, selectedValue);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(row.upgradeName.c_str());
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s · ID %d", KindLabel(row.kind, settings_.chineseUi), row.upgradeId);
                    ImGui::EndTooltip();
                }

                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(row.gearName.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::TextUnformatted(row.location.c_str());
                ImGui::TableSetColumnIndex(4);
                ImGui::TextUnformatted(Coins(row.instantSell).c_str());
                ImGui::TableSetColumnIndex(5);
                ImGui::TextUnformatted(Coins(row.listing).c_str());
                ImGui::TableSetColumnIndex(6);
                ImGui::TextUnformatted(Coins(row.netListing).c_str());
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }

    void App::SaveSettings(bool refreshAfterSave)
    {
        settings_.apiKey.assign(apiKeyBuffer_.data());
        std::string error;
        if (!store_->Save(settings_, error))
        {
            settingsMessage_ = error;
            return;
        }
        settingsMessage_ = T("設定已儲存；API Key 已用 Windows DPAPI 加密。",
                             "Settings saved; the API key is encrypted with Windows DPAPI.");
        if (refreshAfterSave) QueueRefresh();
    }

    void App::RenderOptions()
    {
        EnsureLanguageSupported();
        ProcessPendingRefresh();
        FontScope fontScope(settings_.chineseUi ? static_cast<ImFont*>(cjkFont_) : nullptr);
        ImGui::Separator();
        ImGui::TextUnformatted("Upgrade Value");
        ImGui::Separator();
        ImGui::Checkbox(T("顯示主視窗", "Show main window"), &settings_.showWindow);

        ImGui::TextUnformatted("Language");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(230.0f);
        const bool cjkAvailable = FontSupportsTraditionalChinese();
        const char* languagePreview =
            settings_.chineseUi ? "Traditional Chinese" : "English";
        if (ImGui::BeginCombo("##UpgradeValueLanguage", languagePreview))
        {
            if (ImGui::Selectable("English", !settings_.chineseUi))
                ApplyLanguage(false);

            if (cjkAvailable)
            {
                if (ImGui::Selectable("Traditional Chinese", settings_.chineseUi))
                    ApplyLanguage(true);
            }
            else
            {
                ImGui::Selectable(
                    "Traditional Chinese (CJK font required)",
                    false, ImGuiSelectableFlags_Disabled);
            }
            ImGui::EndCombo();
        }

        if (!cjkAvailable)
        {
            ImGui::TextWrapped(
                "Choose a CJK-capable font in Nexus Options > Style to enable Traditional Chinese.");
        }

        if (!languageWarning_.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.72f, 0.25f, 1.0f));
            ImGui::TextWrapped("%s", languageWarning_.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Checkbox(T("包含灌注（僅顯示價格，不建議黑獅）",
                          "Include infusions (price only; never recommends Black Lion)"),
                        &settings_.includeInfusions);

        ImGui::TextWrapped("%s", T(
            "必要權限：account、inventories、characters。Key 只會傳送到 api.guildwars2.com，並以 Windows DPAPI 加密後存在本機。",
            "Required permissions: account, inventories, characters. The key is only sent to api.guildwars2.com and is encrypted locally with Windows DPAPI."));

        ImGui::SetNextItemWidth(520.0f);
        const ImGuiInputTextFlags inputFlags = revealApiKey_ ? ImGuiInputTextFlags_None : ImGuiInputTextFlags_Password;
        ImGui::InputText("GW2 API Key", apiKeyBuffer_.data(), apiKeyBuffer_.size(), inputFlags);
        ImGui::SameLine();
        ImGui::Checkbox(T("顯示", "Reveal"), &revealApiKey_);

        if (ImGui::Button(T("儲存並掃描", "Save and scan"))) SaveSettings(true);
        ImGui::SameLine();
        if (ImGui::Button(T("開啟 ArenaNet API Key 管理頁", "Open ArenaNet API key manager")))
            ShellExecuteA(nullptr, "open", "https://account.arena.net/applications", nullptr, nullptr, SW_SHOWNORMAL);

        if (!settingsMessage_.empty()) ImGui::TextWrapped("%s", settingsMessage_.c_str());
        ImGui::TextDisabled("%s", T("快捷鍵：Alt + Shift + U", "Keybind: Alt + Shift + U"));
        ImGui::Separator();
        ImGui::Text("%s: jakeuj", T("作者", "Author"));
        ImGui::SameLine();
        ImGui::TextDisabled("| GW2: jakeuj.5260");
        if (ImGui::Button("GitHub: github.com/jakeuj"))
            ShellExecuteA(nullptr, "open", "https://github.com/jakeuj", nullptr, nullptr, SW_SHOWNORMAL);
    }
}
