# Location 分組與價值優先排序

類型：修 bug

Follow-up issue：[GitHub Issue #10](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/issues/10)

## 為什麼做

`v1.0.4` 已讓 Location 欄位可排序，但它比較完整 Location 字串，會優先排列角色內的裝備欄位，而非協助玩家逐一找到各角色或儲存區中的高價升級。Issue #7 的實際使用情境是先集中角色／儲存區，再在每組內優先顯示目前值得使用黑獅拆解工具的項目。

## 要改什麼

- 從 Location 建立角色、Account Bank、Shared Inventory 等群組鍵，未知格式退回完整 Location。
- 搜尋後依目前估價模式排序：Instant sell 或 Net listing，皆由高到低。
- Location 升／降冪只改變群組順序，組內仍維持目前判斷值由高到低。
- Location 第三態恢復全域判斷值排序；估價切換時立即重排。
- 移除掃描階段使用兩種價格最大值的舊排序，將畫面排序集中在 results table。
- 版本升至 `1.0.5.0`，同步 repository contract、不變量檢查、雙語文件、Nexus review、網站與真實遊戲截圖。
- 另開 follow-up issue 引用已完成的 Issue #7，保留 `v1.0.4` 的修復紀錄。

## 影響範圍

- `src/App.cpp`：搜尋後的判斷值排序、Location 群組鍵與三態排序。
- `src/Gw2Api.cpp`：移除舊有 `max(Instant sell, Net listing)` 掃描排序。
- `src/entry.cpp`：版本升至 `1.0.5.0`。
- `.agents/skills/maintain-upgrade-value/references/repository-contract.md`：更新 results-table contract。
- `.agents/skills/maintain-upgrade-value/scripts/check_repo_invariants.py`：新增群組鍵、判斷值及 Location-only 三態檢查。
- `README*.md`、`NEXUS_REVIEW*.md`、`docs/`：同步版本與新排序行為。
- `screenshots/`、`docs/assets/`：更新真實英文主畫面與網站衍生圖片。
