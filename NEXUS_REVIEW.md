# Nexus Submission Review Notes

**English** · [繁體中文](NEXUS_REVIEW.zh-Hant.md)

This document gives Raidcore Nexus reviewers a source-backed summary of Upgrade Value's behavior and review surface. It describes addon version `1.0.3.0`, built against the vendored Nexus API version `6`.

## Reviewer summary

| Review item | Implementation |
|---|---|
| Reverse-engineered code | **None.** The addon does not contain reverse-engineered first-party code, so the requirement to keep such code closed source is not applicable. |
| First-party source availability | All project-owned code required to build the distributed addon is present in this public repository. There is no private service, private library, companion executable, obfuscated payload, or omitted first-party module. |
| Game-process access | Uses the public Nexus `AddonAPI` only. There is no direct game-memory read/write, signature scanning, detouring, function hooking, packet interception, or native Guild Wars 2 UI hook. |
| Gameplay automation | **None.** The addon does not simulate input, activate skills, move the character, interact with inventory UI, salvage items, trade, or enable unattended play. |
| Information provided | Displays official account API snapshots, public item data, Trading Post prices, and a configurable salvage recommendation. All resulting game actions remain manual. |
| Addon-owned network traffic | HTTPS `GET` requests to `api.guildwars2.com` only. |
| Telemetry and advertising | None. No analytics, crash reporting, tracking, advertising, or developer-operated backend is used. |
| Local secret storage | The Guild Wars 2 API key is encrypted with Windows DPAPI before it is written to `settings.json`. |
| Updates | Declares the Nexus GitHub update provider. Nexus, not addon-owned downloader code, handles release checks and installation. |
| AI assistance | Codex assisted with project documentation and development review. This is disclosed for Raidcore's **AI Notice** category; the developer remains responsible for all review, testing, maintenance, and compliance. |

## What the addon does

1. If a saved API key exists, the addon starts a background scan after loading. Users can also start a scan with **Refresh** or **Save and scan**.
2. It validates the key and reads account, bank, shared inventory, character inventory, and currently equipped-item snapshots from the official Guild Wars 2 v2 API.
3. It reads embedded upgrade IDs from the API's `upgrades[]` and optional `infusions[]` fields.
4. It requests public item metadata and Trading Post prices from the official API.
5. It displays the results in an ImGui table and calculates a recommendation from the user-selected market value and threshold.

The addon does not call any API that changes account state, and it does not perform an in-game action on the player's behalf.

## ToS-relevant review point

The feature most relevant to a policy review is the recommendation column. It labels a result as **Use Black Lion** or **Normal salvage** by comparing an embedded upgrade's official Trading Post price with a user-configured threshold.

This calculation may make an economic decision more convenient, but it is informational only:

- It is based on official API snapshots rather than game-memory inspection.
- It cannot select an item or activate a salvage kit.
- It cannot inject or simulate mouse, keyboard, or controller input.
- It cannot play while the user is absent or obtain rewards by itself.
- It does not expose combat, enemy, targeting, positioning, or encounter information.

Final acceptance remains a decision for the Nexus reviewer. ArenaNet also states that it does not review, approve, or endorse individual third-party programs and retains discretion over their use.

## AI Notice

Codex was used to assist with project documentation and development review. AI-assisted output is treated as reviewable project content rather than an authority on correctness or compliance.

- The developer reviews and accepts responsibility for all published content and shipped behavior.
- AI assistance does not replace source review, builds, in-game testing, security review, or Raidcore's policy and ToS assessment.
- This disclosure is included so Raidcore can apply the **AI Notice** classification if required.

## Reverse engineering and source policy

No reverse-engineered implementation is present in this repository or linked by the addon.

Specifically, the project has no:

- disassembly-derived game functions, offsets, signatures, or structures;
- memory scanner, injector, detour, or hook library;
- packet capture or protocol interception;
- bundled closed-source DLL, executable, service, or remote private API;
- hidden build step that retrieves a private first-party component.

The vendored source consists of the public Raidcore Nexus header, Raidcore's Dear ImGui fork, and nlohmann/json, each with its included license. Generated build outputs are excluded from Git and are built by the public GitHub Actions workflow.

## Nexus integration surface

| Public Nexus API surface | Use |
|---|---|
| `Renderer.Register` / `Deregister` | Main overlay and Nexus options rendering |
| `InputBinds.RegisterWithString` / `Deregister` | One toggle keybind, default `Alt + Shift + U` |
| `UI.RegisterCloseOnEscape` / `DeregisterCloseOnEscape` | Closes the addon window with Escape |
| `Paths.GetAddonDirectory` | Selects the writable settings directory |
| `Fonts.Get` / `Release` | Uses Nexus `FONT_DEFAULT` for Traditional Chinese glyphs |
| `Log` | Writes load and unload messages |
| GitHub update provider metadata | Allows Nexus to manage updates from this public repository |

The addon does not use Nexus events, WndProc callbacks, DataLink, MumbleLink, Quick Access, textures, input interception, or self-managed updater requests.

Registration and release pairs are visible in [`src/entry.cpp`](src/entry.cpp) and [`src/App.cpp`](src/App.cpp).

## Network surface

Addon-owned HTTP behavior is implemented in [`src/HttpClient.cpp`](src/HttpClient.cpp). The host is hard-coded to `api.guildwars2.com`, the port is HTTPS `443`, and requests use Windows WinHTTP with `WINHTTP_FLAG_SECURE`.

| Endpoint | Authentication | Purpose |
|---|---|---|
| `/v2/tokeninfo` | Bearer API key | Validate permissions |
| `/v2/account` | Bearer API key | Display the account name |
| `/v2/account/bank` | Bearer API key | Read account-bank item instances |
| `/v2/account/inventory` | Bearer API key | Read shared-inventory item instances |
| `/v2/characters?ids=all` | Bearer API key | Read character bags and current equipment |
| `/v2/items?ids=...` | None | Read public item metadata |
| `/v2/commerce/prices?ids=...` | None | Read public Trading Post prices |

Additional URL behavior:

- The API key is sent in an `Authorization: Bearer` header, never in a URL.
- Clicking the API-key-manager button opens `https://account.arena.net/applications` in the user's default browser.
- Clicking the author button opens `https://github.com/jakeuj` in the user's default browser.
- `GetAddonDef` points Nexus at this GitHub repository for updates. The addon contains no GitHub HTTP client or file replacement code.

There are no other network destinations in project-owned runtime code.

## Data handling and privacy

- Required API permissions are limited to `account`, `inventories`, and `characters`.
- The API key is protected with Windows `CryptProtectData` and stored as Base64-encoded DPAPI ciphertext.
- Settings are written to the directory returned by `Paths.GetAddonDirectory("UpgradeValue")`.
- Scan results, account name, character names, item locations, and prices are held in memory only and are not cached to disk.
- No account or usage data is transmitted to the developer or any third-party analytics service.
- The API key is not written to Nexus logs by this addon.

The local storage implementation is in [`src/Settings.cpp`](src/Settings.cpp).

## Threading and unload behavior

- Network and JSON work runs on one owned `std::thread`, outside the render callback.
- Shared results are protected by a mutex.
- Unload sets a cancellation flag and joins the worker before destroying addon state.
- WinHTTP requests use bounded connection, send, receive, and resolution timeouts. Unload may wait for the current bounded request to finish, but it does not detach the worker.
- All Nexus registrations and the acquired font are released during unload.

## Build and dependency review

- Target: Windows x64 DLL.
- Export: `GetAddonDef` with C linkage.
- Addon signature: `-26071501`.
- Addon version: `1.0.3.0`.
- Nexus API version: `6`.
- Update provider: Nexus GitHub provider.
- Build command:

  ```powershell
  msbuild UpgradeValue.sln /m /p:Configuration=Release /p:Platform=x64
  ```

- CI definition: [`.github/workflows/build-and-release.yml`](.github/workflows/build-and-release.yml)
- Project definition: [`UpgradeValue.vcxproj`](UpgradeValue.vcxproj)
- Third-party licenses: [`vendor/imgui/LICENSE.txt`](vendor/imgui/LICENSE.txt), [`vendor/nexus/LICENSE`](vendor/nexus/LICENSE), and [`vendor/nlohmann_json/LICENSE.MIT`](vendor/nlohmann_json/LICENSE.MIT)

No compiled DLL, executable, static library, object file, or ZIP is tracked in the repository.

## Reviewer code map

| File | Primary review purpose |
|---|---|
| [`src/entry.cpp`](src/entry.cpp) | Addon definition, Nexus API registration, lifecycle, updater metadata |
| [`src/App.cpp`](src/App.cpp) | UI, recommendations, background worker, font and settings lifecycle |
| [`src/Gw2Api.cpp`](src/Gw2Api.cpp) | Exact API endpoints, account-data parsing, price calculation |
| [`src/HttpClient.cpp`](src/HttpClient.cpp) | Fixed network host, HTTPS transport, bearer header |
| [`src/Settings.cpp`](src/Settings.cpp) | DPAPI encryption and local settings storage |
| [`UpgradeValue.vcxproj`](UpgradeValue.vcxproj) | Build target, source list, compiler and linker settings |

## Policy references

- [ArenaNet policy on third-party programs](https://help.guildwars2.com/hc/en-us/articles/360013625034-Policy-Third-Party-Programs)
- [Guild Wars 2 API overview and terms](https://wiki.guildwars2.com/wiki/API:Main)
- [Raidcore `AddonDefinition` documentation](https://docs.raidcore.gg/structAddonDefinition__t.html)
- [Raidcore Nexus API header](https://github.com/RaidcoreGG/Nexus-API/blob/main/Nexus.h)

This document should be updated and the addon should be re-reviewed if a future version adds game-memory access, native game hooks, input automation, new network destinations, new API permissions, persistent account-data caching, or a private/closed-source component.
