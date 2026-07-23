# v1.0.5 測試紀錄

測試日期：2026-07-23

## 本機靜態驗證

- Repository invariant checker：Pass（`1.0.5.0`）
- GW2 website audit：Pass（0 errors、0 warnings；first-fold images 203,013／256,000 bytes）
- JavaScript syntax：Pass
- XML validation：Pass
- Python checker AST：Pass
- `git diff --check`：Pass

## Windows 建置

- Pull request：[PR #11](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/pull/11)
- Follow-up issue：[Issue #10](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/issues/10)
- GitHub Actions：[run 29991515059](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/runs/29991515059)
- Workflow：`Build and release`
- 結果：`Release|x64` 成功
- Artifact：`UpgradeValue-ci-27-92290bb`
- DLL 大小：731,648 bytes
- DLL 格式：PE32+ x86-64 Windows DLL（COFF x86-64）
- 匯出：`GetAddonDef`
- SHA-256：`4ff939100a32b7eb860b845528ad83e8acc9b993993e517fcc6c05d87c0766ca`
- Artifact ZIP 只包含一份 `UpgradeValue.dll`，其 SHA-256 與獨立 DLL 相同。

## CrossOver 執行期驗收

- CrossOver：`26.3.0.39832`
- Bottle：`win10_64`
- Nexus：`2026.2.17.1210`
- Addon：`Upgrade Value 1.0.5.0`

| 情境 | 結果 |
| --- | --- |
| 按 Nexus `Load` 並由 log 確認載入 `1.0.5.0` | Pending |
| 未排序依目前判斷值全域降冪 | Pending |
| Location 群組升冪、降冪與第三態恢復全域判斷值排序 | Pending |
| Account Bank、Shared Inventory 與至少兩個角色各自集中 | Pending |
| 每組內目前判斷值降冪、零價置底與同價固定順序 | Pending |
| 切換 `Use net listing value` 後立即以 Net listing 重排且建議一致 | Pending |
| 搜尋、清除搜尋與 Refresh 保留 Location 方向 | Pending |
| DPAPI API Key 保存／重載且設定檔無明文 Key | Pending |
| 英文預設、CJK 字形降級與繁中模式 | Pending |
| Escape、快捷鍵、卸載／重新載入與掃描中卸載 | Pending |

測試設定、加密 API Key 與 Nexus log 不加入 repository，也不上傳。

## 發布截圖

- Main：Pending（真實遊戲畫面，顯示 Location 箭頭及同一角色內高價項目優先）。

## 正式發布驗證

- Release：Pending
- Tag workflow：Pending
- Tag commit：Pending
- Release 資產：Pending
- 正式 DLL SHA-256：Pending
- ZIP SHA-256：Pending
- latest-download SHA-256：Pending
- GitHub Pages：Pending
- Issue #10：保持開啟至 Release 與 latest-download 驗證完成。
