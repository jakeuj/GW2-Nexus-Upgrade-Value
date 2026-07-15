#include "Settings.h"

#include <Windows.h>
#include <dpapi.h>
#include <wincrypt.h>

#include <fstream>
#include <vector>

#include "nlohmann/json.hpp"

#pragma comment(lib, "crypt32.lib")

using json = nlohmann::json;

namespace UpgradeValue
{
    namespace
    {
        std::string Base64Encode(const BYTE* data, DWORD size)
        {
            DWORD required = 0;
            CryptBinaryToStringA(data, size, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                                 nullptr, &required);
            if (required == 0) return {};
            std::string encoded(required, '\0');
            if (!CryptBinaryToStringA(data, size,
                                      CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                                      encoded.data(), &required)) return {};
            if (!encoded.empty() && encoded.back() == '\0') encoded.pop_back();
            return encoded;
        }

        std::vector<BYTE> Base64Decode(const std::string& encoded)
        {
            DWORD required = 0;
            if (!CryptStringToBinaryA(encoded.c_str(), 0, CRYPT_STRING_BASE64,
                                      nullptr, &required, nullptr, nullptr)) return {};
            std::vector<BYTE> data(required);
            if (!CryptStringToBinaryA(encoded.c_str(), 0, CRYPT_STRING_BASE64,
                                      data.data(), &required, nullptr, nullptr)) return {};
            data.resize(required);
            return data;
        }

        std::string Protect(const std::string& plain)
        {
            if (plain.empty()) return {};
            DATA_BLOB input{};
            input.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(plain.data()));
            input.cbData = static_cast<DWORD>(plain.size());
            DATA_BLOB output{};
            if (!CryptProtectData(&input, L"GW2 Nexus Upgrade Value API key", nullptr,
                                  nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &output)) return {};
            const std::string result = Base64Encode(output.pbData, output.cbData);
            LocalFree(output.pbData);
            return result;
        }

        std::string Unprotect(const std::string& encoded)
        {
            const auto encrypted = Base64Decode(encoded);
            if (encrypted.empty()) return {};
            DATA_BLOB input{};
            input.pbData = const_cast<BYTE*>(encrypted.data());
            input.cbData = static_cast<DWORD>(encrypted.size());
            DATA_BLOB output{};
            if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr,
                                    CRYPTPROTECT_UI_FORBIDDEN, &output)) return {};
            std::string result(reinterpret_cast<char*>(output.pbData), output.cbData);
            LocalFree(output.pbData);
            return result;
        }
    }

    SettingsStore::SettingsStore(std::filesystem::path directory)
        : directory_(std::move(directory)), file_(directory_ / "settings.json")
    {
    }

    Settings SettingsStore::Load() const
    {
        Settings settings;
        std::ifstream input(file_);
        if (!input.is_open()) return settings;

        try
        {
            json data;
            input >> data;
            settings.apiKey = Unprotect(data.value("apiKeyProtected", ""));
            settings.thresholdSilver = data.value("thresholdSilver", 50);
            settings.useNetListing = data.value("useNetListing", false);
            settings.includeInfusions = data.value("includeInfusions", false);
            settings.showWindow = data.value("showWindow", true);
            settings.chineseUi = data.value("chineseUi", true);
        }
        catch (...) {}

        if (settings.thresholdSilver < 0) settings.thresholdSilver = 0;
        return settings;
    }

    bool SettingsStore::Save(const Settings& settings, std::string& error) const
    {
        error.clear();
        std::error_code ec;
        std::filesystem::create_directories(directory_, ec);
        if (ec)
        {
            error = "Unable to create settings directory";
            return false;
        }

        const std::string protectedKey = Protect(settings.apiKey);
        if (!settings.apiKey.empty() && protectedKey.empty())
        {
            error = "Windows could not encrypt the API key";
            return false;
        }

        json data;
        data["apiKeyProtected"] = protectedKey;
        data["thresholdSilver"] = settings.thresholdSilver;
        data["useNetListing"] = settings.useNetListing;
        data["includeInfusions"] = settings.includeInfusions;
        data["showWindow"] = settings.showWindow;
        data["chineseUi"] = settings.chineseUi;

        std::ofstream output(file_, std::ios::trunc);
        if (!output.is_open())
        {
            error = "Unable to open settings file";
            return false;
        }
        output << data.dump(2);
        return output.good();
    }
}
