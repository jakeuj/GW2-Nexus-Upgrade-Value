# Nexus 提交審核說明

[English](NEXUS_REVIEW.md) · **繁體中文**

本文件提供 Raidcore Nexus 審核人員一份可由原始碼直接驗證的 Upgrade Value 行為與審核範圍摘要。內容對應外掛版本 `1.0.5.0`，並使用專案內附的 Nexus API 版本 `6` 建置。

**上架狀態：** Upgrade Value `v1.0.3` 已完成 Raidcore 初審，並公開上架 Nexus Addon Library；公開頁面為 [Listing ID 128](https://raidcore.gg/gw2/addons/upgrade-value)。

## 審核摘要

| 審核項目 | 實作狀況 |
|---|---|
| 逆向工程程式碼 | **無。** 本外掛不含第一方逆向工程程式碼，因此「逆向工程程式碼必須閉源」的要求不適用。 |
| 第一方原始碼公開範圍 | 建置發行版外掛所需的所有專案自有程式碼都在此公開儲存庫內。沒有私有服務、私有函式庫、伴隨執行檔、混淆 Payload 或未公開的第一方模組。 |
| 遊戲程序存取 | 僅使用公開的 Nexus `AddonAPI`。沒有直接讀寫遊戲記憶體、特徵碼掃描、Detour、函式 Hook、封包攔截或 Guild Wars 2 原生 UI Hook。 |
| 遊戲操作自動化 | **無。** 外掛不會模擬輸入、施放技能、移動角色、操作物品欄 UI、分解物品、交易或進行無人操作。 |
| 顯示資訊 | 顯示官方帳號 API 快照、公開物品資料、交易所價格，以及可調整門檻的分解建議。所有後續遊戲操作都必須由玩家手動完成。 |
| 外掛自身網路連線 | 只會向 `api.guildwars2.com` 傳送 HTTPS `GET` 請求。 |
| 遙測與廣告 | 無。沒有分析、崩潰回報、追蹤、廣告或開發者自行營運的後端。 |
| 本機機密資料 | Guild Wars 2 API Key 會先以 Windows DPAPI 加密，再寫入 `settings.json`。 |
| 更新 | 宣告 Nexus 的 GitHub 更新提供者；版本檢查與安裝由 Nexus 處理，外掛本身沒有下載器。 |
| AI 協助 | Codex 曾協助專案文件與開發審查。此項協助會依 Raidcore 的 **AI Notice** 類別揭露；所有審查、測試、維護與合規責任仍由開發者承擔。 |

## 外掛實際行為

1. 外掛預設使用英文，並在沿用已儲存的繁中偏好前，檢查 Nexus `FONT_DEFAULT` 是否含有代表性繁中文字形；若缺字，會保存英文模式並顯示可讀警告。
2. 若已有儲存的 API Key，外掛載入後會在背景開始掃描。使用者也可以按 **重新掃描** 或 **儲存並掃描** 主動執行。
3. 外掛會驗證 Key，並從 Guild Wars 2 官方 v2 API 讀取帳號、銀行、共享物品欄、角色物品欄及目前裝備的快照。
4. 從 API 回傳的 `upgrades[]` 與選用的 `infusions[]` 欄位讀取內嵌升級 ID。
5. 向官方 API 查詢公開物品資料與 Trading Post 價格。
6. 在 ImGui 表格中顯示結果，並依使用者選擇的估價方式與門檻計算建議。三態位置排序會依角色或儲存區集中資料列，各組內仍以目前判斷值由高到低排列；未排序狀態則使用全域判斷值順序。

外掛不會呼叫任何可變更帳號狀態的 API，也不會代替玩家執行遊戲內操作。

## ToS 審核重點

最需要政策審核的功能是「建議」欄。外掛會比較內嵌升級的官方 Trading Post 價格與使用者設定的門檻，顯示 **使用黑獅** 或 **一般分解**。

此計算可能讓經濟決策更方便，但功能僅提供資訊：

- 資料來自官方 API 快照，不是遊戲記憶體。
- 無法選取物品或啟用分解工具。
- 無法注入或模擬滑鼠、鍵盤或控制器輸入。
- 無法在玩家離開時自行遊玩，也無法自行取得獎勵。
- 不會顯示戰鬥、敵人、目標、座標或遭遇機制資訊。

Raidcore 已在初審後公開 Listing ID 128；此外掛能否持續上架及未來分類仍由 Raidcore 審核。ArenaNet 亦明確表示不會審查、核准或背書個別第三方程式，並保留對其使用方式的裁量權。

## AI Notice

Codex 曾用於協助專案文件與開發審查。AI 輔助輸出屬於需要接受審查的專案內容，不是正確性或合規性的判定依據。

- 所有公開內容與發行行為均由開發者審查並承擔責任。
- AI 協助不能取代原始碼審查、建置、遊戲內測試、安全性檢查或 Raidcore 的政策與 ToS 判定。
- 主動加入此揭露，供 Raidcore 判斷是否套用 **AI Notice** 分類。

## 逆向工程與原始碼政策

此儲存庫及外掛所連結的內容都沒有逆向工程實作。

專案明確不包含：

- 由反組譯取得的遊戲函式、位移、特徵碼或結構；
- 記憶體掃描器、注入器、Detour 或 Hook 函式庫；
- 封包擷取或通訊協定攔截；
- 內附的閉源 DLL、執行檔、服務或遠端私有 API；
- 會下載私有第一方元件的隱藏建置步驟。

專案內附的第三方原始碼只有公開的 Raidcore Nexus header、Raidcore Dear ImGui fork 與 nlohmann/json，且都附有各自授權。產生的建置輸出不會加入 Git，發行檔由公開的 GitHub Actions 工作流程建置。

## Nexus 整合範圍

| 公開 Nexus API | 用途 |
|---|---|
| `Renderer.Register` / `Deregister` | 主視窗與 Nexus 設定頁繪製 |
| `InputBinds.RegisterWithString` / `Deregister` | 一個視窗切換快捷鍵，預設 `Alt + Shift + U` |
| `UI.RegisterCloseOnEscape` / `DeregisterCloseOnEscape` | 按 Escape 關閉外掛視窗 |
| `Paths.GetAddonDirectory` | 取得可寫入的設定目錄 |
| `Fonts.Get` / `Release` | 取得 Nexus `FONT_DEFAULT`，啟用繁中前驗證代表性 CJK 字形，缺字時退回英文 |
| `Log` | 寫入載入與卸載訊息 |
| GitHub 更新提供者中繼資料 | 讓 Nexus 從本公開儲存庫管理更新 |

外掛不使用 Nexus Events、WndProc callbacks、DataLink、MumbleLink、Quick Access、Textures、輸入攔截或自行管理的更新請求。

註冊與解除註冊配對可在 [`src/entry.cpp`](src/entry.cpp) 與 [`src/App.cpp`](src/App.cpp) 查驗。

## 網路連線範圍

外掛自身的 HTTP 行為位於 [`src/HttpClient.cpp`](src/HttpClient.cpp)。主機固定寫死為 `api.guildwars2.com`，連接埠為 HTTPS `443`，並透過 Windows WinHTTP 與 `WINHTTP_FLAG_SECURE` 傳輸。

| Endpoint | 驗證方式 | 用途 |
|---|---|---|
| `/v2/tokeninfo` | Bearer API Key | 驗證權限 |
| `/v2/account` | Bearer API Key | 顯示帳號名稱 |
| `/v2/account/bank` | Bearer API Key | 讀取帳號銀行物品實例 |
| `/v2/account/inventory` | Bearer API Key | 讀取共享物品欄物品實例 |
| `/v2/characters?ids=all` | Bearer API Key | 讀取角色背包與目前裝備 |
| `/v2/items?ids=...` | 無 | 讀取公開物品資料 |
| `/v2/commerce/prices?ids=...` | 無 | 讀取公開 Trading Post 價格 |

其他 URL 行為：

- API Key 只會放在 `Authorization: Bearer` header，不會放進 URL。
- 使用者按下 API Key 管理按鈕後，才會以預設瀏覽器開啟 `https://account.arena.net/applications`。
- 使用者按下作者按鈕後，才會以預設瀏覽器開啟 `https://github.com/jakeuj`。
- `GetAddonDef` 將此 GitHub 儲存庫提供給 Nexus 作為更新來源；外掛本身沒有 GitHub HTTP client 或檔案替換程式碼。

專案自有的執行期程式碼沒有其他網路目的地。

## 資料處理與隱私

- 必要 API 權限僅限 `account`、`inventories` 與 `characters`。
- API Key 透過 Windows `CryptProtectData` 保護，並以 Base64 編碼的 DPAPI 密文儲存。
- 設定寫入 `Paths.GetAddonDirectory("UpgradeValue")` 回傳的目錄。
- 掃描結果、帳號名稱、角色名稱、物品位置與價格只保留在記憶體，不會快取到磁碟。
- 不會將帳號或使用資料傳送給開發者或任何第三方分析服務。
- 外掛不會將 API Key 寫入 Nexus log。

本機儲存實作位於 [`src/Settings.cpp`](src/Settings.cpp)。

## 執行緒與卸載行為

- 網路及 JSON 工作在一個由外掛管理的 `std::thread` 執行，不會在 render callback 內執行。
- 共用結果以 mutex 保護。
- 語言變更會使目前掃描世代失效、要求取消，並排程使用新語言重新掃描，避免舊結果覆蓋目前資料列。
- 卸載時會設定取消旗標，並在銷毀外掛狀態前 join worker。
- WinHTTP 的解析、連線、傳送與接收都有有限時間的 timeout。卸載可能會等待目前這次有時限的請求結束，但不會 detach worker。
- 所有 Nexus 註冊項目與取得的字型都會在卸載時釋放。

## 建置與相依性審核

- 目標：Windows x64 DLL。
- Export：使用 C linkage 的 `GetAddonDef`。
- Addon signature：`-26071501`。
- Addon version：`1.0.5.0`。
- Nexus API version：`6`。
- Update provider：Nexus GitHub provider。
- 建置指令：

  ```powershell
  msbuild UpgradeValue.sln /m /p:Configuration=Release /p:Platform=x64
  ```

- CI 定義：[`.github/workflows/build-and-release.yml`](.github/workflows/build-and-release.yml)
- 專案定義：[`UpgradeValue.vcxproj`](UpgradeValue.vcxproj)
- 第三方授權：[`vendor/imgui/LICENSE.txt`](vendor/imgui/LICENSE.txt)、[`vendor/nexus/LICENSE`](vendor/nexus/LICENSE) 與 [`vendor/nlohmann_json/LICENSE.MIT`](vendor/nlohmann_json/LICENSE.MIT)

儲存庫沒有追蹤任何編譯後的 DLL、執行檔、靜態函式庫、物件檔或 ZIP。

## 審核程式碼索引

| 檔案 | 主要審核用途 |
|---|---|
| [`src/entry.cpp`](src/entry.cpp) | Addon definition、Nexus API 註冊、生命週期、更新中繼資料 |
| [`src/App.cpp`](src/App.cpp) | UI、建議計算、背景 worker、字型與設定生命週期 |
| [`src/Gw2Api.cpp`](src/Gw2Api.cpp) | 完整 API endpoints、帳號資料解析、價格計算 |
| [`src/HttpClient.cpp`](src/HttpClient.cpp) | 固定網路主機、HTTPS 傳輸、Bearer header |
| [`src/Settings.cpp`](src/Settings.cpp) | DPAPI 加密與本機設定儲存 |
| [`UpgradeValue.vcxproj`](UpgradeValue.vcxproj) | 建置目標、原始碼清單、編譯與連結設定 |

## 政策參考

- [ArenaNet 第三方程式政策](https://help.guildwars2.com/hc/en-us/articles/360013625034-Policy-Third-Party-Programs)
- [Guild Wars 2 API 概覽與條款](https://wiki.guildwars2.com/wiki/API:Main)
- [Raidcore `AddonDefinition` 文件](https://docs.raidcore.gg/structAddonDefinition__t.html)
- [Raidcore Nexus API header](https://github.com/RaidcoreGG/Nexus-API/blob/main/Nexus.h)

若未來版本加入遊戲記憶體存取、原生遊戲 Hook、輸入自動化、新網路目的地、新 API 權限、帳號資料持久快取或私有／閉源元件，應更新本文件並重新接受審核。
