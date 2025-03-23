# 1132 Parallel hw1
## Part 1: Matrix

#### 1. **循環依賴 (Cyclic Dependency)**
`Column_Major_Matrix` 和 `Row_Major_Matrix` 這兩個類別相互依賴，因為 `Column_Major_Matrix` 需要存取 `Row_Major_Matrix` 來實現矩陣乘法與型別轉換。然而，在程式中使用尚未定義的類別會導致編譯錯誤。

為了解決這個問題，可以在程式開頭加入 **forward declaration**：
```cpp=10
template <typename T>
class Row_Major_Matrix;

template <typename T>
class Column_Major_Matrix;
```
這樣可以讓編譯器知道這些類別的存在，而無需包含完整的類別定義。

另一種解法是使用**標頭檔 (header file)** 來定義這些類別，並在其他檔案中包含該標頭。然而，這種方法可能會增加編譯時間，特別是在大型專案中。相較之下，**forward declaration** 只告知編譯器類別的存在，避免載入不必要的額外程式碼，從而有效縮短編譯時間。

#### 2. **矩陣乘法 (Matrix Multiplication)**
在未使用 **multi-threading** 的情況下，矩陣乘法的效能會受到三層巢狀 `for` 迴圈的影響。因此，我們採用了**直覺性的並行化方式**，將矩陣拆分，每個 thread 負責 10 個 columns 的運算，以提升效能。

然而，當矩陣規模較小時，開啟過多的 threads 可能會帶來額外的開銷，反而降低效率。因此，可以利用 `std::thread::hardware_concurrency()` 來動態獲取理想的 thread 數量。這部分會在part 2 thread pool中使用到。

#### 3. **隱式轉換 (Implicit Conversion)**
在類別內部，`this` 指標可用於建立當前類別的隱式物件。例如，在 `Column_Major_Matrix` 中，`this->getRow(i)` 表示存取當前 `Column_Major_Matrix` 物件的 `getRow()` 方法。

此外，這也展現了**資料交換與協作機制**：
1. `Column_Major_Matrix` 提供行向量資料 (`getRow()`)
2. `Row_Major_Matrix` 接收並設定這些資料 (`setRow()`)

兩個類別的方法**介面必須相容**，確保資料交換能順利進行。

* 減少了顯示轉換的冗餘程式碼，讓演算法實現更專注於核心邏輯而非格式轉換。
* 透過setRow這樣的方法，可以對整行資料進行優化處理，比如使用 **SIMD**。

## Part 2: Threads Pool

#### 1. Synchronization
在`increment_printCount()`中，確保使用`notify_all()`所有等待的threads都能被通知條件變更。

#### 2. Mutex使用
將mutex相關的操作放在大括號`{}`中是一個重要的程式設計模式，稱為**限定範圍鎖(Scope-based lock)**。

將std::unique_lock 放在一個 {} 作用域中時：
1. 在進入作用域時：mutex被鎖定，其他試圖獲取這個 mutex 的線程將被阻塞
2. 在離開作用域時：mutex自動被釋放（即使發生異常也會釋放）

* 防止死鎖，確保mutex總是被釋放。
* 不須手動釋放mutex，縮短鎖的持有時間，讓其他線程更快獲得資源的訪問權。
