# Location 分組排序實作工作

- [x] 1. 建立 Location 群組鍵，將搜尋後結果依目前 Instant sell／Net listing 判斷值排序。
- [x] 2. 完成 Location 群組升冪、降冪、全域價值三態，並移除舊有最大值掃描排序。
- [x] 3. 更新 results-table contract 與 repository invariant checker。
- [x] 4. 將版本升至 `1.0.5.0`，同步雙語 README、Nexus review 與網站內容。
- [x] 5. 執行本機不變量、網站、JavaScript／XML 與差異檢查。
- [x] 6. 建立 follow-up issue 與 Draft PR，以 Windows CI 建置並驗證 DLL。
- [x] 7. 在 CrossOver 載入 CI DLL，完成排序、估價切換、語言、API Key 與生命週期驗收。
- [x] 8. 更新真實遊戲截圖、衍生圖片與測試紀錄，完成合併前驗證。
- [ ] 9. 發布 `v1.0.5`，確認 Release、latest download、GitHub Pages 並關閉 follow-up issue。

## 驗收條件

- 情境：Location 升冪或降冪時，相同角色、Account Bank、Shared Inventory 的資料集中顯示，且組內價值由高到低。
- 情境：Location 第三次點擊回到目前估價模式的全域價值排序。
- 情境：切換 `Use net listing value` 後，畫面立即改用 Net listing 重排，Recommendation 與排序判斷一致。
- 情境：零價或無 Trading Post 價格位於各組後方；同價時以完整 Location、Upgrade、Equipment、ID 得到固定順序。
- 情境：搜尋、清除搜尋與 Refresh 後保留 Location 排序方向和組內價值順序。
- 情境：`v1.0.5` 的 DLL、雙語文件、Nexus review、網站、Release 資產與下載雜湊一致。
