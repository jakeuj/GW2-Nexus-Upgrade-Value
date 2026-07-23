#!/usr/bin/env python3
"""Read-only consistency checks for the Upgrade Value Nexus addon repository."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


REQUIRED_FILES = (
    "src/entry.cpp",
    "src/App.cpp",
    "src/App.h",
    "src/Gw2Api.cpp",
    "src/HttpClient.cpp",
    "src/Settings.cpp",
    "src/Settings.h",
    "vendor/nexus/Nexus.h",
    "UpgradeValue.vcxproj",
    ".github/workflows/build-and-release.yml",
    "README.md",
    "README.zh-Hant.md",
    "NEXUS_REVIEW.md",
    "NEXUS_REVIEW.zh-Hant.md",
    "docs/index.html",
)


def parse_args() -> argparse.Namespace:
    default_root = Path(__file__).resolve().parents[4]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "root",
        nargs="?",
        type=Path,
        default=default_root,
        help="repository root (defaults to the root containing this repo-local skill)",
    )
    return parser.parse_args()


def read_required(root: Path, relative: str, errors: list[str]) -> str:
    path = root / relative
    if not path.is_file():
        errors.append(f"missing required file: {relative}")
        return ""
    return path.read_text(encoding="utf-8")


def require(condition: bool, message: str, errors: list[str]) -> None:
    if not condition:
        errors.append(message)


def main() -> int:
    args = parse_args()
    root = args.root.resolve()
    errors: list[str] = []

    if not (root / "UpgradeValue.sln").is_file():
        print(f"ERROR: not an Upgrade Value repository root: {root}", file=sys.stderr)
        return 2

    files = {name: read_required(root, name, errors) for name in REQUIRED_FILES}
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    entry = files["src/entry.cpp"]
    signature_match = re.search(
        r"constexpr\s+int\s+AddonSignature\s*=\s*(-?\d+)\s*;", entry
    )
    require(signature_match is not None, "cannot parse AddonSignature", errors)
    signature = int(signature_match.group(1)) if signature_match else 0
    require(-(2**31) <= signature < 2**31, "AddonSignature is not signed 32-bit", errors)
    require(signature < 0, "AddonSignature must remain a negative integer", errors)
    signature_hex = f"{signature & 0xFFFFFFFF:08X}"
    raidcore_signature = f"0x{signature_hex}"
    require(
        re.fullmatch(r"0x[0-9A-F]{8}", raidcore_signature) is not None,
        "Raidcore signature must be 0x followed by exactly 8 uppercase hex digits",
        errors,
    )
    require(
        "Definition.Signature = AddonSignature;" in entry,
        "GetAddonDef does not assign AddonSignature",
        errors,
    )

    version_match = re.search(
        r"Definition\.Version\s*=\s*\{\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\}",
        entry,
    )
    require(version_match is not None, "cannot parse Definition.Version", errors)
    version_parts = tuple(int(part) for part in version_match.groups()) if version_match else (0, 0, 0, 0)
    version = ".".join(str(part) for part in version_parts[:3])
    require(version_parts[3] == 0, "Definition.Version revision must remain 0", errors)

    metadata_patterns = {
        "name": r'Definition\.Name\s*=\s*"([^"]+)"',
        "author": r'Definition\.Author\s*=\s*"([^"]+)"',
        "description": r'Definition\.Description\s*=\s*"([^"]+)"',
    }
    metadata: dict[str, str] = {}
    for key, pattern in metadata_patterns.items():
        match = re.search(pattern, entry)
        require(match is not None and bool(match.group(1).strip()), f"missing Definition.{key}", errors)
        metadata[key] = match.group(1).strip() if match else ""

    require(
        "Definition.Provider = EUpdateProvider_GitHub;" in entry,
        "update provider must be EUpdateProvider_GitHub",
        errors,
    )
    repository_url = "https://github.com/jakeuj/GW2-Nexus-Upgrade-Value"
    require(repository_url in entry, "GetAddonDef update link does not target this repository", errors)

    readmes = (files["README.md"], files["README.zh-Hant.md"])
    reviews = (files["NEXUS_REVIEW.md"], files["NEXUS_REVIEW.zh-Hant.md"])
    latest_dll = f"{repository_url}/releases/latest/download/UpgradeValue.dll"
    release_tag_url = f"{repository_url}/releases/tag/v{version}"
    versioned_zip_url = (
        f"{repository_url}/releases/download/v{version}/UpgradeValue-v{version}.zip"
    )
    current_release_links = (
        f"Current stable release: [**v{version}**]({release_tag_url})",
        f"目前正式版：[**v{version}**]({release_tag_url})",
    )
    versioned_zip_links = (
        f"[Download `UpgradeValue-v{version}.zip`]({versioned_zip_url})",
        f"[下載 `UpgradeValue-v{version}.zip`]({versioned_zip_url})",
    )
    for name, text, current_link, zip_link in zip(
        ("README.md", "README.zh-Hant.md"),
        readmes,
        current_release_links,
        versioned_zip_links,
    ):
        require(
            current_link in text,
            f"{name} current release label and tag URL are not both v{version}",
            errors,
        )
        require(
            zip_link in text,
            f"{name} ZIP label and download URL are not both v{version}",
            errors,
        )
        require(latest_dll in text, f"{name} lacks the stable latest DLL URL", errors)
    for name, text in zip(("NEXUS_REVIEW.md", "NEXUS_REVIEW.zh-Hant.md"), reviews):
        documented_versions = set(re.findall(r"`(\d+\.\d+\.\d+\.\d+)`", text))
        require(
            documented_versions == {f"{version}.0"},
            f"{name} contains review version drift: {sorted(documented_versions)}",
            errors,
        )
        require(
            text.count(f"`{version}.0`") >= 2,
            f"{name} does not update both reviewed-version locations",
            errors,
        )
        require(f"`{signature}`" in text, f"{name} does not document signature {signature}", errors)

    site = files["docs/index.html"]
    site_version_patterns = (
        r'class="status-pill"[^>]*>.*?穩定版 v(\d+\.\d+\.\d+)',
        r'<p>穩定版 v(\d+\.\d+\.\d+) 可手動安裝',
        r'id="download-title">Upgrade Value v(\d+\.\d+\.\d+)<',
    )
    for pattern in site_version_patterns:
        match = re.search(pattern, site)
        require(match is not None, f"docs/index.html current-version marker missing: {pattern}", errors)
        if match:
            require(
                match.group(1) == version,
                f"docs/index.html current-version marker is v{match.group(1)}, expected v{version}",
                errors,
            )

    workflow = files[".github/workflows/build-and-release.yml"]
    require("DLL_PATH: bin/Release/UpgradeValue.dll" in workflow, "workflow DLL_PATH drifted", errors)
    require("UpgradeValue.dll" in workflow, "workflow does not publish UpgradeValue.dll", errors)
    require("Definition\\.Version" in workflow, "workflow no longer checks Definition.Version", errors)

    nexus_header = files["vendor/nexus/Nexus.h"]
    api_version_match = re.search(r"#define\s+NEXUS_API_VERSION\s+(\d+)", nexus_header)
    require(api_version_match is not None, "cannot parse NEXUS_API_VERSION", errors)
    api_version = api_version_match.group(1) if api_version_match else "unknown"
    for name, text in zip(("NEXUS_REVIEW.md", "NEXUS_REVIEW.zh-Hant.md"), reviews):
        require(f"`{api_version}`" in text, f"{name} does not mention Nexus API version {api_version}", errors)

    app = files["src/App.cpp"]
    app_header = files["src/App.h"]
    lifecycle_pairs = (
        ("Renderer.Register", "Renderer.Deregister", entry),
        ("InputBinds.RegisterWithString", "InputBinds.Deregister", entry),
        ("UI.RegisterCloseOnEscape", "UI.DeregisterCloseOnEscape", entry),
        ("Fonts.Get", "Fonts.Release", app),
    )
    for acquire, release, text in lifecycle_pairs:
        require(acquire in text and release in text, f"unpaired Nexus lifecycle: {acquire} / {release}", errors)
    require("JoinWorker" in app and "worker_.join()" in app, "owned worker is not explicitly joined", errors)

    http = files["src/HttpClient.cpp"]
    require('L"api.guildwars2.com"' in http, "GW2 API host is not fixed in HttpClient", errors)
    require("WINHTTP_FLAG_SECURE" in http, "WinHTTP secure transport flag is missing", errors)
    require('L"Authorization: Bearer "' in http, "bearer Authorization header is missing", errors)

    settings = files["src/Settings.cpp"]
    settings_header = files["src/Settings.h"]
    require("CryptProtectData" in settings, "DPAPI encryption is missing", errors)
    require("CryptUnprotectData" in settings, "DPAPI decryption is missing", errors)
    require(
        "bool chineseUi = false;" in settings_header,
        "new installations must default chineseUi to false",
        errors,
    )
    require(
        'data.value("chineseUi", false)' in settings,
        "missing chineseUi settings must load as false",
        errors,
    )

    gw2_api = files["src/Gw2Api.cpp"]
    require("ToTraditionalChinese" in gw2_api, "Traditional Chinese conversion is missing", errors)
    require('L"&lang=zh"' in gw2_api, "GW2 API lang=zh behavior is missing", errors)
    require('"FONT_DEFAULT"' in app, "Nexus FONT_DEFAULT use is missing", errors)
    require(
        "FontSupportsTraditionalChinese" in app_header
        and "FindGlyphNoFallback" in app,
        "Traditional Chinese UI is not guarded by actual FONT_DEFAULT glyph coverage",
        errors,
    )
    require(
        '"##UpgradeValueLanguage"' in app
        and 'ImGui::TextUnformatted("Language")' in app
        and '"Traditional Chinese (CJK font required)"' in app
        and "ImGuiSelectableFlags_Disabled" in app
        and '"Choose a CJK-capable font in Nexus Options > Style' in app,
        "fixed ASCII language selector, disabled CJK state, or font guidance is missing",
        errors,
    )
    require(
        "scanGeneration_" in app_header
        and "refreshPending_" in app_header
        and "QueueRefresh()" in app,
        "language/font changes no longer invalidate and requeue background scans",
        errors,
    )
    require(
        "ImGuiTableFlags_Sortable" in app
        and "ImGuiTableFlags_SortTristate" in app
        and "ImGui::TableGetSortSpecs()" in app,
        "tri-state table sorting is missing",
        errors,
    )
    require(
        "LocationColumnId" in app
        and "std::stable_sort" in app
        and app.count("ImGuiTableColumnFlags_NoSort") >= 6,
        "Location-only stable sorting is missing or another column became sortable",
        errors,
    )

    project = files["UpgradeValue.vcxproj"]
    require("<LanguageStandard>stdcpp17</LanguageStandard>" in project, "project is not configured for C++17", errors)
    require("<TargetName>UpgradeValue</TargetName>" in project, "DLL target name drifted", errors)

    print(f"Repository: {root}")
    print(f"Addon: {metadata['name']} by {metadata['author']}")
    print(f"Version: {version_parts[0]}.{version_parts[1]}.{version_parts[2]}.{version_parts[3]}")
    print(
        f"Signature: {signature} -> {raidcore_signature} "
        "(required 0x prefix + 4 bytes / 8 hex digits)"
    )
    print(f"Nexus API: {api_version}")

    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 1

    print("OK: repository invariants are consistent")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
