#pragma once
#include <QByteArray>
#include <QVector>
#include "models/TelemetryData.h"
#include "models/AppConfig.h"

namespace GDT {

class PacketParser {
public:
    explicit PacketParser(const Calib5A42& calib = Calib5A42{});
    void setCalib(const Calib5A42& calib);

    // Parse a raw payload byte array (after stripping TCP wrapper)
    DataTelemetry  parseTelemetry(const QByteArray& data, uint32_t index, const QString& client);
    Data5A42       parse5A42    (const QByteArray& data, uint32_t index, const QString& client);
    Data5U44       parse5U44    (const QByteArray& data, uint32_t index, const QString& client);
    Data5I41Block  parse5I41    (const QByteArray& data, uint32_t index, const QString& client);
    Data5E15       parse5E15VT  (const QByteArray& data);
    Data5E15       parse5E15FM  (const QByteArray& data);

    // CRC validation
    bool checkCrc8  (const QByteArray& data) const;
    bool checkCrc16_1021(const QByteArray& data) const;
    bool checkCrc16_8005(const QByteArray& data) const;
    bool checkCrc32 (const QByteArray& data) const;

    // Strip TCP wrapper and return (type, payload)
    // Returns type=0 if frame is invalid
    struct TcpFrame { uint8_t type = 0; QByteArray payload; };
    TcpFrame stripTcpWrapper(const QByteArray& raw) const;

private:
    Calib5A42 m_calib;

    // Byte-level helpers
    static uint16_t u16BE(const QByteArray& d, int idx);
    static int16_t  i16BE(const QByteArray& d, int idx);
    static uint32_t u24BE(const QByteArray& d, int idx);
    static uint32_t u32BE(const QByteArray& d, int idx);
    // Little-endian readers (used by 5E15-FM format)
    static uint16_t u16LE(const QByteArray& d, int idx);  // d[idx]=LSB, d[idx+1]=MSB
    static uint32_t u32LE(const QByteArray& d, int idx);  // d[idx]=LSB ... d[idx+3]=MSB
    static bool     bit(uint8_t byte, int pos);

    // ADC calibration formula
    double calAdc5A42(double val, double minV, double maxV, int mul, int plus, double adc26Raw) const;
};

} // namespace GDT
