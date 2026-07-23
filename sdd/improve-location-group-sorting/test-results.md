# v1.0.5 測試紀錄

測試日期：2026-07-23

## 本機靜態驗證

- Repository invariant checker：Pass（`1.0.5.0`）
- GW2 website audit：Pass（0 errors、0 warnings；first-fold images 223,967／256,000 bytes）
- JavaScript syntax：Pass
- XML validation：Pass
- Python checker AST：Pass
- `git diff --check`：Pass
- 桌面版 1280px：Pass（無水平溢位、版本與新版主畫面可見、console 無 error／warning）
- 行動版 390px：Pass（無水平溢位、導覽選單可展開、console 無 error／warning）

## Windows 建置

- Pull request：[PR #11](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/pull/11)
- Follow-up issue：[Issue #10](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/issues/10)
- GitHub Actions：[run 29992747527](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/runs/29992747527)
- Workflow：`Build and release`
- 結果：`Release|x64` 成功
- Artifact：`UpgradeValue-ci-28-bb4be02`
- DLL 大小：731,648 bytes
- DLL 格式：PE32+ x86-64 Windows DLL（COFF x86-64）
- 匯出：`GetAddonDef`
- SHA-256：`a33031d7ec400f6e9bddc3a97907b0408252b508c5868c1cb6f0ccd9b8cd880c`
- Artifact ZIP 只包含一份 `UpgradeValue.dll`，其 SHA-256 與獨立 DLL 相同。

## CrossOver 執行期驗收

- CrossOver：`26.3.0.39832`
- Bottle：`win10_64`
- Nexus：`2026.2.17.1210`
- Addon：`Upgrade Value 1.0.5.0`

| 情境 | 結果 |
| --- | --- |
| Nexus 顯示 `Upgrade Value 1.0.5.0`／`Disable`，log 確認載入 | Pass |
| 未排序依目前 Instant sell 判斷值全域降冪 | Pass |
| Location 群組升冪、降冪與第三態恢復全域判斷值排序 | Pass |
| Account Bank 與至少兩個角色各自集中 | Pass |
| Shared Inventory 群組鍵 | 本次帳號沒有可見代表列；由 invariant checker 驗證映射 |
| 每組內目前判斷值降冪、無價格列置後與同價固定順序 | Pass |
| 切換 `Use net listing value` 後立即以 Net listing 重排且建議一致 | Pass |
| 搜尋、清除搜尋與 Refresh 保留 Location 方向 | 接受既有行為；Refresh／重啟後方向與分組維持 |
| DPAPI API Key 保存／重載且設定檔無明文 Key | Pass |
| 英文預設與缺字防護 | Pass；CJK 切換路徑未變更，沿用 v1.0.4 驗收 |
| Escape、目前快捷鍵、完整程序卸載／重新載入 | Pass |

CrossOver 最初載入並執行遊戲內排序驗收的是 [run 29991515059](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/runs/29991515059) 的 DLL，SHA-256 為 `4ff939100a32b7eb860b845528ad83e8acc9b993993e517fcc6c05d87c0766ca`。最終 CI 僅新增文件、真實截圖與測試紀錄，程式碼未再變更；最終 DLL 因建置時間戳不同而具有上列新雜湊。測試設定、加密 API Key 與 Nexus log 不加入 repository，也不上傳。維護者確認目前測試範圍足以發布。

## 發布截圖

- Main：Pass（真實遊戲畫面，顯示 Location 升冪箭頭、角色／儲存區群組及同一角色內高價項目優先）。
- 已同步 `screenshots/upgrade-value-main.png`、網站 PNG fallback，以及 640／960px AVIF、WebP 衍生圖。

## 正式發布驗證

- Release：[v1.0.5](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/releases/tag/v1.0.5)
- Tag workflow：[run 29993546975](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/runs/29993546975) 成功
- Annotated tag commit：`83b694f2c3ea7b193d267c45a423adace1ef4ab2`
- Release 資產：僅 `UpgradeValue.dll` 與 `UpgradeValue-v1.0.5.zip`
- 正式 DLL：731,648 bytes、PE32+ x86-64 Windows DLL（COFF x86-64），匯出 `GetAddonDef`
- 正式 DLL SHA-256：`7bc7d062ea6c9ff1a5bfed87781b18b7ce388a02a88559b2df1f95db9864d641`
- ZIP SHA-256：`de120d5ab84800e40fc6576833a2c3315779b03ca2701cb826a776afb6ee38e6`
- ZIP：只包含一份 `UpgradeValue.dll`，其 SHA-256 與正式 DLL 相同
- latest-download：SHA-256 與正式 DLL 相同
- GitHub Pages：[run 29993327798](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/actions/runs/29993327798) 成功；正式站顯示 `v1.0.5`、新版排序說明與真實遊戲截圖，1280px 無水平溢位
- Follow-up issue：[Issue #10](https://github.com/jakeuj/GW2-Nexus-Upgrade-Value/issues/10) 已留下發布與驗證紀錄並關閉。
