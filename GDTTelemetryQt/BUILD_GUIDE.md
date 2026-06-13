# Hướng dẫn Build GDTTelemetryTLDK35 Qt C++

## Yêu cầu
- **Qt 6.x** (đã test Qt 6.11.1 MinGW 64-bit)  
  Tải tại: https://www.qt.io/download-qt-installer
- **Qt Creator 13+** (đi kèm khi cài Qt)
- **CMake 3.16+** (đi kèm trong Qt Tools)
- **MinGW 13.x** hoặc **MSVC 2022** (đi kèm khi cài Qt)

---

## Cách 1: Mở bằng Qt Creator (Khuyến nghị)

### Bước 1 — Mở project
1. Mở **Qt Creator**
2. Menu **File → Open File or Project...**
3. Chọn file `d:\code\GDTTelemetryQt\CMakeLists.txt`
4. Click **Open**

### Bước 2 — Chọn Kit
Hộp thoại **Configure Project** xuất hiện:
- Chọn kit: `Desktop Qt 6.11.1 MinGW 64-bit`
- Nếu chưa có kit: vào **Tools → Options → Kits** để thêm

### Bước 3 — Cấu hình CMake
Qt Creator sẽ tự chạy CMake configure. Nếu thấy lỗi:
- Vào **Projects** (thanh bên trái) → **Build Settings**
- Kiểm tra **CMake** section có đúng prefix path chưa

### Bước 4 — Build
- Nhấn **Ctrl+B** hoặc click nút **Build** (icon búa)
- Build lần đầu ~1-2 phút

### Bước 5 — Run
- Nhấn **Ctrl+R** hoặc click nút **Run** (icon tam giác xanh)
- App sẽ chạy trực tiếp trong Qt Creator

---

## Cách 2: Command Line (CMake + Ninja)

```batch
:: Thiết lập PATH (thay đường dẫn nếu Qt cài chỗ khác)
set PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;%PATH%
set QT_DIR=C:\Qt\6.11.1\mingw_64

:: Tạo thư mục build
mkdir d:\code\GDTTelemetryQt\build
cd d:\code\GDTTelemetryQt\build

:: Configure
cmake -S .. -B . ^
      -G "Ninja" ^
      -DCMAKE_PREFIX_PATH=%QT_DIR% ^
      -DCMAKE_BUILD_TYPE=Release

:: Build
cmake --build . --parallel
```

Exe output: `build\GDTTelemetryTLDK35.exe`

---

## Cách 3: Build Release với windeployqt

Sau khi build Release, cần copy Qt DLLs để chạy được ngoài build directory:

```batch
set PATH=C:\Qt\Tools\mingw1310_64\bin;%PATH%

:: Build Release
cmake --build d:\code\GDTTelemetryQt\build --config Release

:: Copy DLLs cần thiết
set EXE=d:\code\GDTTelemetryQt\build\GDTTelemetryTLDK35.exe
set DEPLOY_DIR=d:\code\GDTTelemetryQt\deploy

mkdir %DEPLOY_DIR%
copy %EXE% %DEPLOY_DIR%

C:\Qt\6.11.1\mingw_64\bin\windeployqt.exe ^
    --release ^
    --no-translations ^
    --dir %DEPLOY_DIR% ^
    %DEPLOY_DIR%\GDTTelemetryTLDK35.exe
```

---

## Cấu trúc project

```
GDTTelemetryQt/
├── CMakeLists.txt          ← CMake build file
├── main.cpp                ← Entry point
├── core/
│   ├── Constants.h         ← Byte offsets, CRC tables, protocol constants
│   ├── PacketParser.h      ← Binary packet parser interface
│   └── PacketParser.cpp    ← CRC8/16/32 + parse Tele/5A42/5U44/5E15
├── models/
│   ├── TelemetryData.h     ← Data structs: DataTelemetry, Data5A42, Data5U44, Data5E15
│   └── AppConfig.h         ← ServerConfig + Calib5A42
├── network/
│   ├── TelemetryServer.h/cpp   ← QTcpServer, per-client threads
│   └── MulticastReceiver.h/cpp ← QUdpSocket multicast 225.0.0.5:20020
├── storage/
│   ├── DatabaseManager.h/cpp   ← SQLite (QSqlDatabase)
│   └── CsvLogger.h/cpp         ← CSV file logging
└── ui/
    ├── MainWindow.h/cpp        ← Main window, menu, layout
    ├── Panel5A42.h/cpp         ← K1/K2/ML/DUC display + LED status
    ├── Panel5U44.h/cpp         ← RF receiver display + LED status
    ├── Panel5E15.h/cpp         ← Transmitter display
    ├── Panel5I41.h/cpp         ← Inertial unit display
    ├── PanelTelemetry.h/cpp    ← Telemetry stream info
    ├── ChartPanel.h/cpp        ← Real-time QtCharts
    ├── LedIndicator.h/cpp      ← Custom LED widget
    ├── SetupServerDialog.h/cpp ← TCP server config dialog
    └── Setup5A42Dialog.h/cpp   ← ADC calibration dialog
```

---

## Troubleshooting

| Lỗi | Giải pháp |
|-----|-----------|
| `Qt6Charts not found` | Chắc chắn đã cài Qt Charts khi cài Qt (tick "Qt Charts" trong installer) |
| `Cannot find -lQt6Sql` | Cài thêm Qt SQL module |
| `windeployqt: not found` | Tìm tại `C:\Qt\6.x.x\mingw_64\bin\windeployqt.exe` |
| App chạy không mở được (missing DLL) | Chạy `windeployqt` như hướng dẫn Cách 3 |
| Server không lắng nghe | Kiểm tra firewall Windows cho port 16022 |
| Không join multicast | Chắc chắn network interface hỗ trợ multicast |

---

## Lần build đã test

```
Qt:       6.11.1 MinGW 64-bit
CMake:    4.1.0
Compiler: GCC 13.1.0 (MinGW)
OS:       Windows 11 Pro
Thời gian build: ~45 giây (Ninja, 4 core)
Exe size: 11.9 MB (Debug)
```
