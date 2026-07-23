# Issue #7 語言相容性與 Location 排序

類型：修 bug

## 為什麼做

Upgrade Value 目前把繁體中文設為新安裝預設值，但 Raidcore Nexus 內建的 Inter 字型不含中文字形，導致英文原版 Nexus 上的中文介面與物品名稱顯示為 `?`。此外，結果表格的 Location 欄位不能排序，使用者難以把同一角色或儲存位置的升級集中查看。

## 要改什麼

- 新安裝預設使用英文，既有設定在字型支援時仍可保留繁中。
- 檢查 Nexus `FONT_DEFAULT` 是否含必要繁中文字形；缺字時自動退回英文並顯示可讀警告。
- 提供固定 ASCII 的語言選單；不支援 CJK 時停用繁中選項。
- 語言切換或字型降級時安全取消舊掃描，再以正確語言重新掃描。
- 讓 Location 欄位支援升冪、降冪與恢復預設價值順序。
- 發布版本升至 `1.0.4.0`，同步雙語文件、Nexus review、網站內容與截圖。
- 擴充 repository invariant checker，防止本次修復回歸。

## 影響範圍

- `src/App.cpp`、`src/App.h`：字形檢查、語言選單、掃描切換與 Location 排序。
- `src/Settings.cpp`、`src/Settings.h`：英文預設與既有設定相容。
- `src/entry.cpp`：版本升至 `1.0.4.0`。
- `README.md`、`README.zh-Hant.md`、`NEXUS_REVIEW*.md`、`docs/`：版本、功能、限制與畫面同步。
- `.agents/skills/maintain-upgrade-value/scripts/check_repo_invariants.py`：新增修復不變量。
- `screenshots/` 與 `docs/assets/`：更新實際遊戲畫面及網站圖片衍生格式。
