# GDTTelemetryQt — Bug Fix Log

Ghi lại các bug phát hiện và cách fix trong quá trình phát triển app server Qt thay thế app server C#.

---

## 1. Socket Memory Leak — Worker không xóa socket

**Triệu chứng:** Sau khi client ngắt kết nối, bộ nhớ không được giải phóng.

**Nguyên nhân:**  
`ClientWorker` giữ raw pointer `m_socket`. Khi `stop()` được gọi, worker bị xóa nhưng `m_socket` không có owner → socket tồn tại mãi.

**Fix:**  
Thêm `connect(thread, &QThread::finished, socket, &QObject::deleteLater)` trong `onNewConnection()`. Khi thread kết thúc, socket/worker/thread đều tự xóa qua `deleteLater`.

```cpp
connect(thread, &QThread::finished, socket, &QObject::deleteLater);
connect(thread, &QThread::finished, worker, &QObject::deleteLater);
connect(thread, &QThread::finished, thread, &QObject::deleteLater);
```

---

## 2. Main thread bị block khi client disconnect

**Triệu chứng:** UI freeze khi một GND client ngắt kết nối.

**Nguyên nhân:**  
Lambda `clientDisconnected` gọi `t->quit(); t->wait(); delete t;` — `t->wait()` block main thread event loop vô thời hạn nếu client phản hồi chậm.

**Fix:**  
Bỏ `t->wait()` trong lambda, chỉ gọi `thread->quit()`. Cleanup được xử lý tự động qua chuỗi `finished → deleteLater`.

```cpp
connect(worker, &ClientWorker::clientDisconnected, this,
    [this, addr, thread](const QString&) {
        emit clientDisconnected(addr);
        m_workers.remove(addr);
        m_threads.remove(addr);
        thread->quit(); // finished → deleteLater chain handles cleanup
    }, Qt::QueuedConnection);
```

---

## 3. Server stop dùng `abort()` khiến GND không reconnect được

**Triệu chứng:** Sau khi bấm Stop server rồi Start lại, GND không kết nối lại được.

**Nguyên nhân:**  
`abort()` gửi TCP RST → GND nhận error, không retry. Đặt lại thành `disconnectFromHost()` thì server `stop()` bị treo do `t->wait()` không có timeout.

**Fix:**  
- Dùng `disconnectFromHost()` (gửi FIN thay vì RST) để GND nhận clean disconnect.
- Thêm timeout cho `t->wait(2000)`, nếu quá 2s thì `t->terminate()`.

```cpp
// ClientWorker::stop()
QMetaObject::invokeMethod(m_socket, "disconnectFromHost", Qt::QueuedConnection);

// TelemetryServer::stop()
for (auto* t : threads) {
    t->quit();
    if (!t->wait(2000)) t->terminate();
}
```

---

## 4. `m_clientCount` không reset khi Stop server

**Triệu chứng:** Sau khi Stop, UI vẫn hiển thị "Clients: N" thay vì "Clients: 0".

**Nguyên nhân:**  
`stop()` xóa worker trước khi signal `clientDisconnected` kịp fire theo `QueuedConnection` → counter không về 0.

**Fix:**  
Reset thủ công trong `onStopServer()`:

```cpp
void MainWindow::onStopServer() {
    m_server->stop();
    m_mcast->stop();
    m_clientCount = 0;
    m_connLabel->setText("Clients: 0");
    updateStatusBar("Server da dung");
}
```

---

## 5. Sub-packet misalignment khi parse thất bại (bug gốc)

**Triệu chứng:** Sau khi 1 sub-packet lỗi tailer/CRC8, các sub-packet tiếp theo trong cùng frame bị parse sai dây chuyền.

**Nguyên nhân:**  
Code gốc dùng `++i` (advance 1 byte) khi tailer hoặc CRC8 fail. Nếu byte `0xAF` xuất hiện bên trong data của packet lỗi, parser nhảy vào giữa packet → cascade misalignment.

C# advance `len` bytes khi tailer OK + CRC8 fail, và 1 byte khi tailer fail.

**Fix:**  
Đổi sang `i += len` cho cả hai trường hợp (an toàn hơn, tránh false 0xAF trong data bytes):

```cpp
if ((uint8_t)pkt[len - 2] != TAILER) { i += len; continue; }
if (!m_parser.checkCrc8(pkt))         { i += len; continue; }
```

---

## 6. Sub-packet bị split giữa 2 TCP frame — NGUYÊN NHÂN CHÍNH của error rate cao ★

**Triệu chứng:** Qt server có tỷ lệ lỗi 1-3% (5A42 ~3%, Tele ~1.4%, 5U44 ~1%, 5I41 ~1.2%) trong khi C# server với cùng GND cho 0%.

**Điều tra:**  
Từ raw telemetry file thực tế (`Raw_Telemetry_DESKTOP-5PP76I1_date_11_04_2026_15_48_07.text`):

```
pos=  0: 5A42(52B) idx=8166  ← Frame 1 bắt đầu
pos= 52: 5U44(20B) idx=8167
pos= 72: Tele(30B) idx=8167
pos=102: 5I41(20B) idx=8514  ← 5I41 có counter riêng!
pos=122: 5A42(52B) idx=8167  ← Frame 2 bắt đầu
pos=174: 5A42(52B) idx=8168  ← HAI 5A42 LIÊN TIẾP!
pos=226: 5U44(20B) idx=8168
pos=246: Tele(30B) idx=8168
pos=276: 5I41(20B) idx=8515
```

TLDK35 đôi khi gửi 2 packet 5A42 liên tiếp. Khi GND đóng gói 122 bytes vào 1 TCP frame:
- Frame 2 chứa `[5A42(52)] + [5A42(52)] = 104 bytes` → chỉ còn 18 bytes cuối cho 5U44 bị cắt đôi
- Sub-packet 5U44 bị split giữa TCP frame 2 và frame 3

**Tại sao C# OK:**  
C# dùng `_teleData` (CircularBuffer 1MB) tích lũy bytes từ tất cả TCP frame. Sub-packet bị split sẽ được ghép lại khi bytes còn lại đến ở frame tiếp theo.

**Tại sao Qt lỗi:**  
Qt cũ parse từng `payload` của 1 TCP frame độc lập (`parseSubPackets(payload)`). Sub-packet bị split → bị mất.

**Tại sao 5A42 lỗi nhiều nhất:**  
5A42 là packet lớn nhất (52 bytes), xác suất bị cắt đôi qua frame boundary cao hơn.

**Fix:**  
Thêm `m_subBuffer` (QByteArray) tích lũy bytes xuyên TCP frame, giống hệt `_teleData` trong C#:

```cpp
// TelemetryServer.h
QByteArray m_subBuffer; // accumulates sub-packet bytes across TCP frames

// TelemetryServer.cpp — processBuffer()
if (type == TCP_TYPE_TELE) {
    m_subBuffer.append(payload);   // tích lũy
    parseSubPackets();
}

// parseSubPackets() đọc từ m_subBuffer, không nhận payload parameter
void ClientWorker::parseSubPackets() {
    int i = 0;
    while (i < m_subBuffer.size()) {
        // ... scan for 0xAF, determine len, validate ...
        if (i + len > m_subBuffer.size()) break; // incomplete — đợi frame tiếp
        // ... emit signal ...
        i += len;
    }
    m_subBuffer.remove(0, i); // giữ phần chưa parse xong
}

// onDisconnected() — xóa buffer khi client disconnect
void ClientWorker::onDisconnected() {
    m_subBuffer.clear();
    emit clientDisconnected(m_address);
}
```

---

## 7. CRC16-8005 check trên TCP outer frame (đã bỏ)

**Triệu chứng:** Một số frame bị reject dù C# chấp nhận (đóng góp vào error rate).

**Điều tra:**  
So sánh kỹ C# và Qt:
- Cùng table `CRC16_8005_TABLE` = `CRC16TABLE1`
- Cùng init = `0x0000`
- Cùng algorithm: `(crc << 8) ^ table[(crc >> 8) ^ byte]`
- Cùng byte range: `frame.size() - 2`

**Kết luận:** Implementations GIỐNG NHAU 100%. CRC check không phải nguyên nhân chính. Tuy nhiên vẫn bỏ check để khớp hoàn toàn với C# và phòng trường hợp TLDK35 firmware có CRC sai:

```cpp
// processBuffer() — đã bỏ đoạn này:
// if (!checkCrc16_8005(m_buffer.left(frameLen)))
//     { m_buffer.remove(0, 1); continue; }
```

---

## 8. Phát hiện: Các packet type có 20-bit index RIÊNG BI LẬP

Từ raw file, các index KHÔNG giống nhau giữa các loại:
```
5A42: idx=8166, 8167, 8168, ...  (counter A)
5U44: idx=8167, 8168, 8169, ...  (counter B, bắt đầu +1)
Tele: idx=8167, 8168, 8169, ...  (counter C, bắt đầu +1)
5I41: idx=8514, 8515, 8516, ...  (counter D, hoàn toàn khác!)
```

→ PktStats trong Qt đúng khi track từng loại độc lập.  
→ 5I41 có counter riêng (tăng theo groupId 0-4, không theo frame rate).

---

## Simulator tạo ra để test

File: `D:\Code\SimulateSignal\tcp_simulator.py`

```
# Kết nối trực tiếp vào Qt server, gửi TCP frame đúng format
python tcp_simulator.py                          # synthetic data, 125Hz, localhost:16022
python tcp_simulator.py --mode file --file Raw_Telemetry_...text  # dùng data thực
python tcp_simulator.py --ip 192.168.168.30      # kết nối server thực
python tcp_simulator.py --bad-crc 3              # test với 3% frame CRC sai
python tcp_simulator.py --loss 2                 # giả lập 2% mất gói
```

Frame format: `[DE][AD][01][lenH][lenL][payload:122B][EE][ED][CRC16_H][CRC16_L]`  
Sub-packet: `[AF][type<<5|idx_hi][idx_mid][idx_lo][data][FA][CRC8]`

---

## Files đã thay đổi trong session này

| File | Thay đổi |
|------|----------|
| `network/TelemetryServer.h` | Thêm `m_subBuffer`; đổi signature `parseSubPackets()` |
| `network/TelemetryServer.cpp` | Fix socket leak, block main thread, abort→disconnect, sub-packet split, misalignment |
| `ui/MainWindow.h` | Thêm slot `onResetServer()`, `m_connLabel` |
| `ui/MainWindow.cpp` | Client count realtime, Stop/Reset server, info bar, toolbar |
| `storage/CsvLogger.h/.cpp` | Periodic flush mỗi 5s (dùng QTimer), tránh data loss |
| `D:\Code\SimulateSignal\tcp_simulator.py` | Tạo mới — TCP simulator để test |

---

## Update 2026-06-14 — Chart redesign + TelemetryServer.h sync fix

### Fix: TelemetryServer.h không đồng bộ với .cpp

`TelemetryServer.h` vẫn khai báo `parseSubPackets(const QByteArray& payload)` và thiếu `m_subBuffer`
sau khi session trước chỉ sync `.cpp` từ inner project sang outer.

**Fix:** Cập nhật `.h` để khớp `.cpp`:
- Đổi signature thành `void parseSubPackets();`
- Thêm `QByteArray m_subBuffer;` vào private members

→ Rebuild thành công. Binary mới có đúng fix Bug #6 (tích lũy bytes qua TCP frame).

### Chart redesign: SeparatedChart → OverlaidChart, layout 2×2

**Vấn đề:** Layout 4 chart xếp chồng dọc + CANoe-style separated lanes → khó đọc.

**Thay đổi:**
- Tạo mới `ui/OverlaidChart.h/.cpp`: tất cả signals vẽ chồng nhau trên 1 chart,
  nền trắng, có checkbox toggle từng signal, Y-axis giá trị vật lý thực.
- Cập nhật `ui/MultiChartWidget.h/.cpp`: layout 2×2 grid giống C# reference:
  - Top-left: **5U44** — K1, K2, Xung Hỏi (Y: -2 đến 50)
  - Top-right: **5A42** — 13 signals: K1, K2, ω1, ω2, ω3, DUK, ADC26V, ML1, ML2, ML3, SP1, SP2, SP3 (Y: -100 đến 100)
  - Bottom-left: **5E15** — Điện áp 150V, Nhiệt độ DSP, Số xung ×10³ (Y: 0 đến 160)
  - Bottom-right: **5I41** — 26VDC, 36VAC, 115VAC, Tần số/10, Th CCY, Th CO, Th SSP (Y: 0 đến 130)
- Cập nhật `CMakeLists.txt`: thêm `OverlaidChart.cpp/.h`

| File | Thay đổi |
|------|----------|
| `network/TelemetryServer.h` | Sync với .cpp: thêm `m_subBuffer`, đổi `parseSubPackets()` signature |
| `ui/OverlaidChart.h` | Tạo mới — overlaid chart widget |
| `ui/OverlaidChart.cpp` | Tạo mới — implementation |
| `ui/MultiChartWidget.h` | Đổi từ SeparatedChart → OverlaidChart |
| `ui/MultiChartWidget.cpp` | Layout 2×2, 13 signals 5A42, 7 signals 5I41 |
| `CMakeLists.txt` | Thêm OverlaidChart.cpp/.h |

---

## Update 2026-06-14 — Compact sidebar, larger chart text, drag-drop signals

### 1. Compact sidebar thay thế panel cuộn

**Vấn đề:** Panel trái có 5 bảng xếp lưới → phải kéo thanh lăn mới xem hết.

**Fix:** Thay bằng sidebar cố định 195px, 5 card xếp dọc không cần scroll:
- Header màu per block + nút "↗" để mở full panel trong dialog Qt::Tool
- Mỗi card hiện 2-4 thông số quan trọng nhất (Index, Status, K1/K2, ADC, Freq...)
- Full panels (Panel5A42, Panel5I41...) vẫn update live khi dialog ẩn

### 2. Chữ trên đồ thị to hơn

- OverlaidChart: checkbox label 7pt → **9pt bold**, axis Y font 7 → **8**, groupbox title 8pt → **10pt**
- Checkbox indicator size: 10×10 → **12×12 px**

### 3. Drag-drop signal từ sidebar vào chart

- Value label trong sidebar là `SignalDragLabel` — có cursor tay, tooltip "Kéo vào đồ thị"
- Kéo label vào vùng chart → signal tương ứng được bật checkbox (nếu chart có signal đó)
- MIME type: `application/x-gdt-signal` (signal name khớp với tên trong OverlaidChart)
- OverlaidChart bắt drop qua event filter trên QChartView

| File | Thay đổi |
|------|----------|
| `ui/OverlaidChart.h` | Thêm `eventFilter()` override |
| `ui/OverlaidChart.cpp` | Font lớn hơn; drop support qua event filter |
| `ui/MainWindow.h` | Thêm compact label members + dialog members |
| `ui/MainWindow.cpp` | `SignalDragLabel` class; `buildUi()` compact sidebar; `onUiTimer()` cập nhật labels |

---

## Update 2026-06-14 — Auto-prompt log name on startup & reset

**Yêu cầu:** Giống app C# cũ — khi mở app và sau mỗi lần Reset Server, tự động hỏi tên log để tránh quên lưu data.

**Thay đổi:**
- Tách logic "hỏi tên + mở log" thành `promptAndStartLog()` (private slot)
- **Startup**: `QTimer::singleShot(350ms)` gọi `promptAndStartLog()` sau khi window hiện
- **Reset Server (`onResetServer`)**: sau khi restart, gọi `promptAndStartLog()` — đóng log cũ, mở log mới
- **Menu "Bắt đầu ghi CSV"**: `onStartLog()` giờ delegate vào `promptAndStartLog()`
- Nếu user bấm Hủy hoặc để trống → không ghi log (không bắt buộc)

| File | Thay đổi |
|------|----------|
| `ui/MainWindow.h` | Thêm slot `promptAndStartLog()` |
| `ui/MainWindow.cpp` | Constructor + `onResetServer()` + `onStartLog()` + thêm `promptAndStartLog()` impl |

---

## Update 2026-06-14 — UI redesign: chart to fullscreen, error rate panel to popup

**Thay đổi:**
- `ErrorRatePanel` được chuyển ra khỏi main layout, đặt trong floating `QDialog` riêng.
- `MultiChartWidget` (4 đồ thị 5E/5I/5A/5U) giờ chiếm toàn bộ chiều cao panel phải.
- Thêm nút **📈 Tỷ lệ lỗi** trên toolbar — bấm để hiện/ẩn dialog tỷ lệ lỗi.
- InfoBar tỷ lệ lỗi: chiều cao 28→38px, font 9px→12px, hiển thị màu:
  - Xanh lá (OK) khi < 0.1%
  - Vàng (cảnh báo) khi 0.1–1%
  - Đỏ (lỗi) khi ≥ 1%
  - Xám (chưa nhận data)

| File | Thay đổi |
|------|----------|
| `ui/MainWindow.h` | Thêm `QDialog* m_errRateDlg`, slot `onToggleErrorRatePanel()` |
| `ui/MainWindow.cpp` | Redesign `buildUi()`, `buildMenuBar()`, `onUiTimer()`, thêm `onToggleErrorRatePanel()` |

---

## Update 2026-06-14 — Fix 5E15 CSV từ 9 cột → 165 cột (khớp C#)

**Vấn đề:** Qt log 5E15 chỉ có 9 cột vs C# có **165 cột** (3 outer + 162 model fields).
Các tool phân tích data kỳ vọng đúng 165 cột → file Qt không compatible.

**Phát hiện từ C# source (`GeneralModel.cs`, `Infor5E15BlockModel.cs`):**
- `CSV_DATA_HEADER_5E15VT` định nghĩa 165 cột (client + time + index + 162 model fields)
- `Infor5E15BlockModel` **chỉ parse 6 fields thực sự**: NhietDoDsp, DienAp150vNguon, SoXungPhatXaTxDuCS, Detonated, K3, Nlc
- 156 cột còn lại (IF spectrum IF_X/IF_Y, tichluy, các dòng điện/điện áp phụ) = 0 trong C# (chưa parse)
- Qt `Data5E15` struct đã có đúng 6 fields → **không cần thay đổi struct hay parser**

**Fix:** Chỉ cần cập nhật `CsvLogger.cpp`:
- `HDR_5E15`: copy chính xác `CSV_DATA_HEADER_5E15VT` từ C# (165 cột)
- `write5E15()`: viết đủ 165 giá trị per row, 6 field thực + 159 giá trị 0
- Column mapping (0-based từ model): [1]=nhietdoDsp, [2]=dinap150vNguon, [19]=detonated, [21]=k3, [24]=nlc, [32]=soxungphatxaTxduCS

| File | Thay đổi |
|------|----------|
| `storage/CsvLogger.cpp` | HDR_5E15: 9→165 cột; write5E15: output đủ 165 giá trị khớp C# |

---

## Update 2026-06-14 — Dashed line khi chưa nhận được packet

**Yêu cầu:** Khi server đang chạy nhưng một loại packet chưa được gửi tới (hoặc chưa nhận được), các đường tín hiệu trên chart tương ứng hiển thị **nét đứt** (`Qt::DashLine`) dưới dạng đường ngang tại y=0.  
Khi bắt đầu nhận data, đường chuyển về **nét liền** (`Qt::SolidLine`) như bình thường.

**Cơ chế:**
- `OverlaidChart` thêm flag `m_hasData = false`
- Constructor: set series pen `Qt::DashLine`, set `m_dirty = true` để vẽ placeholder ngay
- `refreshChart()`: nếu `!m_hasData` → vẽ flat dashed line tại y=0 cho mỗi visible series, return sớm
- `addSamples()`: lần đầu được gọi (`!m_hasData`) → switch tất cả series sang `Qt::SolidLine`, set `m_hasData = true`
- `reset()`: set `m_hasData = false`, switch lại `Qt::DashLine`, set `m_dirty = true`

Không cần thay đổi `MainWindow` hay `MultiChartWidget`.

| File | Thay đổi |
|------|----------|
| `ui/OverlaidChart.h` | Thêm `bool m_hasData = false` |
| `ui/OverlaidChart.cpp` | Constructor: DashLine pen + m_dirty=true; `addSamples()`: switch SolidLine on first call; `refreshChart()`: placeholder block; `reset()`: restore DashLine |

---

## Update 2026-06-14 — Kéo thả từ dialog chi tiết vào đồ thị

**Vấn đề:** Các label giá trị trong dialog chi tiết (↗) của từng khối không kéo được vào đồ thị, chỉ có sidebar mới kéo được.

**Fix:**
- Tách `SignalDragLabel` ra file riêng `ui/SignalDragLabel.h` (header-only, Q_OBJECT, không hardcode style)
- Xóa inline class khỏi `MainWindow.cpp`, thêm `#include "SignalDragLabel.h"`; `mkDrag()` lambda tự set style sidebar
- Mỗi Panel .cpp dùng `SignalDragLabel(sigName)` thay `QLabel("---")` cho các label có signal trên chart
- .h files không đổi — C++ polymorphism: `SignalDragLabel*` gán vào `QLabel*` member hợp lệ

**Mapping label → chart signal:**

| Panel | Label | Chart signal |
|-------|-------|-------------|
| 5E15 | m_lTemp | "Nhiệt độ DSP" |
| 5E15 | m_lDienAp | "Điện áp 150V" |
| 5E15 | m_lXung | "Xung ×10³" |
| 5A42 | m_lK1/K2 | "K1"/"K2" |
| 5A42 | m_lAdc26 | "ADC26V" |
| 5A42 | m_lMl1/2/3 | "ML1"/"ML2"/"ML3" |
| 5A42 | m_lDuc1/2/3 | "ω1"/"ω2"/"ω3" |
| 5A42 | m_lDuk | "DUK" |
| 5A42 | m_lSp1/2/3 | "SP1"/"SP2"/"SP3" |
| 5U44 | m_lK1/K2 | "K1"/"K2" |
| 5U44 | m_lXungHoi | "Xung Hỏi" |
| 5I41 | m_lAdc26/36/115 | "26VDC"/"36VAC"/"115VAC" |
| 5I41 | m_lFreq | "Tần số/10" |

| File | Thay đổi |
|------|----------|
| `ui/SignalDragLabel.h` | Tạo mới — shared draggable label header |
| `ui/MainWindow.cpp` | Xóa inline class; thêm include; mkDrag() set style |
| `ui/Panel5E15.cpp` | mkVal() dùng SignalDragLabel cho 3 labels |
| `ui/Panel5A42.cpp` | makeValueLabel(sigName) dùng SignalDragLabel cho 13 labels |
| `ui/Panel5U44.cpp` | makeValueLabel(sigName) dùng SignalDragLabel cho 3 labels |
| `ui/Panel5I41.cpp` | mkVal(sig) dùng SignalDragLabel cho 4 labels |
| `ui/Panel5A42.h` | Cập nhật signature makeValueLabel |
| `ui/Panel5U44.h` | Cập nhật signature makeValueLabel |
| `CMakeLists.txt` | Thêm SignalDragLabel.h vào HEADERS |

---

## Update 2026-06-14 — Chart controls: Clear All, Show All, Pause+Zoom

**Thêm vào mỗi OverlaidChart (4 charts):**

### Nút điều khiển (góc phải của checkbox row)
- **✓ Tất cả** — bật visible tất cả signals + checkbox
- **✗ Xóa** — ẩn tất cả signals + uncheck
- **⏸ / ▶ Dừng** — toggle freeze display

### Chế độ Dừng (khi bấm ⏸)
- Nền chart chuyển vàng nhạt → hiển thị trực quan "đang dừng"
- **Kéo chuột trái** → zoom vùng (rubber-band `RectangleRubberBand`)
- **Cuộn chuột** → zoom in/out (factor ×1.15)
- **Chuột phải** → reset zoom về toàn bộ
- Data vẫn tích lũy trong ring buffer khi dừng
- Khi bấm ▶ tiếp tục: zoom reset, auto-scale tiếp tục, refresh ngay

### Tooltip giá trị live
- Mỗi checkbox cập nhật tooltip = tên signal + giá trị hiện tại
- Ví dụ: "K1\n▶ 0.123"

### Reset behavior
- `reset()` unfreeze pause, reset zoom, restore dashed placeholder

| File | Thay đổi |
|------|----------|
| `ui/OverlaidChart.h` | Thêm `m_paused`, `m_btnPause`, slots `clearAll()`/`showAll()` |
| `ui/OverlaidChart.cpp` | Constructor: 3 control buttons + separator; `addSamples()`: tooltip live; `refreshChart()`: pause guard; `eventFilter()`: wheel zoom + right-click reset; `clearAll()`/`showAll()`/`reset()` |
