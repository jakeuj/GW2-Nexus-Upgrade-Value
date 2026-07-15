#pragma once

#include <atomic>
#include <string>

#include "Models.h"

namespace UpgradeValue
{
    class Gw2Api
    {
    public:
        ScanResult Scan(const std::string& apiKey,
                        bool includeInfusions,
                        bool chineseNames,
                        const std::atomic<bool>& stopRequested) const;
    };
}
