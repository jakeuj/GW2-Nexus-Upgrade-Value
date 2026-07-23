# Upgrade Value Repository Contract

Read the relevant sections before changing a covered surface. Derive current values from source; examples here describe the contract, not a substitute for inspection.

## Source map

| Surface | Authoritative files |
|---|---|
| Addon metadata and lifecycle | `src/entry.cpp` |
| UI, worker, fonts, settings flow | `src/App.cpp`, `src/App.h` |
| GW2 API requests and calculations | `src/Gw2Api.cpp`, `src/Gw2Api.h` |
| HTTPS transport and bearer header | `src/HttpClient.cpp`, `src/HttpClient.h` |
| DPAPI persistence | `src/Settings.cpp`, `src/Settings.h` |
| Data structures | `src/Models.h` |
| Build target and source list | `UpgradeValue.vcxproj`, `UpgradeValue.sln` |
| Release automation | `.github/workflows/build-and-release.yml` |
| User documentation | `README.md`, `README.zh-Hant.md`, `docs/` |
| Review assertions | `NEXUS_REVIEW.md`, `NEXUS_REVIEW.zh-Hant.md` |

## Metadata and Raidcore submission

Treat `GetAddonDef()` as authoritative for the signature, API version, name, version, author, description, provider, and update link.

- Convert the signed signature to the Raidcore field with `f"0x{value & 0xFFFFFFFF:08X}"`.
- Enter the required `0x` prefix followed by exactly eight hexadecimal digits, for example `0xFE722E33`. The four-byte/eight-character validation applies to the digits after the prefix. Do not include a minus sign, spaces, quotes, or Markdown backticks.
- Verify a newly chosen signature against Raidcore's registry. Static repository checks can validate width and representation, but not ecosystem-wide uniqueness.
- Keep the Raidcore name and short summary aligned with `Definition.Name` and `Definition.Description`.
- Prefer the slug `upgrade-value` while it remains available.
- Use `screenshots/upgrade-value-main.png` as the default cover candidate.
- Use `https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/releases/latest/download/UpgradeValue.dll` for the direct DLL URL.
- Use the repository URL for source code and `https://gw2-value.jakeuj.com/` for the website.
- Treat Discord, Patreon, and Ko-fi as optional owner-controlled links; never invent them.
- Use `Public` for a public listing submission, understanding that Raidcore approval still controls public accessibility.

## Security and policy boundary

Preserve these claims unless implementation and review documents are deliberately changed together:

- Use only the public Nexus API; do not add memory scanning, detours, hooks, packet interception, or native GW2 UI hooks.
- Do not simulate input, automate gameplay, salvage items, trade, or enable unattended play.
- Limit addon-owned traffic to HTTPS `GET` requests against `api.guildwars2.com`.
- Send the API key only in an `Authorization: Bearer` header.
- Require only `account`, `inventories`, and `characters` permissions.
- Encrypt the API key with Windows DPAPI before writing `settings.json`.
- Keep scan results and account-derived data in memory; do not add telemetry or analytics.
- Run network and JSON work on the owned worker thread, cancel it, and join it during unload.
- Pair renderer, input-bind, close-on-Escape, and font registrations with their cleanup calls.

Any new network destination, permission, persisted account data, updater, hook, automation, closed-source component, or game-process access requires explicit review and synchronized changes to both Nexus review documents.

## Localization contract

- Supply Traditional Chinese and English for every visible label, status, error, tooltip, and settings explanation.
- Use the existing `T(zh, en)` selection pattern.
- Request official Chinese names with `lang=zh`, then retain the Windows conversion to Traditional Chinese.
- Acquire and release Nexus `FONT_DEFAULT`; before enabling Traditional Chinese, verify that the selected font actually contains representative CJK glyphs instead of assuming the requested atlas range guarantees coverage.
- Keep `README.md` and `README.zh-Hant.md` structurally aligned; do the same for both `NEXUS_REVIEW` files.

## Version and release checklist

1. Set `Definition.Version` to `{major, minor, patch, 0}` in `src/entry.cpp`.
2. Update the current stable version and version-specific ZIP links in both READMEs. Preserve historical compatibility statements when they intentionally name an older milestone.
3. Update the reviewed addon version in both Nexus review documents.
4. Update every current-version label in `docs/index.html`; do not alter unrelated historical version text.
5. Run the invariant checker and `git diff --check`.
6. Build `Release|x64` on Windows and verify `bin/Release/UpgradeValue.dll` is non-empty.
7. Tag `vX.Y.Z`, or dispatch the release workflow with that version. The workflow rejects a tag that differs from `Definition.Version`.
8. Confirm the GitHub Release contains exactly the stable `UpgradeValue.dll` asset plus `UpgradeValue-vX.Y.Z.zip`.
9. Confirm the stable latest-download URL resolves to the new DLL.

Do not commit DLL, executable, object, static-library, or ZIP build output.

## In-game smoke test

After a build-affecting change or before release, verify on Windows with Guild Wars 2 and Nexus:

1. Load the addon and confirm no Nexus error is reported.
2. Open and close the main window with `Alt + Shift + U` and Escape.
3. Save settings, restart or reload, and confirm persistence without exposing the API key.
4. Test a missing permission or invalid key and confirm a useful, non-secret error.
5. Switch between English and Traditional Chinese and inspect glyph rendering.
6. Refresh data and verify bank, shared inventory, character inventory/equipment, price columns, and recommendations.
7. Unload or reload the addon and confirm the worker exits and registrations/fonts are released cleanly.

## Documentation update triggers

| Change | Documentation to reconcile |
|---|---|
| UI, shortcut, settings, recommendation | Both READMEs and site content/screenshots when visible |
| Endpoint, permission, host, authentication | Both READMEs and both Nexus review documents |
| Storage, DPAPI, logs, caching | Both Nexus review documents; README privacy claims when user-facing |
| Registration, threading, unload, fonts | Both Nexus review documents |
| Version or release asset | `entry.cpp`, both READMEs, both review documents, `docs/index.html`, release workflow assumptions |
| Hook, automation, memory/process access | Full ToS review and both Nexus review documents |
| AI-assisted public content | Preserve or update the AI Notice disclosure |
