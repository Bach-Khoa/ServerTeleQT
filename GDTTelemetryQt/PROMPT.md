# Prompt hiểu repo GDTTelemetryTLDK35

## Mục đích
Repository này là hệ thống thu thập và hiển thị dữ liệu telemetry thời gian thực cho tổ hợp tên lửa TLDK35 (5V27). Được viết bằng C# WPF (.NET 4.7.2), kiến trúc MVVM.

## Hệ thống gồm 4 luồng dữ liệu song song

| Khối | Tên | Vai trò | Kích thước gói |
|------|-----|---------|----------------|
| Telemetry | 5V27 | Con quay hồi chuyển + gia tốc + nguồn | 30 byte |
| 5A42 | Bộ điều khiển động cơ lái | K1/K2, ML1-3, DUC1-3, DUK, điểm đặt | 36 byte |
| 5U44 | Bộ thu RF | IO bits, DSP flags, suy hao, công suất | 20 byte |
| 5E15 | Bộ phát TX | Nhiệt độ DSP, điện áp 150V, số xung phát | 184/186 byte |

## Cấu trúc gói tin TCP

```
TCP Wrapper (12 byte overhead):
  Header: DE AD C0 DF  (4 byte)
  Type:   01=Tele, 02=5E15, A1-A5=file ops  (1 byte)
  [payload]
  Tailer: EE ED FB CE  (4 byte)

Payload gói Telemetry/5A42/5U44:
  Header: AF  (1 byte)
  Seq:    [1 byte]  index[1]
  Data:   ...
  Tailer: FA  (1 byte)
  CRC8:   [1 byte]

Payload gói 5E15 (multicast UDP 225.0.0.5:20020):
  Header: AB CD EF FE  (4 byte)
  Data:   ...
  Tailer: E1 E2 E3  (3 byte)
  CRC16:  [2 byte] (poly 0x1021, init 0xFFFF)
```

## Byte offsets trong payload (sau khi bóc TCP wrapper)

### Telemetry (30 byte, CRC8)
```
[4..7]   ADC36  (2x uint16 big-endian, trung bình) × 0.0001875 × 15
[8..9]   ADC115 (uint16 big-endian) × 0.0001875 × 44.2
[10..11] ADC26  (uint16 big-endian) × 0.0001875 × 10
[12..13] FREQ   (uint16 big-endian) = tần số 1000Hz
[14..15] X_Gyro (int16 big-endian)
[16..17] Y_Gyro (int16 big-endian)
[18..19] Z_Gyro (int16 big-endian)
[20..21] X_Accl (int16 big-endian)
[22..23] Y_Accl (int16 big-endian)
[24..25] Z_Accl (int16 big-endian)
[26]     Status 5A42 (byte)
[27]     Status 5U44 (byte)
```

### 5A42 (36 byte, CRC8)
```
[1]      Sequence index
[4..5]   K1   = (uint16_BE - 2000) / 1000.0
[6..7]   K2   = (uint16_BE - 2000) / 1000.0
[8..9]   ML1  (uint16_BE raw ADC)
[10..11] ML2  (uint16_BE raw ADC)
[12..13] ML3  (uint16_BE raw ADC)
[14..15] DUC1 (uint16_BE raw ADC)
[16..17] DUC2 (uint16_BE raw ADC)
[18..19] DUC3 (uint16_BE raw ADC)
[20..21] DUK  (int16_BE raw ADC)
[22..23] ADC26 (uint16_BE raw) → / 65535 × 10.24 × 3.2117 [V]
[24..25] SetPoint1 = uint16_BE / 100.0 - 40.0
[26..27] SetPoint2 = uint16_BE / 100.0 - 40.0
[28..29] SetPoint3 = uint16_BE / 100.0 - 40.0
[30..31] SIndex (uint16_BE)
[32]     Status byte 0: bit0=BDUC1, bit1=BDUC2, bit2=BDUC3, bit3=BDUK,
                        bit4=BVoltage26, bit5=BML1, bit6=BML2, bit7=BML3
[33]     Status byte 1: bit0=BFlashConfig, bit1=BADCCamBien, bit2=BADCML,
                        bit3=DieuKhien, bit4=ThaoHam, bit5=KetNoi5U
```

### 5U44 (20 byte, CRC8)
```
[1]      Sequence index
[4]      IO byte:  bit7=SoiDot, bit6=Anode, bit5=TachTangVao, bit4=TachTangRa,
                   bit3=Ranh, bit2=ChuyenDoDoc, bit1=K3, bit0=K6
[5]      DSP byte: bit5=K7, bit4=RxLock, bit3=TxLock, bit[2:0]=Phach(hoán vị bit)
[6]      SuyHao (byte, unsigned)
[7..8]   CongSuat (int16_BE)
[9]      XungHoi (byte)
[10..11] K1 = int16_BE / 1000.0
[12..13] K2 = int16_BE / 1000.0
[14]     SelfTest: bit3=AD9643, bit2=AD9523, bit1=ADF4360, bit0=ADF5355
[15]     LiveTest: bit5=Voltage26, bit4=CurrentTxRx, bit3=Current5VFPGA,
                   bit2=Voltage40, bit1=Voltage575, bit0=Connection5A
[16..17] SIndex (uint16_BE)
```

### 5E15-VT (184/186 byte, CRC16 poly 0x1021)
```
[6..7]   NhietDoDsp  = ((data[6]&0x0F)<<8 | (data[7]&0xFC)>>2) × 0.0625
[7..9]   DienAp150V  = ((data[7]&0x03)<<9 | data[8]<<1 | (data[9]&0x80)>>7) × 0.2082
[27]     bit7=Detonated, bit5=K3, bit1=NLC
[30..32] SoXungPhatXa = 3-byte big-endian uint
```

## Công thức tính ADC 5A42

```
para_26 = adc26_raw / 54270.0          // chuẩn hóa theo nguồn 26V
result  = (val - min × para_26) / (max × para_26 - min × para_26) × mul - plus
```

Hệ số cho từng kênh:
- ML1/ML2/ML3: min=891/530/48595, max=49809/50090/1287, mul=60, plus=30
- DUC1/DUC2:   min=6108/44689, max=45492/6271, mul=160, plus=80
- DUC3:        min=5827, max=45293, mul=400, plus=200
- DUK:         min=8324, max=42636, mul=40, plus=20

## Mạng

- TCP server lắng nghe: `192.168.168.30:16022` (primary)
- TCP client forward: `192.168.168.35:16025` (secondary)
- UDP multicast 5E15: `225.0.0.5:20020`

## Database SQLite

2 bảng:
- `TcpServerDatabaseModel` (ID="TCPSERVERPARAM"): cấu hình IP/port server
- `Setup5A42DatabaseModel` (ID="5A42PARAM"): min/max ADC các kênh ML, DUC, DUK

## Lưu trữ CSV

```
Desktop/DataLog5V27VT/
├── {tên}_5V27VT_Telemetry_{timestamp}.csv    (14 cột)
├── {tên}_5V27VT_5A42VT_{timestamp}.csv       (30 cột)
├── {tên}_5V27VT_5U44VT_{timestamp}.csv       (30 cột)
└── {tên}_5V27VT_5E15VT_{timestamp}.text      (165 cột)
```

## CRC algorithms

- CRC8 (poly 0x07): dùng cho Telemetry, 5A42, 5U44
- CRC16 (poly 0x8005, init=0x0000): validate 5E15
- CRC16 (poly 0x1021, init=0xFFFF): dùng cho 5E15-FM
- CRC32 (poly 0x04C11DB7, input+result reflect, init=xor=0xFFFFFFFF)

## Kiến trúc xử lý đa luồng

```
TCP client kết nối → ConcurrentQueue<byte[]>
                         ↓
                 Thread parse gói tin
                    ↙    ↓    ↘    ↘
               Tele  5A42  5U44  5E15(UDP)
                 ↓    ↓     ↓     ↓
              Ghi CSV + Cập nhật UI
```

## Mục tiêu viết lại bằng Qt C++

Dùng:
- Qt6 Widgets cho UI
- QTcpServer + QTcpSocket cho TCP
- QUdpSocket cho UDP multicast
- QSqlDatabase (sqlite driver) cho database
- QtCharts cho biểu đồ thời gian thực
- QThread + QMutex + QQueue cho đa luồng
- Signals/Slots thay cho Observer pattern
