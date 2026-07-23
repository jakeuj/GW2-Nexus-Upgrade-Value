#include "Gw2Api.h"

#include <Windows.h>

#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "HttpClient.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace UpgradeValue
{
    namespace
    {
        constexpr size_t BatchSize = 150;

        std::string ToTraditionalChinese(const std::string& utf8)
        {
            if (utf8.empty()) return utf8;
            const int wideLength = MultiByteToWideChar(CP_UTF8, 0, utf8.data(),
                                                       static_cast<int>(utf8.size()), nullptr, 0);
            if (wideLength <= 0) return utf8;
            std::wstring simplified(static_cast<size_t>(wideLength), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()),
                                simplified.data(), wideLength);

            const int traditionalLength = LCMapStringEx(
                L"zh-CN", LCMAP_TRADITIONAL_CHINESE,
                simplified.data(), wideLength, nullptr, 0, nullptr, nullptr, 0);
            if (traditionalLength <= 0) return utf8;
            std::wstring traditional(static_cast<size_t>(traditionalLength), L'\0');
            LCMapStringEx(L"zh-CN", LCMAP_TRADITIONAL_CHINESE,
                          simplified.data(), wideLength, traditional.data(),
                          traditionalLength, nullptr, nullptr, 0);

            const int resultLength = WideCharToMultiByte(
                CP_UTF8, 0, traditional.data(), traditionalLength,
                nullptr, 0, nullptr, nullptr);
            if (resultLength <= 0) return utf8;
            std::string result(static_cast<size_t>(resultLength), '\0');
            WideCharToMultiByte(CP_UTF8, 0, traditional.data(), traditionalLength,
                                result.data(), resultLength, nullptr, nullptr);
            return result;
        }

        bool GetJson(HttpClient& http, const std::wstring& path,
                     const std::string& token, json& result, std::string& error)
        {
            std::string body;
            if (!http.Get(path, token, body, error))
            {
                try
                {
                    const json detail = json::parse(body);
                    const std::string text = detail.value("text", "");
                    if (!text.empty()) error += ": " + text;
                }
                catch (...) {}
                return false;
            }

            try
            {
                result = json::parse(body);
                return true;
            }
            catch (const std::exception& ex)
            {
                error = std::string("Invalid JSON response: ") + ex.what();
                return false;
            }
        }

        std::vector<int> ReadIds(const json& value, const char* field)
        {
            std::vector<int> ids;
            const auto it = value.find(field);
            if (it == value.end() || !it->is_array()) return ids;
            for (const auto& id : *it)
                if (id.is_number_integer()) ids.push_back(id.get<int>());
            return ids;
        }

        void AddInstance(const json& value, const std::string& location,
                         bool includeInfusions, std::vector<ItemInstance>& instances)
        {
            if (!value.is_object()) return;
            const auto upgrades = ReadIds(value, "upgrades");
            const auto infusions = includeInfusions ? ReadIds(value, "infusions") : std::vector<int>{};
            if (upgrades.empty() && infusions.empty()) return;

            ItemInstance instance;
            instance.itemId = value.value("id", 0);
            if (instance.itemId == 0) return;
            instance.location = location;
            instance.binding = value.value("binding", "");
            const std::string boundTo = value.value("bound_to", "");
            if (!boundTo.empty()) instance.binding += ": " + boundTo;
            instance.upgrades = upgrades;
            instance.infusions = infusions;
            instances.push_back(std::move(instance));
        }

        void ParseFlatStorage(const json& data, const std::string& label,
                              bool includeInfusions, std::vector<ItemInstance>& instances)
        {
            if (!data.is_array()) return;
            for (size_t i = 0; i < data.size(); ++i)
                AddInstance(data[i], label + " #" + std::to_string(i + 1),
                            includeInfusions, instances);
        }

        void ParseCharacters(const json& characters, bool includeInfusions,
                             std::vector<ItemInstance>& instances)
        {
            if (!characters.is_array()) return;
            for (const auto& character : characters)
            {
                if (!character.is_object()) continue;
                const std::string name = character.value("name", "Unknown character");

                const auto equipment = character.find("equipment");
                if (equipment != character.end() && equipment->is_array())
                {
                    for (const auto& item : *equipment)
                    {
                        const std::string slot = item.is_object() ? item.value("slot", "") : "";
                        AddInstance(item, name + " / Equipped " + slot,
                                    includeInfusions, instances);
                    }
                }

                const auto bags = character.find("bags");
                if (bags == character.end() || !bags->is_array()) continue;
                for (size_t bagIndex = 0; bagIndex < bags->size(); ++bagIndex)
                {
                    const auto& bag = (*bags)[bagIndex];
                    if (!bag.is_object()) continue;
                    const auto inventory = bag.find("inventory");
                    if (inventory == bag.end() || !inventory->is_array()) continue;
                    for (size_t slot = 0; slot < inventory->size(); ++slot)
                    {
                        AddInstance((*inventory)[slot],
                            name + " / Bag " + std::to_string(bagIndex + 1) +
                            " / Slot " + std::to_string(slot + 1),
                            includeInfusions, instances);
                    }
                }
            }
        }

        std::wstring IdQuery(const wchar_t* endpoint, const std::vector<int>& ids,
                             size_t begin, size_t end, bool chinese)
        {
            std::wostringstream path;
            path << endpoint << L"?ids=";
            for (size_t i = begin; i < end; ++i)
            {
                if (i != begin) path << L',';
                path << ids[i];
            }
            if (chinese) path << L"&lang=zh";
            return path.str();
        }

        bool HasFlag(const json& item, const char* wanted)
        {
            const auto flags = item.find("flags");
            if (flags == item.end() || !flags->is_array()) return false;
            return std::any_of(flags->begin(), flags->end(), [wanted](const json& value)
            {
                return value.is_string() && value.get<std::string>() == wanted;
            });
        }

        bool FetchItemInfo(HttpClient& http, const std::vector<int>& ids,
                           std::unordered_map<int, ItemInfo>& output, std::string& error,
                           const std::atomic<bool>& stopRequested, bool chineseNames)
        {
            for (size_t begin = 0; begin < ids.size(); begin += BatchSize)
            {
                if (stopRequested.load()) return false;
                const size_t end = std::min(ids.size(), begin + BatchSize);
                json response;
                if (!GetJson(http, IdQuery(L"/v2/items", ids, begin, end, chineseNames),
                             "", response, error)) return false;
                if (!response.is_array()) continue;

                for (const auto& item : response)
                {
                    if (!item.is_object()) continue;
                    ItemInfo info;
                    info.id = item.value("id", 0);
                    info.name = item.value("name", "Unknown item");
                    if (chineseNames) info.name = ToTraditionalChinese(info.name);
                    info.rarity = item.value("rarity", "");
                    info.type = item.value("type", "");
                    info.noSalvage = HasFlag(item, "NoSalvage");
                    info.accountBound = HasFlag(item, "AccountBound") ||
                                        HasFlag(item, "SoulbindOnAcquire");
                    const auto details = item.find("details");
                    if (details != item.end() && details->is_object())
                        info.detailType = details->value("type", "");
                    if (info.id != 0) output[info.id] = std::move(info);
                }
            }
            return true;
        }

        bool FetchPrices(HttpClient& http, const std::vector<int>& ids,
                         std::unordered_map<int, PriceInfo>& output, std::string& error,
                         const std::atomic<bool>& stopRequested)
        {
            for (size_t begin = 0; begin < ids.size(); begin += BatchSize)
            {
                if (stopRequested.load()) return false;
                const size_t end = std::min(ids.size(), begin + BatchSize);
                json response;
                if (!GetJson(http, IdQuery(L"/v2/commerce/prices", ids, begin, end, false),
                             "", response, error))
                {
                    // The API can reject a batch containing only non-tradable IDs.
                    // Missing price rows are valid for account-bound upgrades.
                    if (error.rfind("HTTP 400", 0) == 0)
                    {
                        error.clear();
                        continue;
                    }
                    return false;
                }
                if (!response.is_array()) continue;

                for (const auto& value : response)
                {
                    if (!value.is_object()) continue;
                    PriceInfo price;
                    price.listed = true;
                    const auto buys = value.find("buys");
                    const auto sells = value.find("sells");
                    if (buys != value.end() && buys->is_object())
                        price.buy = buys->value("unit_price", 0);
                    if (sells != value.end() && sells->is_object())
                        price.sell = sells->value("unit_price", 0);
                    output[value.value("id", 0)] = price;
                }
            }
            return true;
        }

        UpgradeKind DetectKind(const ItemInfo& info, bool isInfusion)
        {
            if (isInfusion || info.detailType == "Infusion") return UpgradeKind::Infusion;
            if (info.detailType == "Rune") return UpgradeKind::Rune;
            if (info.detailType == "Sigil") return UpgradeKind::Sigil;
            return UpgradeKind::Other;
        }

        std::vector<int> SortedIds(const std::unordered_set<int>& ids)
        {
            std::vector<int> result(ids.begin(), ids.end());
            std::sort(result.begin(), result.end());
            return result;
        }
    }

    ScanResult Gw2Api::Scan(const std::string& apiKey,
                            bool includeInfusions,
                            bool chineseNames,
                            const std::atomic<bool>& stopRequested) const
    {
        ScanResult result;
        if (apiKey.empty())
        {
            result.error = "API key is empty";
            return result;
        }

        HttpClient http;
        json tokenInfo;
        std::string error;
        if (!GetJson(http, L"/v2/tokeninfo", apiKey, tokenInfo, error))
        {
            result.error = "API key validation failed: " + error;
            return result;
        }

        std::unordered_set<std::string> permissions;
        for (const auto& permission : tokenInfo.value("permissions", json::array()))
            if (permission.is_string()) permissions.insert(permission.get<std::string>());
        for (const char* required : {"account", "inventories", "characters"})
        {
            if (permissions.find(required) == permissions.end())
            {
                result.error = std::string("API key is missing permission: ") + required;
                return result;
            }
        }

        json account;
        if (GetJson(http, L"/v2/account", apiKey, account, error))
            result.accountName = account.value("name", "");
        error.clear();

        json bank;
        json shared;
        json characters;
        if (!GetJson(http, L"/v2/account/bank", apiKey, bank, error) ||
            !GetJson(http, L"/v2/account/inventory", apiKey, shared, error) ||
            !GetJson(http, L"/v2/characters?ids=all", apiKey, characters, error))
        {
            result.error = "Unable to read account inventory: " + error;
            return result;
        }
        if (stopRequested.load()) return result;

        std::vector<ItemInstance> instances;
        ParseFlatStorage(bank, "Account Bank", includeInfusions, instances);
        ParseFlatStorage(shared, "Shared Inventory", includeInfusions, instances);
        ParseCharacters(characters, includeInfusions, instances);

        std::unordered_set<int> allItemIds;
        std::unordered_set<int> upgradeIds;
        for (const auto& instance : instances)
        {
            allItemIds.insert(instance.itemId);
            for (const int id : instance.upgrades)
            {
                allItemIds.insert(id);
                upgradeIds.insert(id);
            }
            for (const int id : instance.infusions)
            {
                allItemIds.insert(id);
                upgradeIds.insert(id);
            }
        }

        std::unordered_map<int, ItemInfo> itemInfo;
        if (!allItemIds.empty() &&
            !FetchItemInfo(http, SortedIds(allItemIds), itemInfo, error, stopRequested, chineseNames))
        {
            if (!stopRequested.load()) result.error = "Unable to read item details: " + error;
            return result;
        }

        std::unordered_map<int, PriceInfo> prices;
        if (!upgradeIds.empty() &&
            !FetchPrices(http, SortedIds(upgradeIds), prices, error, stopRequested))
        {
            if (!stopRequested.load()) result.error = "Unable to read Trading Post prices: " + error;
            return result;
        }
        if (stopRequested.load()) return result;

        for (const auto& instance : instances)
        {
            const auto gearIt = itemInfo.find(instance.itemId);
            if (gearIt == itemInfo.end()) continue;
            const ItemInfo& gear = gearIt->second;

            // Orange equipment is Exotic. Legendary equipment is purple and must never be salvaged.
            if (gear.rarity != "Exotic" && gear.rarity != "Legendary") continue;

            auto append = [&](int id, bool isInfusion)
            {
                const auto upgradeIt = itemInfo.find(id);
                if (upgradeIt == itemInfo.end()) return;
                const ItemInfo& upgrade = upgradeIt->second;

                ResultRow row;
                row.gearId = gear.id;
                row.gearName = gear.name;
                row.gearRarity = gear.rarity;
                row.location = instance.location;
                row.binding = instance.binding;
                row.gearNoSalvage = gear.noSalvage;
                row.upgradeId = upgrade.id;
                row.upgradeName = upgrade.name;
                row.kind = DetectKind(upgrade, isInfusion);
                row.upgradeAccountBound = upgrade.accountBound;

                const auto priceIt = prices.find(id);
                if (priceIt != prices.end())
                {
                    row.instantSell = priceIt->second.buy;
                    row.listing = priceIt->second.sell;
                    row.netListing = static_cast<int>(row.listing * 0.85);
                    row.hasPrice = priceIt->second.listed &&
                                   (row.instantSell > 0 || row.listing > 0);
                }
                result.rows.push_back(std::move(row));
            };

            for (const int id : instance.upgrades) append(id, false);
            for (const int id : instance.infusions) append(id, true);
        }

        result.status = "Scan complete: " + std::to_string(result.rows.size()) +
                        " embedded upgrades found";
        return result;
    }
}
