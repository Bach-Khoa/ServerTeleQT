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
