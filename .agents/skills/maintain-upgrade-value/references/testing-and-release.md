# macOS、CrossOver 與發布流程

Read this reference only for Windows artifacts, CrossOver runtime tests, screenshots, or releases. Derive current workflow names, paths, versions, and keybinds from the repository and test environment instead of copying historical values.

## Platform boundary

- Treat the deliverable as an MSVC/Windows SDK x64 DLL. Do not claim that macOS static checks compile it.
- Do not introduce MinGW or a macOS cross-build as a release gate unless the user explicitly requests and accepts the ABI risk.
- Use Windows GitHub Actions for compilation evidence. Use CrossOver for Nexus/Guild Wars 2 runtime evidence when the user accepts it instead of native Windows hardware.

## Preflight and Windows artifact

1. Start from the intended base branch and isolate the release changes.
2. Run the repository invariant checker, `git diff --check`, and the website audit when `docs/` changes.
3. Push a branch and open a pull request to trigger the current `pull_request` workflow.
4. Do not use `workflow_dispatch` for a test build when the workflow creates a tag or Release.
5. Download the successful Actions artifact and record:
   - pull request and run URLs;
   - artifact name and source commit;
   - DLL size and SHA-256;
   - `file` output showing PE32+ x86-64;
   - a COFF export check showing `GetAddonDef`.
6. Reject an empty DLL, the wrong architecture, a missing export, or an artifact from a different source commit.

Useful macOS checks:

```bash
file UpgradeValue.dll
llvm-readobj --coff-exports UpgradeValue.dll
shasum -a 256 UpgradeValue.dll
```

## CrossOver installation safety

1. Resolve the active Guild Wars 2 bottle and installation directory; do not assume a path from old notes.
2. Inspect the existing addon filename. Addon Library installations may use `addons/Upgrade_Value.dll`, while manual documentation uses `addons/UpgradeValue.dll`.
3. Replace the active filename in place. Never leave both filenames or another copy of the same signed addon under `addons/`.
4. Fully close Guild Wars 2 and the bottle before replacing a DLL or editing settings.
5. Copy the previous DLL and settings to a timestamped directory outside `addons/`. Do not leave a backup DLL where Nexus can discover it.
6. Preserve `apiKeyProtected`; never print, upload, screenshot, or copy a plaintext API key. Inspect settings by key names and safe metadata only.
7. Test Nexus itself first. Confirm its UI opens and `addons/Nexus/Nexus.log` is produced before diagnosing the addon.
8. Select the addon and press `Load` when it is not already loaded. Confirm the expected version in Addon Library and a successful addon load line in the Nexus log.

## Runtime matrix

Record pass/fail evidence for:

1. **Fresh settings with stock Inter**
   - English is selected by default.
   - UI and item names contain no replacement `?`.
   - Traditional Chinese is disabled with readable Nexus Options → Style guidance.
2. **Legacy Chinese setting**
   - Set only `chineseUi` to `true` while preserving `apiKeyProtected`.
   - Restart with Inter and confirm English fallback plus persisted `chineseUi: false`.
3. **CJK-capable Nexus font**
   - Add a locally licensed system font to the Nexus Fonts directory for testing only.
   - Enable Traditional Chinese, scan with Chinese names, restart, and confirm persistence.
   - Do not commit or distribute the test font.
4. **Runtime font downgrade**
   - Start a Chinese refresh, immediately switch Nexus back to a non-CJK font, and wait beyond the slowest expected request.
   - Confirm immediate English fallback, stale-row clearing, cancelled Chinese work, and one final English result set.
5. **Location sorting**
   - Capture the default first rows and confirm they follow the active decision value globally.
   - Verify ascending groups, descending groups, and third-click global-value restoration.
   - Confirm account bank, shared inventory, and each character form distinct groups whose rows remain active-value descending.
   - Toggle the net-listing decision mode and confirm immediate global and per-group reordering without a refresh.
   - Confirm zero values sort last and equal values use the deterministic Location, Upgrade, Equipment, and ID tie-breakers.
   - Apply and clear a search filter without losing the selected direction.
6. **General smoke**
   - Save and reload the protected key.
   - Test invalid or missing permissions without leaking the key.
   - Verify refresh, Escape, the current keybind, normal unload/reload, and unload during a scan.

Use the test environment's configured keybind. The default documentation does not override a user's Nexus customization.

## Screenshots and evidence

- Capture a real Settings view showing English, the disabled Traditional Chinese option, and the CJK hint.
- Capture a real Main view showing the Location sort arrow and grouped rows.
- Mask the API key and avoid public logs, settings files, unintended account identifiers, or other secrets. Include an owner-approved public account ID only when it is already part of the project documentation.
- Crop mechanically; do not fabricate an in-game UI.
- Update `screenshots/`, website PNG fallbacks, AVIF/WebP derivatives, intrinsic dimensions, and alt text together.
- Record the CrossOver version, bottle type, Nexus version, Actions run, artifact name, hash, and matrix results in a repository test record or pull-request body.

## Release gate

1. Merge only after the Windows build, accepted runtime matrix, real screenshots, invariants, and relevant website checks pass.
2. Confirm the source version, then create and push an annotated `vX.Y.Z` tag. Do not also dispatch a second release workflow.
3. Wait for the tag run to finish successfully.
4. Confirm the Release is neither draft nor prerelease and contains exactly:
   - `UpgradeValue.dll`
   - `UpgradeValue-vX.Y.Z.zip`
5. Download both assets. Confirm:
   - the DLL is non-empty PE32+ x86-64 and exports `GetAddonDef`;
   - the ZIP contains only `UpgradeValue.dll`;
   - the ZIP's DLL hash equals the standalone DLL hash;
   - `releases/latest/download/UpgradeValue.dll` has the same hash.
6. Do not replace a DLL while Guild Wars 2 or CrossOver is still running. If a post-release live load is skipped for safety, record that fact without claiming it passed.
7. Update the related issue with the fixed behavior and download link, then close it only after the release checks succeed.
8. Confirm the final branch is clean and any Pages build is successful when release documentation or website assets changed.
