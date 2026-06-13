#include "PacketParser.h"
#include "Constants.h"
#include <cmath>

namespace GDT {

PacketParser::PacketParser(const Calib5A42& calib) : m_calib(calib) {}

void PacketParser::setCalib(const Calib5A42& calib) { m_calib = calib; }

// ============================================================
// Static byte helpers
// ============================================================
uint16_t PacketParser::u16BE(const QByteArray& d, int i) {
    return (uint16_t)(((uint8_t)d[i] << 8) | (uint8_t)d[i+1]);
}
int16_t PacketParser::i16BE(const QByteArray& d, int i) {
    return (int16_t)(((uint8_t)d[i] << 8) | (uint8_t)d[i+1]);
}
uint32_t PacketParser::u24BE(const QByteArray& d, int i) {
    return ((uint32_t)(uint8_t)d[i] << 16) | ((uint32_t)(uint8_t)d[i+1] << 8) | (uint8_t)d[i+2];
}
uint32_t PacketParser::u32BE(const QByteArray& d, int i) {
    return ((uint32_t)(uint8_t)d[i]   << 24) | ((uint32_t)(uint8_t)d[i+1] << 16) |
           ((uint32_t)(uint8_t)d[i+2] <<  8) |  (uint32_t)(uint8_t)d[i+3];
}
uint16_t PacketParser::u16LE(const QByteArray& d, int i) {
    return (uint16_t)((uint8_t)d[i] | ((uint8_t)d[i+1] << 8));
}
uint32_t PacketParser::u32LE(const QByteArray& d, int i) {
    return (uint32_t)(uint8_t)d[i]          |
           ((uint32_t)(uint8_t)d[i+1] <<  8) |
           ((uint32_t)(uint8_t)d[i+2] << 16) |
           ((uint32_t)(uint8_t)d[i+3] << 24);
}
bool PacketParser::bit(uint8_t b, int pos) {
    return (b & (1 << pos)) != 0;
}

// ============================================================
// ADC 5A42 calibration formula
// ============================================================
double PacketParser::calAdc5A42(double val, double minV, double maxV, int mul, int plus, double adc26Raw) const {
    double para26 = adc26Raw / 54270.0;
    double denom  = maxV * para26 - minV * para26;
    if (std::abs(denom) < 1e-9) return 0.0;
    return (val - minV * para26) / denom * mul - plus;
}

// ============================================================
// TCP wrapper stripper
// Frame: [DE][AD][type][len_H][len_L][data:len][EE][ED][CRC_H][CRC_L]
// ============================================================
PacketParser::TcpFrame PacketParser::stripTcpWrapper(const QByteArray& raw) const {
    TcpFrame frame;
    // Minimum frame: 2+1+2+0+2+2 = 9 bytes
    if (raw.size() < 9) return frame;
    if ((uint8_t)raw[0] != TCP_HDR1 || (uint8_t)raw[1] != TCP_HDR2) return frame;
    uint16_t dlen = (uint16_t)(((uint8_t)raw[3] << 8) | (uint8_t)raw[4]);
    int frameLen  = 2 + 1 + 2 + (int)dlen + 2 + 2;
    if (raw.size() < frameLen) return frame;
    int tailerPos = 5 + (int)dlen;
    if ((uint8_t)raw[tailerPos] != TCP_TLR1 || (uint8_t)raw[tailerPos+1] != TCP_TLR2) return frame;
    if (!checkCrc16_8005(raw.left(frameLen))) return frame;
    frame.type    = (uint8_t)raw[2];
    frame.payload = raw.mid(5, (int)dlen);
    return frame;
}

// ============================================================
// CRC8
// ============================================================
bool PacketParser::checkCrc8(const QByteArray& data) const {
    if (data.isEmpty()) return false;
    uint8_t crc = 0;
    for (int i = 0; i < data.size() - 1; ++i)
        crc = CRC8_TABLE[(uint8_t)data[i] ^ crc];
    return crc == (uint8_t)data[data.size()-1];
}

// ============================================================
// CRC16 poly 0x1021 init=0xFFFF
// ============================================================
bool PacketParser::checkCrc16_1021(const QByteArray& data) const {
    if (data.size() < 2) return false;
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < data.size() - 2; ++i)
        crc = (uint16_t)((crc << 8) ^ CRC16_1021_TABLE[(crc >> 8) ^ (uint8_t)data[i]]);
    uint16_t expected = ((uint8_t)data[data.size()-2] << 8) | (uint8_t)data[data.size()-1];
    return crc == expected;
}

// ============================================================
// CRC16 poly 0x8005 init=0x0000
// ============================================================
bool PacketParser::checkCrc16_8005(const QByteArray& data) const {
    if (data.size() < 2) return false;
    uint16_t crc = 0x0000;
    for (int i = 0; i < data.size() - 2; ++i)
        crc = (uint16_t)((crc << 8) ^ CRC16_8005_TABLE[(crc >> 8) ^ (uint8_t)data[i]]);
    uint16_t stored = ((uint8_t)data[data.size()-2] << 8) | (uint8_t)data[data.size()-1];
    return (crc & 0xFFFF) == stored;
}

// ============================================================
// CRC32 (input+result reflect, init=0xFFFFFFFF, xor=0xFFFFFFFF)
// ============================================================
static uint8_t reflect8(uint8_t d) {
    uint8_t r = 0;
    for (int i = 0; i < 8; ++i) { r = (r << 1) | (d & 1); d >>= 1; }
    return r;
}
static uint32_t reflect32(uint32_t d) {
    uint32_t r = 0;
    for (int i = 0; i < 32; ++i) { r = (r << 1) | (d & 1); d >>= 1; }
    return r;
}

bool PacketParser::checkCrc32(const QByteArray& data) const {
    if (data.size() < 4) return false;
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < data.size() - 4; ++i) {
        uint8_t ref = reflect8((uint8_t)data[i]);
        uint8_t pos = (uint8_t)((crc ^ ((uint32_t)ref << 24)) >> 24);
        crc = (crc << 8) ^ CRC32_TABLE[pos];
    }
    crc = reflect32(crc) ^ 0xFFFFFFFF;
    int n = data.size();
    uint32_t stored = ((uint32_t)(uint8_t)data[n-1] << 24) | ((uint32_t)(uint8_t)data[n-2] << 16) |
                      ((uint32_t)(uint8_t)data[n-3] <<  8) |  (uint32_t)(uint8_t)data[n-4];
    return crc == stored;
}

// ============================================================
// Telemetry parser
// ============================================================
DataTelemetry PacketParser::parseTelemetry(const QByteArray& d, uint32_t index, const QString& client) {
    DataTelemetry t;
    t.client = client;
    t.index  = index;

    double adc36a = u16BE(d, IDX_TELE_ADC36)   * PARAM_ADC_TELE;
    double adc36b = u16BE(d, IDX_TELE_ADC36+2) * PARAM_ADC_TELE;
    t.infor5I41.adc36  = (adc36a + adc36b) / 2.0 * PARAM_U36_TELE;
    t.infor5I41.adc115 = u16BE(d, IDX_TELE_ADC115) * PARAM_ADC_TELE * PARAM_U115_TELE;
    t.infor5I41.adc26  = u16BE(d, IDX_TELE_ADC26)  * PARAM_ADC_TELE * PARAM_U26_TELE;
    t.infor5I41.freq   = u16BE(d, IDX_TELE_FREQ);

    t.xGyro = i16BE(d, IDX_TELE_XGYRO);
    t.yGyro = i16BE(d, IDX_TELE_YGYRO);
    t.zGyro = i16BE(d, IDX_TELE_ZGYRO);
    t.xAccl = i16BE(d, IDX_TELE_XACCL);
    t.yAccl = i16BE(d, IDX_TELE_YACCL);
    t.zAccl = i16BE(d, IDX_TELE_ZACCL);
    t.status5A = (uint8_t)d[IDX_TELE_ST5A42];
    t.status5U = (uint8_t)d[IDX_TELE_ST5U44];
    return t;
}

// ============================================================
// 5A42 parser — offsets and formulas match C# AnalystDataService
// ============================================================
Data5A42 PacketParser::parse5A42(const QByteArray& d, uint32_t index, const QString& client) {
    Data5A42 a;
    a.client = client;
    a.index  = index;

    a.adc26  = (double)u16BE(d, IDX_5A42_ADC26) * 10.24 * 3.19 / 65535.0;
    a.ml1    = i16BE(d, IDX_5A42_ML1)    / 512.0;
    a.ml2    = i16BE(d, IDX_5A42_ML2)    / 512.0;
    a.ml3    = i16BE(d, IDX_5A42_ML3)    / 512.0;
    a.sp1    = i16BE(d, IDX_5A42_SP1)    / 512.0;
    a.sp2    = i16BE(d, IDX_5A42_SP2)    / 512.0;
    a.sp3    = i16BE(d, IDX_5A42_SP3)    / 512.0;
    a.k1     = i16BE(d, IDX_5A42_K1)     * 0.001;
    a.k2     = i16BE(d, IDX_5A42_K2)     * 0.001;
    a.duc1   = i16BE(d, IDX_5A42_DUC1)   / 128.0;
    a.duc2   = i16BE(d, IDX_5A42_DUC2)   / 128.0;
    a.duc3   = i16BE(d, IDX_5A42_DUC3)   / 128.0;
    a.duk    = i16BE(d, IDX_5A42_DUK)    / 128.0;
    a.omega1 = i16BE(d, IDX_5A42_OMEGA1) / 128.0;
    a.omega2 = i16BE(d, IDX_5A42_OMEGA2) / 128.0;
    a.omega3 = i16BE(d, IDX_5A42_OMEGA3) / 128.0;
    a.kernelGama   = i16BE(d, IDX_5A42_KGAMA)   / 128.0;
    a.uforsGama    = i16BE(d, IDX_5A42_UGAMA)   / 128.0;
    a.kernelOffset = i16BE(d, IDX_5A42_KOFFSET) / 128.0;
    a.uforsOffset  = i16BE(d, IDX_5A42_UOFFSET) / 128.0;

    uint8_t st0 = (uint8_t)d[IDX_5A42_STATUS];
    uint8_t st1 = (uint8_t)d[IDX_5A42_STATUS + 1];

    a.bOmega1   = bit(st0, 0);
    a.bOmega2   = bit(st0, 1);
    a.bOmega3   = bit(st0, 2);
    a.bGama     = bit(st0, 3);
    a.bADC26    = bit(st0, 4);
    a.bCtrlML1  = bit(st0, 5);
    a.bCtrlML2  = bit(st0, 6);
    a.bCtrlML3  = bit(st0, 7);

    a.systemInit    = bit(st1, 0);
    a.adcSP         = bit(st1, 1);
    a.adcFB         = bit(st1, 2);
    a.ratioIntegral = bit(st1, 3);
    a.gamaEnable    = bit(st1, 4);
    a.b5uDigital    = bit(st1, 5);
    a.kernelSensor  = bit(st1, 6);
    a.uforsSensor   = bit(st1, 7);

    uint8_t kst = (uint8_t)d[IDX_5A42_KSTATUS];
    a.sensorComm   = bit(kst, 0);
    a.sensorConfig = bit(kst, 1);
    a.xRate        = bit(kst, 2);
    a.yRate        = bit(kst, 3);
    a.zRate        = bit(kst, 4);
    a.tempOk       = bit(kst, 6);

    uint8_t ust = (uint8_t)d[IDX_5A42_USTATUS];
    a.errorIn          = bit(ust, 0);
    a.tempWarn         = bit(ust, 3);
    a.controlLoopError = bit(ust, 4);
    a.hardwareError    = bit(ust, 5);
    a.rangeExceed      = bit(ust, 6);

    a.sIndex = u16BE(d, IDX_5A42_SIDX);
    return a;
}

// ============================================================
// 5I41 parser — standalone power supply packet
// 6 uint16 little-endian values per groupId, scale 0.01 each
// ============================================================
Data5I41Block PacketParser::parse5I41(const QByteArray& d, uint32_t index, const QString& client) {
    Data5I41Block b;
    b.client     = client;
    b.index      = index;
    b.groupId    = (uint8_t)d[IDX_5I41_GROUPID];
    b.sampleTime = (uint8_t)d[IDX_5I41_STIME];

    // Little-endian uint16: low byte at i, high byte at i+1
    auto u16le = [&](int i) -> double {
        return (double)(((uint8_t)d[i+1] << 8) | (uint8_t)d[i]);
    };

    constexpr double K = 0.01;
    switch (b.groupId) {
        case 0x00:
            b.u26VPWR   = u16le(5)  * K;
            b.u26VML    = u16le(7)  * K;
            b.u26V5U44  = u16le(9)  * K;
            b.u26V5A42  = u16le(11) * K;
            b.u115V5U44 = u16le(13) * K;
            b.u115V5A42 = u16le(15) * K;
            break;
        case 0x01:
            b.u115V5E15 = u16le(5)  * K;
            b.u36VPWR   = u16le(7)  * K;
            b.freqPWR   = u16le(9);       // no scaling
            b.i26VPWR   = u16le(11) * K;
            b.i26VML    = u16le(13) * K;
            b.i26V5U44  = u16le(15) * K;
            break;
        case 0x02:
            b.i26V5A42  = u16le(5)  * K;
            b.i26VMF    = u16le(7)  * K;
            b.i115V5U44 = u16le(9)  * K;
            b.i115V5E15 = u16le(11) * K;
            b.i36VAPWR  = u16le(13) * K;
            b.i36VBPWR  = u16le(15) * K;
            break;
        case 0x03:
            b.thKhBkkn  = u16le(5)  * K;
            b.thKhPn    = u16le(7)  * K;
            b.th26VMfss = u16le(9)  * K;
            b.thChTltp  = u16le(11) * K;
            b.thFbCb    = u16le(13) * K;
            b.thThaoHam = u16le(15) * K;
            break;
        case 0x04:
            b.thK3    = u16le(5)  * K;
            b.thKhDcp = u16le(7)  * K;
            b.thCo    = u16le(9)  * K;
            b.thCcy   = u16le(11) * K;
            b.thSsp   = u16le(13) * K;
            b.thDp    = u16le(15) * K;
            break;
        default: break;
    }
    return b;
}

// ============================================================
// 5U44 parser
// ============================================================
Data5U44 PacketParser::parse5U44(const QByteArray& d, uint32_t index, const QString& client) {
    Data5U44 u;
    u.client = client;
    u.index  = index;

    uint8_t io  = (uint8_t)d[IDX_5U44_IO];
    uint8_t dsp = (uint8_t)d[IDX_5U44_DSP];

    u.soiDot      = bit(io, 7);
    u.aNode       = bit(io, 6);
    u.tachTangVao = bit(io, 5);
    u.tachTangRa  = bit(io, 4);
    u.ranh        = bit(io, 3);
    u.chuyenDoDoc = bit(io, 2);
    u.k3          = bit(io, 1);
    u.k6          = bit(io, 0);

    u.k7     = bit(dsp, 5);
    u.rxLock = bit(dsp, 4);
    u.txLock = bit(dsp, 3);

    // Phach: bits [2:0] hoán vị: p0=(bit0<<2), p1=bit1, p2=(bit2>>2)
    int p0 = ((dsp & 0x1) << 2);
    int p1 =  (dsp & 0x2);
    int p2 = ((dsp & 0x4) >> 2);
    u.phach = (p0 | p1 | p2) & 0x7;

    u.suyHao   = (uint8_t)d[IDX_5U44_ANTENA];
    u.congSuat = i16BE(d, IDX_5U44_RSSI);
    u.xungHoi  = (uint8_t)d[IDX_5U44_XUNGHOI];
    u.k1 = i16BE(d, IDX_5U44_K1) / 1000.0;
    u.k2 = i16BE(d, IDX_5U44_K2) / 1000.0;

    uint8_t st0 = (uint8_t)d[IDX_5U44_STATUS];
    uint8_t st1 = (uint8_t)d[IDX_5U44_STATUS+1];

    u.ad9643  = bit(st0, 3);
    u.ad9523  = bit(st0, 2);
    u.adf4360 = bit(st0, 1);
    u.adf5355 = bit(st0, 0);

    u.voltage26    = bit(st1, 5);
    u.currentTxRx  = bit(st1, 4);
    u.current5VFPGA= bit(st1, 3);
    u.voltage40    = bit(st1, 2);
    u.voltage575   = bit(st1, 1);
    u.connection5A = bit(st1, 0);

    u.sIndex = u16BE(d, IDX_5U44_SIDX);
    return u;
}

// ============================================================
// 5E15-VT parser (184 byte multicast packet)
// ============================================================
Data5E15 PacketParser::parse5E15VT(const QByteArray& d) {
    Data5E15 e;
    if (d.size() < 33) return e;

    e.nhietDoDsp = (double)((((uint32_t)(uint8_t)d[6] & 0x0F) << 8) |
                             (((uint32_t)(uint8_t)d[7] & 0xFC) >> 2)) * 0.0625;

    e.dienAp150V = (double)((((uint32_t)(uint8_t)d[7] & 0x03) << 9) |
                             (((uint32_t)(uint8_t)d[8] & 0xFF) << 1) |
                             (((uint32_t)(uint8_t)d[9] & 0x80) >> 7)) * 0.2082;

    e.detonated   = bit((uint8_t)d[27], 7);
    e.k3          = bit((uint8_t)d[27], 5);
    e.nlc         = bit((uint8_t)d[27], 1);
    e.soXungPhatXa= u24BE(d, 30);
    return e;
}

// ============================================================
// 5E15-FM parser (alternative format)
// ============================================================
Data5E15 PacketParser::parse5E15FM(const QByteArray& d) {
    Data5E15 e;
    if (d.size() < 172) return e;

    uint8_t type = (uint8_t)d[12];
    if (type == 0x01) {
        // FM format stores multi-byte values little-endian:
        // e.g. _data[117,116,115,114] → highest addr = MSB → u32LE from addr 114
        e.dienAp150V   = u32LE(d, 114) / 100000.0;  // [114]=LSB .. [117]=MSB
        e.nhietDoDsp   = u16LE(d, 170) / 1000.0;    // [170]=LSB,  [171]=MSB
        e.soXungPhatXa = u32LE(d, 162);              // [162]=LSB .. [165]=MSB
        e.k3  = bit((uint8_t)d[17], 5);
        e.nlc = bit((uint8_t)d[17], 2);
    }
    e.detonated = (type == 2);
    return e;
}

} // namespace GDT
