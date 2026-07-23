# Upgrade Value — GW2 Nexus Addon

**English** · [繁體中文](README.zh-Hant.md)

[![Build and release](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/workflows/build-and-release.yml/badge.svg)](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/workflows/build-and-release.yml)
[![GitHub Release](https://img.shields.io/github/v/release/jakeuj/GW2-Nexus-Upgrade-Value)](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/releases/latest)

Scan Exotic equipment across your Guild Wars 2 account, list embedded runes and sigils with live Trading Post prices, and decide whether they are worth extracting with a Black Lion Salvage Kit.

Current stable release: [**v1.0.3**](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/releases/tag/v1.0.3)

Project website: [gw2-value.jakeuj.com](https://gw2-value.jakeuj.com/)

## Nexus Addon Library and review

Upgrade Value `v1.0.3` completed Raidcore's initial review and is now publicly listed in the Nexus Addon Library as [listing ID 128](https://raidcore.gg/gw2/addons/upgrade-value). Players can find and install it directly from the in-game library.

For the source-backed review scope, see the [Nexus submission review notes](NEXUS_REVIEW.md).

The short version: this project contains no reverse-engineered code, no private first-party component, no game-memory access, and no gameplay automation. Guild Wars 2 data is read only through the official v2 API, while in-game integration is limited to the public Nexus API. The review notes document the complete API, network, storage, lifecycle, and ToS-relevant surface for independent verification.

Codex assisted with project documentation and development review. This assistance is disclosed for Raidcore's **AI Notice** category; the developer remains responsible for reviewing, testing, maintaining, and publishing the addon.

> [!NOTE]
> Publication in the Nexus Addon Library records Raidcore's initial review. It does not claim ArenaNet approval or make this community addon an official ArenaNet or Raidcore release.

## Manual download

- [Download `UpgradeValue.dll` directly](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/releases/latest/download/UpgradeValue.dll)
- [Download `UpgradeValue-v1.0.3.zip`](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/releases/download/v1.0.3/UpgradeValue-v1.0.3.zip)
- [View all releases and release notes](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/releases)

> [!IMPORTANT]
> Manual installation is optional. If you download the DLL directly, place `UpgradeValue.dll` in the Guild Wars 2 `addons` root directory. Do not place it inside the `addons\Nexus` subdirectory.

> [!NOTE]
> Starting with `v1.0.3`, Nexus can check GitHub Releases for later versions and install them. Versions `v1.0.2` and earlier cannot discover this update automatically, so you must manually install `v1.0.3` once. Future automatic updates depend on the user's Nexus update settings.

## Author

- GitHub: [@jakeuj](https://github.com/jakeuj)
- Guild Wars 2 ID: `jakeuj.5260`
- Full details: [AUTHORS.md](AUTHORS.md)

## Screenshots

### Upgrade values and salvage recommendations

![Upgrade Value main window](screenshots/upgrade-value-main.png)

The main window lists embedded upgrades, their equipment and location, instant-sell prices, lowest listings, and listing values after the 15% Trading Post fee.

### Nexus settings

![Upgrade Value Nexus settings](screenshots/upgrade-value-settings.png)

Enter an API key, enable Traditional Chinese, choose whether to display infusions, and configure the keybind in the Nexus settings.

## Features

- Scans the account bank, shared inventory slots, every character's bags, and currently equipped items.
- Reads embedded runes and sigils from each item instance's `upgrades[]` field.
- Traditional Chinese mode requests official API names with `lang=zh` and converts them to Traditional Chinese through Windows; English mode uses the official English names.
- Queries the official `/v2/commerce/prices` endpoint in batches:
  - `Instant sell`: current highest buy order.
  - `Listing`: current lowest sell listing.
  - `Net listing`: estimated value after the 15% Trading Post fee.
- Provides a configurable Black Lion salvage threshold, with a default of 50 silver.
- Clearly marks items that cannot be salvaged, upgrades without Trading Post prices, and Legendary equipment that should never be salvaged.
- Can display infusion prices. Black Lion Salvage Kits do not recover infusions, so the addon never recommends using one for them.
- Encrypts the API key with Windows DPAPI before saving it locally; the key is never stored in plaintext.
- Performs all network requests in the background without blocking the game.
- Uses the complete Chinese glyph range from Nexus `FONT_DEFAULT` to prevent Traditional Chinese text from appearing as `?`.

## Important distinctions

- The orange equipment rarity in Guild Wars 2 is `Exotic`.
- `Legendary` equipment is purple and should never be salvaged. The addon displays a warning and never recommends salvaging Legendary items.
- The public Nexus SDK does not expose native Guild Wars 2 item-tooltip hover events. This version displays prices in the addon's own table and hover tooltips without relying on unstable game-memory hooks.

## Installation

1. Install [Raidcore Nexus](https://raidcore.gg/gw2/nexus).
2. Launch Guild Wars 2, open the Nexus in-game Addon Library, and search for **Upgrade Value**.
3. Install and load the addon from the library.
4. Create a Guild Wars 2 API key with at least these permissions:

   - `account`
   - `inventories`
   - `characters`

5. Open the **Upgrade Value** settings, paste the key, and select **Save and scan**.

API key management: <https://account.arena.net/applications>

For a manual fallback, download `UpgradeValue.dll` from the [latest release](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/releases/latest) and copy it into the Guild Wars 2 `addons` root directory:

```text
F:\SteamLibrary\steamapps\common\Guild Wars 2\addons\UpgradeValue.dll
```

Do not place it inside the `addons\Nexus` subdirectory.

## Usage

1. Press `Alt + Shift + U` to open or close the main window.
2. Select **Refresh** to retrieve the current account equipment and latest Trading Post prices.
3. Set the **Black Lion threshold (silver)**; the default is 50 silver.
4. Enable **Use net listing value** to base recommendations on the net listing value; otherwise, the addon uses the instant-sell value.
5. Use the search field to filter by equipment, upgrade name, or location.

The Recommendation column may display:

- `Use Black Lion`: the embedded rune or sigil value meets the configured threshold.
- `Normal salvage`: the upgrade value is below the threshold.
- `Not salvageable`: the item has the `NoSalvage` restriction.
- `No TP price`: the upgrade is account-bound or has no valid listing.
- `BL kit won't recover`: infusions are not recovered by Black Lion Salvage Kits.
- `Never salvage`: a safety warning for Legendary equipment.

## Building

Requirements: Visual Studio 2022, Desktop development with C++, and the Windows 10/11 SDK.

```powershell
msbuild UpgradeValue.sln /m /p:Configuration=Release /p:Platform=x64
```

Output: `bin/Release/UpgradeValue.dll`

The repository includes the Nexus API header, the Raidcore ImGui fork, and nlohmann/json required for building.

## CI/CD and releases

- [Build and release](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/workflows/build-and-release.yml) automatically runs a Windows x64 Release build and uploads artifacts when changes are pushed to `main` or a pull request is created.
- Pushing a tag that matches `v*`, such as `v1.0.3`, automatically creates a GitHub Release with the DLL and ZIP files.
- You can also open **Actions → Build and release → Run workflow** and enter a version to publish manually. The version must match the addon version in `src/entry.cpp`.
- GitHub Release Notes are generated automatically from the commits included in the release.

## Data sources and security

- Account items: `https://api.guildwars2.com/v2/account/bank`, `/account/inventory`, and `/characters?ids=all`
- Item data: `/v2/items?ids=...&lang=zh`
- Price data: `/v2/commerce/prices?ids=...`
- The API key is sent through the `Authorization: Bearer` header and only to `api.guildwars2.com`.

## Known limitations

- The official API provides account snapshots and does not push updates immediately when items move. Select **Refresh** to update the data.
- The addon currently reads each character's active equipment and bags but does not expand every inactive equipment template.
- Recommendations are based only on the embedded upgrade's market-value threshold. They do not calculate the equipment's own value, Globs of Ectoplasm, or the subjective opportunity cost of one Black Lion Salvage Kit.

## License

Copyright © 2026 [jakeuj](https://github.com/jakeuj).

This project's source code is licensed under the MIT License. Third-party components under `vendor/` are used under their respective bundled licenses.
