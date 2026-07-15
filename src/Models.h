#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace UpgradeValue
{
    enum class UpgradeKind
    {
        Rune,
        Sigil,
        Infusion,
        Other
    };

    struct ItemInstance
    {
        int itemId = 0;
        std::string location;
        std::string binding;
        std::vector<int> upgrades;
        std::vector<int> infusions;
    };

    struct ItemInfo
    {
        int id = 0;
        std::string name;
        std::string rarity;
        std::string type;
        std::string detailType;
        bool noSalvage = false;
        bool accountBound = false;
    };

    struct PriceInfo
    {
        int buy = 0;
        int sell = 0;
        bool listed = false;
    };

    struct ResultRow
    {
        int gearId = 0;
        std::string gearName;
        std::string gearRarity;
        std::string location;
        std::string binding;
        bool gearNoSalvage = false;

        int upgradeId = 0;
        std::string upgradeName;
        UpgradeKind kind = UpgradeKind::Other;
        bool upgradeAccountBound = false;

        int instantSell = 0;
        int listing = 0;
        int netListing = 0;
        bool hasPrice = false;
    };

    struct ScanResult
    {
        std::vector<ResultRow> rows;
        std::string accountName;
        std::string status;
        std::string error;
    };
}
