# Upgrade Value Repository Contract

Read the relevant sections before changing a covered surface. Derive current values from source; examples here describe the contract, not a substitute for inspection.

## Contents

- [Source map](#source-map)
- [Metadata and Raidcore submission](#metadata-and-raidcore-submission)
- [Security and policy boundary](#security-and-policy-boundary)
- [Localization contract](#localization-contract)
- [Results-table contract](#results-table-contract)
- [Version and release checklist](#version-and-release-checklist)
- [In-game smoke test](#in-game-smoke-test)
- [Documentation update triggers](#documentation-update-triggers)

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
- Keep the language selector and missing-glyph warning readable without CJK. When a selected or persisted Chinese mode loses glyph support, persist the English fallback, invalidate the in-flight Chinese scan, clear stale rows, and schedule one English scan so cancelled results cannot overwrite the fallback.
- Keep `README.md` and `README.zh-Hant.md` structurally aligned; do the same for both `NEXUS_REVIEW` files.

## Results-table contract

- Filter search results before applying display ordering.
- Derive the active decision value from the current recommendation mode: use `Instant sell` when net listing is disabled and `Net listing` when it is enabled. Sort that value descending globally and within every Location group; zero or unavailable values therefore remain at the end.
- Break equal decision values deterministically by full Location, Upgrade name, Equipment name, upgrade ID, then equipment ID.
- Derive the Location group from its visible owner or storage area: collapse `Account Bank #n` to `Account Bank`, collapse `Shared Inventory #n` to `Shared Inventory`, use the prefix before ` / ` for character locations, and fall back to the complete Location for unknown formats.
- Preserve global active-value ordering as the unsorted state.
- Limit user sorting to the Location column unless a requested change explicitly expands the table contract.
- Keep Location sorting tri-state: ascending, descending, then unsorted.
- Sort Location by its visible group name; descending reverses group order only. Use stable sorting so every group retains active-value-descending order.
- Apply the selected sort after search filtering without resetting the sort direction.
- Re-evaluate display ordering immediately when the recommendation mode changes; changing the mode must not require another API scan.

## Version and release checklist

1. Set `Definition.Version` to `{major, minor, patch, 0}` in `src/entry.cpp`.
2. Update the current stable version and version-specific ZIP links in both READMEs. Preserve historical compatibility statements when they intentionally name an older milestone.
3. Update the reviewed addon version in both Nexus review documents.
4. Update every current-version label in `docs/index.html`; do not alter unrelated historical version text.
5. Run the invariant checker and `git diff --check`.
6. Build `Release|x64` on Windows and verify `bin/Release/UpgradeValue.dll` is non-empty. On macOS, use a pull request to trigger the Windows workflow and test its artifact in CrossOver.
7. Use `workflow_dispatch` only when explicitly intending to create an official tag and Release; never use it as a test build. Prefer an annotated `vX.Y.Z` tag after the pull request and runtime tests pass.
8. Wait for the tag workflow and confirm the GitHub Release contains exactly the stable `UpgradeValue.dll` asset plus `UpgradeValue-vX.Y.Z.zip`.
9. Confirm the standalone DLL and the DLL inside the ZIP are identical, and confirm the stable latest-download URL resolves to the same SHA-256.
10. Publish or close the related issue only after the release and latest-download checks pass.

Do not commit DLL, executable, object, static-library, or ZIP build output.

## In-game smoke test

After a build-affecting change or before release, verify on Windows with Guild Wars 2 and Nexus:

1. Press the Nexus Addon Library `Load` action when needed, then confirm the expected addon version and a successful load entry in `addons/Nexus/Nexus.log`. Selecting the addon row is not evidence of loading.
2. Open and close the main window with Escape and the currently configured keybind; do not assume the documented default is unchanged in the test environment.
3. Save settings, restart or reload, and confirm persistence without exposing the API key.
4. Test a missing permission or invalid key and confirm a useful, non-secret error.
5. With the stock Inter font, verify English default, disabled Traditional Chinese, readable CJK guidance, and migration from a persisted `chineseUi: true`.
6. With a locally supplied CJK-capable Nexus font, switch between English and Traditional Chinese, restart, and inspect glyph rendering. Do not commit or distribute the font.
7. Start a Chinese refresh and switch back to a non-CJK font; confirm cancellation, immediate English fallback, cleared stale rows, and one final English result set.
8. Verify Location group ascending, group descending, global-value restoration, per-group value priority, selected-value switching, deterministic ties, and search-filter persistence.
9. Refresh data and verify bank, shared inventory, character inventory/equipment, price columns, and recommendations.
10. Unload or reload during a scan and confirm the worker exits and registrations/fonts are released cleanly.

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

Use only real Nexus/Guild Wars 2 screenshots for user-facing addon UI. Mask secrets, exclude settings and logs from public evidence, update `screenshots/` fallbacks and every website AVIF/WebP derivative, and verify image dimensions and alt text.
