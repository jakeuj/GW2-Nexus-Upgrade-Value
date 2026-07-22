---
name: maintain-upgrade-value
description: Maintain the Upgrade Value Guild Wars 2 Nexus C++ addon in this repository. Use when Codex changes src/, GetAddonDef or AddonDefinition metadata, Nexus lifecycle registration, GW2 API endpoints, DPAPI settings, English or Traditional Chinese UI, README/NEXUS_REVIEW/docs, build and release automation, Raidcore submission fields, addon signature conversion, or versioned release packaging.
---

# Maintain Upgrade Value

Work from the repository root containing `UpgradeValue.sln`. Preserve the public Nexus API boundary, the documented privacy model, and synchronization between runtime metadata, documentation, and release assets.

## Start every task

1. Read the files directly involved in the request; do not infer current values from this skill.
2. Read [references/repository-contract.md](references/repository-contract.md) when changing metadata, networking, storage, lifecycle, localization, documentation, releases, or Raidcore submission data.
3. Run the invariant checker before and after material changes:

   ```bash
   python3 .agents/skills/maintain-upgrade-value/scripts/check_repo_invariants.py
   ```

4. Treat `vendor/` as third-party source. Change it only for an explicit dependency update.

## Route changes by responsibility

- Edit `src/entry.cpp` for `GetAddonDef`, version, signature, author, description, update provider, and Nexus registration lifecycle.
- Edit `src/App.cpp` and `src/App.h` for UI, background work, settings flow, fonts, keybind-facing behavior, and recommendation presentation.
- Edit `src/Gw2Api.cpp` and `src/Gw2Api.h` for official GW2 API endpoints, permission validation, parsing, and price calculations.
- Edit `src/HttpClient.cpp` and `src/HttpClient.h` for WinHTTP transport. Keep authenticated data in the `Authorization: Bearer` header.
- Edit `src/Settings.cpp` and `src/Settings.h` for local persistence and Windows DPAPI handling.
- Update both English and Traditional Chinese documentation when user-visible behavior or review claims change.

## Preserve hard invariants

- Keep the addon signature a negative signed 32-bit integer. For Raidcore, format the field as `0x` followed by `signature & 0xFFFFFFFF` rendered as exactly eight uppercase hexadecimal digits, for example `0xFE722E33`. The eight-digit rule excludes the required `0x` prefix. Do not include whitespace or a minus sign. If changing the signature, verify uniqueness with Raidcore; the local checker cannot prove registry-wide uniqueness.
- Keep every Nexus acquire/register operation paired with release/deregister during unload. Stop and join owned background work before destroying addon state.
- Keep HTTP and JSON work off the render callback. Preserve bounded WinHTTP timeouts.
- Never log, place in a URL, or persist the API key in plaintext. Preserve DPAPI encryption and the documented permissions: `account`, `inventories`, and `characters`.
- Keep addon-owned requests limited to HTTPS `GET` calls to `api.guildwars2.com` unless the user explicitly expands the network surface. Treat a new host, permission, persistence behavior, hook, automation feature, or game-memory access as a policy-review change.
- Add every user-visible string in both Traditional Chinese and English through the existing `T(zh, en)` pattern. Preserve `FONT_DEFAULT`, `lang=zh`, and Simplified-to-Traditional conversion behavior.
- Do not claim ArenaNet approval. Keep the AI-assistance disclosure when editing the review material.

## Synchronize related surfaces

When changing the release version, update `Definition.Version`, both READMEs, both Nexus review documents, and current-version text in `docs/index.html` in one change. Keep the stable direct-download URL pointed at `releases/latest/download/UpgradeValue.dll`; reserve version-pinned URLs for version-specific ZIP references. Do not mechanically replace historical compatibility statements such as the version that first introduced Nexus updating.

When behavior changes, update documentation by evidence rather than aspiration. In particular, reconcile:

- API endpoints, permissions, and network destinations;
- settings location, secret handling, and persisted data;
- Nexus registrations, fonts, threading, and unload behavior;
- user-visible features, shortcuts, screenshots, and installation steps;
- ToS-relevant automation, hooks, memory access, and review disclosures.

## Validate proportionally

Always run:

```bash
python3 .agents/skills/maintain-upgrade-value/scripts/check_repo_invariants.py
git diff --check
```

For build-affecting changes, build on Windows:

```powershell
msbuild UpgradeValue.sln /m /p:Configuration=Release /p:Platform=x64
```

Confirm the non-empty output at `bin/Release/UpgradeValue.dll`. On non-Windows hosts, report that the native build was not run; do not imply compilation success from static checks alone.

After a successful Windows build, smoke-test loading, opening with `Alt + Shift + U`, settings save/reload, API permission errors, English/Traditional Chinese switching, refresh and price display, Escape/keybind behavior, and clean unload/reload without a stuck worker or stale registration.

For a release, follow the release checklist in [references/repository-contract.md](references/repository-contract.md) and confirm the tag version matches `Definition.Version` before publishing.
