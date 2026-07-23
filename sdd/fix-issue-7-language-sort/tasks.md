# Issue #7 實作工作

- [x] 1. 將新安裝語言預設改為英文，加入 `FONT_DEFAULT` 繁中文字形檢查與既有設定降級遷移。
- [x] 2. 建立固定 ASCII 的語言選單，並完成語言變更時的掃描取消、排程與警告呈現。
- [x] 3. 為 Location 欄位加入升冪、降冪、未排序三態與穩定排序。
- [x] 4. 將版本升至 `1.0.4.0`，同步雙語 README、Nexus review 與網站行為說明。
- [x] 5. 擴充 repository invariant checker，涵蓋英文預設、字形防護、語言選單與 Location 排序。
- [x] 6. 執行本機不變量、差異、網站靜態檢查與可用的編譯前驗證。
- [x] 7. 在 Windows `Release|x64` 建置並完成 Nexus／GW2 實機 smoke test。
- [x] 8. 擷取新設定與 Location 排序畫面，更新 README 與網站衍生圖片。
- [ ] 9. 發布 `v1.0.4`，確認 Release 資產與 latest DLL，並回覆 issue #7。

## 驗收條件

- 情境：在英文原版 Nexus 全新安裝，介面與物品名稱直接使用英文，不顯示 `?`。
- 情境：舊設定為繁中但目前 Nexus 字型缺字，外掛自動保存英文模式、停止舊掃描並顯示英文警告。
- 情境：目前 Nexus 字型不支援 CJK，設定頁仍可讀，且 Traditional Chinese 選項不可選。
- 情境：目前 Nexus 字型支援 CJK，使用者可在 English 與 Traditional Chinese 間切換，保存後以相同語言重新掃描。
- 情境：點擊 Location 欄位可在升冪、降冪與原始價值順序間切換；相同位置保留原本相對順序。
- 情境：搜尋過濾結果時，Location 排序方向仍正確。
- 情境：`v1.0.4` 的程式、雙語文件、Nexus review、網站與 Release 資產版本一致。
