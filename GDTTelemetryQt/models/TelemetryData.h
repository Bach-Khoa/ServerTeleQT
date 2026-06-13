#pragma once
#include <QString>
#include <cstdint>

namespace GDT {

// ============================================================
// 5I41 - Inertial unit (embedded in Telemetry packet)
// ============================================================
struct Data5I41 {
    double adc26  = 0;   // [V]
    double adc36  = 0;   // [V]
    double adc115 = 0;   // [V]
    uint32_t freq = 0;   // 1000 Hz counter
};

// ============================================================
// Telemetry block (5V27)
// ============================================================
struct DataTelemetry {
    QString  client;
    uint32_t index  = 0;
    Data5I41 infor5I41;
    int16_t  xGyro  = 0;
    int16_t  yGyro  = 0;
    int16_t  zGyro  = 0;
    int16_t  xAccl  = 0;
    int16_t  yAccl  = 0;
    int16_t  zAccl  = 0;
    uint8_t  status5A = 0;
    uint8_t  status5U = 0;
};

// ============================================================
// 5A42 - Control unit (52 bytes)
// Field layout matches C# server AnalystDataService offsets
// ============================================================
struct Data5A42 {
    QString  client;
    uint32_t index   = 0;

    // Analog data
    double adc26  = 0;    // [V]  uint16 * 10.24 * 3.19 / 65535
    double ml1    = 0;    // int16 / 512
    double ml2    = 0;
    double ml3    = 0;
    double sp1    = 0;    // setPoint, int16 / 512
    double sp2    = 0;
    double sp3    = 0;
    double k1     = 0;    // int16 * 0.001
    double k2     = 0;
    double duc1   = 0;    // int16 / 128
    double duc2   = 0;
    double duc3   = 0;
    double duk    = 0;    // int16 / 128
    double omega1 = 0;    // int16 / 128
    double omega2 = 0;
    double omega3 = 0;
    double kernelGama   = 0;   // int16 / 128
    double uforsGama    = 0;
    double kernelOffset = 0;
    double uforsOffset  = 0;

    // Status byte 0 [offset 4]
    bool bOmega1   = false;  // bit0
    bool bOmega2   = false;  // bit1
    bool bOmega3   = false;  // bit2
    bool bGama     = false;  // bit3
    bool bADC26    = false;  // bit4
    bool bCtrlML1  = false;  // bit5
    bool bCtrlML2  = false;  // bit6
    bool bCtrlML3  = false;  // bit7

    // Status byte 1 [offset 5]
    bool systemInit    = false;  // bit0
    bool adcSP         = false;  // bit1
    bool adcFB         = false;  // bit2
    bool ratioIntegral = false;  // bit3
    bool gamaEnable    = false;  // bit4
    bool b5uDigital    = false;  // bit5
    bool kernelSensor  = false;  // bit6
    bool uforsSensor   = false;  // bit7

    // Kernel status byte [offset 46]
    bool sensorComm    = false;  // bit0
    bool sensorConfig  = false;  // bit1
    bool xRate         = false;  // bit2
    bool yRate         = false;  // bit3
    bool zRate         = false;  // bit4
    bool tempOk        = false;  // bit6

    // Ufors status byte [offset 47]
    bool errorIn          = false;  // bit0
    bool tempWarn         = false;  // bit3
    bool controlLoopError = false;  // bit4
    bool hardwareError    = false;  // bit5
    bool rangeExceed      = false;  // bit6

    uint16_t sIndex = 0;
};

// ============================================================
// 5U44 - RF Receiver
// ============================================================
struct Data5U44 {
    QString  client;
    uint32_t index = 0;

    // IO byte
    bool soiDot      = false;  // bit7
    bool aNode       = false;  // bit6
    bool tachTangVao = false;  // bit5
    bool tachTangRa  = false;  // bit4
    bool ranh        = false;  // bit3 (0=ranh2, 1=ranh1)
    bool chuyenDoDoc = false;  // bit2
    bool k3          = false;  // bit1
    bool k6          = false;  // bit0

    // DSP byte
    bool    k7     = false;  // bit5
    bool    rxLock = false;  // bit4
    bool    txLock = false;  // bit3
    int     phach  = 0;      // bits[2:0] - hoán vị

    int     suyHao   = 0;   // byte unsigned
    int     congSuat = 0;   // int16
    int     xungHoi  = 0;   // byte
    double  k1       = 0;   // int16 / 1000
    double  k2       = 0;

    // Self-test (status byte 0)
    bool ad9643  = false;  // bit3
    bool ad9523  = false;  // bit2
    bool adf4360 = false;  // bit1
    bool adf5355 = false;  // bit0

    // Live-test (status byte 1)
    bool voltage26    = false;  // bit5
    bool currentTxRx  = false;  // bit4
    bool current5VFPGA= false;  // bit3
    bool voltage40    = false;  // bit2
    bool voltage575   = false;  // bit1
    bool connection5A = false;  // bit0

    uint16_t sIndex = 0;
};

// ============================================================
// 5I41Block - Standalone power supply unit (20 bytes)
// Each packet carries 6 uint16LE measurements for one groupId
// Full picture assembled from groupId 0x00..0x04
// ============================================================
struct Data5I41Block {
    QString  client;
    uint32_t index      = 0;
    uint8_t  groupId    = 0;
    uint8_t  sampleTime = 0;

    // Group 0: voltages [V] (raw * 0.01)
    double u26VPWR   = 0;
    double u26VML    = 0;
    double u26V5U44  = 0;
    double u26V5A42  = 0;
    double u115V5U44 = 0;
    double u115V5A42 = 0;

    // Group 1: voltages + current + frequency
    double u115V5E15 = 0;
    double u36VPWR   = 0;
    double freqPWR   = 0;  // raw uint16, no scaling
    double i26VPWR   = 0;
    double i26VML    = 0;
    double i26V5U44  = 0;

    // Group 2: currents
    double i26V5A42  = 0;
    double i26VMF    = 0;
    double i115V5U44 = 0;
    double i115V5E15 = 0;
    double i36VAPWR  = 0;
    double i36VBPWR  = 0;

    // Group 3: temperatures
    double thKhBkkn  = 0;
    double thKhPn    = 0;
    double th26VMfss = 0;
    double thChTltp  = 0;
    double thFbCb    = 0;
    double thThaoHam = 0;

    // Group 4: temperatures (cont.)
    double thK3      = 0;
    double thKhDcp   = 0;
    double thCo      = 0;
    double thCcy     = 0;
    double thSsp     = 0;
    double thDp      = 0;
};

// ============================================================
// 5E15 - Transmitter
// ============================================================
struct Data5E15 {
    double   nhietDoDsp     = 0;   // [°C]
    double   dienAp150V     = 0;   // [V]
    uint32_t soXungPhatXa   = 0;
    bool     detonated      = false;
    bool     k3             = false;
    bool     nlc            = false;
};

} // namespace GDT
