# v1.0.4 測試紀錄

測試日期：2026-07-23

## Windows 建置

- Pull request：[PR #9](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/pull/9)
- GitHub Actions：[run 29986982492](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/runs/29986982492)
- Workflow：`Build and release`
- 結果：`Release|x64` 成功
- Artifact：`UpgradeValue-ci-21-5a0eb03`
- DLL 大小：729,600 bytes
- DLL 格式：PE32+ x86-64 Windows DLL
- 匯出：`GetAddonDef`
- SHA-256：`d81a2c6f9b0c674fbd3c51527d465022466e7662278194961fa860f2186a89f3`

## CrossOver 執行期驗收

- CrossOver：`26.3.0.39832`
- Bottle：`win10_64`
- Nexus：`2026.2.17.1210`
- Addon：`Upgrade Value 1.0.4.0`

| 情境 | 結果 |
| --- | --- |
| 載入外掛並由 Nexus log 確認 `Upgrade Value loaded.` | Pass |
| 全新安裝、內建 Inter、英文預設且無 `?` | Pass |
| 缺少 CJK 字形時停用 Traditional Chinese 並顯示英文提示 | Pass |
| 舊 `chineseUi: true` 設定降級、保存與英文重掃描 | Pass |
| 支援 CJK 的自訂字型、繁中切換與重新啟動保存 | Pass |
| 執行中切回 Default 字型、取消中文掃描並完成英文重掃描 | Pass |
| Location 升冪、降冪、第三次恢復價值順序與相同位置穩定排序 | Pass |
| 搜尋過濾與清除搜尋不重設 Location 排序 | Pass |
| DPAPI API Key 保存／重載、無明文 Key、錯誤訊息不洩漏 Key | Pass |
| Refresh、Escape、快捷鍵、卸載／重新載入與掃描中卸載 | Pass |

CrossOver 驗收由維護者完成並確認可發布。測試設定、加密 API Key 與 Nexus log 未加入 repository，也未上傳。

## 發布截圖

- Settings：英文已選取、Traditional Chinese 因 CJK 字形不足而停用，並顯示 Nexus Options → Style 提示。
- Main：Location 升冪箭頭與依角色／儲存位置集中後的實際資料列。
