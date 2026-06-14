#include "CsvLogger.h"
#include <QStandardPaths>
#include <QDir>

namespace GDT {

static const char* HDR_TELE =
    "client, index, 5i_adc26, 5i_adc36, 5i_adc115, 5i_freq, "
    "tele_x_gyro, tele_y_gyro, tele_z_gyro, tele_x_accl, tele_y_accl, tele_z_accl, "
    "5a_status, 5u_status";

// Column order matches C# ServerTelemetry CSV_FORM_HEADER_5A42VT exactly (50 columns)
static const char* HDR_5A42 =
    "client, index, 5a_adc26, 5a_ml1, 5a_ml2, 5a_ml3, 5a_setpoint1, 5a_setpoint2, 5a_setpoint3, 5a_k1, 5a_k2, "
    "5a_duc1, 5a_duc2, 5a_duc3, 5a_duk, 5a_omega1, 5a_omega2, 5a_omega3, "
    "5a_kerrnel_gama, 5a_ufors_gama, 5a_kerrnel_offset, 5a_ufors_offset, "
    "5a_BFlashConfig, 5a_BADCSetpoint, 5a_BADCFeedback, 5a_BRatioIntegral, "
    "5a_BGamaEnable, 5a_B5u_digital, 5a_BKernelSensor, 5a_BuForsSensor, "
    "5a_BOmega1, 5a_BOmega2, 5a_BOmega3, 5a_BGama, 5a_BVoltage26, "
    "5a_BCtrl_ml1, 5a_BCtrl_ml2, 5a_BCtrl_ml3, BInternal_Sensor_Communication, "
    "BInternal_Sensor_Configuration, BX_Angular_Rate, BY_Angular_Rate, BZ_Angular_Rate, "
    "BTemperature, BError_In, BTemperature_Warning, BAurixiliary_Control_Loop_Error, "
    "BHardware_Bit_Error, BMeasurenment_Range_Exceeded, 5a_index";

static const char* HDR_5U44 =
    "client, index, 5u_soidot, 5u_anode, 5u_tachtangvao, 5u_tachtangra, 5u_ranh, "
    "5u_chuyendodoc, 5u_k3, 5u_k6, 5u_k7, 5u_rx_lock, 5u_tx_lock, 5u_phach, "
    "5u_suyhao, 5u_congsuat, 5u_xunghoi, 5u_k1, 5u_k2, "
    "5u_ad9643, 5u_ad9523, 5u_adf4360, 5u_adf5355, "
    "5u_voltage26, 5u_currentTxRx, 5u_current5VFpga, 5u_voltage40, 5u_voltage575, "
    "5u_connection5a, 5u_index";

// Column order matches C# CSV_FORM_HEADER_5I41VT (34 columns)
static const char* HDR_5I41 =
    "client, index, GroupId, "
    "U26VPWR, U26VML, U26V5U44, U26V5A42, U115V5U44, U115V5A42, "
    "U115V5E15, U36VPWR, FreqPWR, I26VPWR, I26VML, I26V5U44, "
    "I26V5A42, I26VMF, I115V5U44, I115V5E15, I36VAPWR, I36VBPWR, "
    "TH_KH_BKKN, TH_KH_PN, TH_26V_MFSS, TH_CH_TLTP, TH_FB_CB, TH_ThaoHam, "
    "TH_K3, TH_KH_DCP, TH_CO, TH_CCY, TH_SSP, TH_DP, SampleTime";

// 165-column header matching C# CSV_DATA_HEADER_5E15VT exactly
// Only 6 columns carry real data; the rest are 0 (C# model has same 6 parsed fields)
static const char* HDR_5E15 =
    "client, time, index, nhietdoRx, nhietdoDsp, dinap150vNguon, dong5vNguon, dienap5vNguon, dienap150vTu, dong32vTx, "
    "dienap32vTx, dong5v5Tx, dienap5v5Tx, dong5v5Rx, dienap5v5Rx, dong32vDsp, dienap32vDsp, dienap3v3Nguon, dong3v3Nguon, rxchannel, lch, chophepkichno, "
    "detonated, detonate, k3, release, k6, nlc, power_on, rom_crc, lock_rx, lock_tx, "
    "rx_trx_sw, dorongxungtichluy, sochukytichluytoida, soxungphatxaTxduCS, soxungphatxaTxkhongduCS, sochukycoxungthuRx, biendodinhthuRxtucthoi, biendodinhthuRxtoida, giatringuongmangconRxtucthoi, gaitringuongmangconRxtoida, "
    "giatringuongmangtongRxtucthoi, giatringuongmangtongRxtoida, nenmaythuRxnhonhat, khoangcachmuctieu, IF_X_0, IF_X_2, IF_X_4, IF_X_6, IF_X_8, IF_X_10, "
    "IF_X_12, IF_X_14, IF_X_16, IF_X_18, IF_X2_0, IF_X2_2, IF_X2_4, IF_X2_6, IF_X2_8, IF_X2_10, "
    "IF_X2_12, IF_X2_14, IF_X2_16, IF_X2_18, IF_X4_0, IF_X4_2, IF_X4_4, IF_X4_6, IF_X4_8, IF_X4_10, "
    "IF_X4_12, IF_X4_14, IF_X4_16, IF_X4_18, IF_X6_0, IF_X6_2, IF_X6_4, IF_X6_6, IF_X6_8, IF_X6_10, "
    "IF_X6_12, IF_X6_14, IF_X6_16, IF_X6_18, IF_X8_0, IF_X8_2, IF_X8_4, IF_X8_6, IF_X8_8, IF_X8_10, "
    "IF_X8_12, IF_X8_14, IF_X8_16, IF_X8_18, IF_Y_0, IF_Y_2, IF_Y_4, IF_Y_6, IF_Y_8, IF_Y_10, "
    "IF_Y_12, IF_Y_14, IF_Y_16, IF_Y_18, IF_Y2_0, IF_Y2_2, IF_Y2_4, IF_Y2_6, IF_Y2_8, IF_Y2_10, "
    "IF_Y2_12, IF_Y2_14, IF_Y2_16, IF_Y2_18, IF_Y4_0, IF_Y4_2, IF_Y4_4, IF_Y4_6, IF_Y4_8, IF_Y4_10, "
    "IF_Y4_12, IF_Y4_14, IF_Y4_16, IF_Y4_18, IF_Y6_0, IF_Y6_2, IF_Y6_4, IF_Y6_6, IF_Y6_8, IF_Y6_10, "
    "IF_Y6_12, IF_Y6_14, IF_Y6_16, IF_Y6_18, IF_Y8_0, IF_Y8_2, IF_Y8_4, IF_Y8_6, IF_Y8_8, IF_Y8_10, "
    "IF_Y8_12, IF_Y8_14, IF_Y8_16, IF_Y8_18, tichluy0, tichluy1, tichluy2, tichluy3, tichluy4, tichluy5, "
    "tichluy6, tichluy7, tichluy8, tichluy9, tichluy10, tichluy11, tichluy12, tichluy13, tichluy14, tichluy15, "
    "tichluy16, tichluy17, crc";

CsvLogger::CsvLogger(QObject* parent) : QObject(parent) {
    m_flushTimer = new QTimer(this);
    m_flushTimer->setInterval(5000);
    connect(m_flushTimer, &QTimer::timeout, this, &CsvLogger::flushAll);
}

CsvLogger::~CsvLogger() { close(); }

QString CsvLogger::logDir() {
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/DataLog5V27VT";
}

bool CsvLogger::openFile(QFile& f, QTextStream& s, const QString& path, const QString& header) {
    f.setFileName(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    s.setDevice(&f);
    s.setEncoding(QStringConverter::Utf8);
    s << header << "\n";
    return true;
}

bool CsvLogger::open(const QString& flightName) {
    QString dir = logDir();
    QDir().mkpath(dir);
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

    bool ok = true;
    ok &= openFile(m_fileTele, m_stTele,
                   dir + QString("/%1_5V27VT_Telemetry_%2.csv").arg(flightName, ts), HDR_TELE);
    ok &= openFile(m_file5A42, m_st5A42,
                   dir + QString("/%1_5V27VT_5A42VT_%2.csv").arg(flightName, ts), HDR_5A42);
    ok &= openFile(m_file5U44, m_st5U44,
                   dir + QString("/%1_5V27VT_5U44VT_%2.csv").arg(flightName, ts), HDR_5U44);
    ok &= openFile(m_file5I41, m_st5I41,
                   dir + QString("/%1_5V27VT_5I41VT_%2.csv").arg(flightName, ts), HDR_5I41);
    ok &= openFile(m_file5E15, m_st5E15,
                   dir + QString("/%1_5V27VT_5E15VT_%2.text").arg(flightName, ts), HDR_5E15);
    m_open = ok;
    if (ok) m_flushTimer->start();
    return ok;
}

void CsvLogger::flushAll() {
    if (!m_open) return;
    m_stTele.flush();
    m_st5A42.flush();
    m_st5U44.flush();
    m_st5I41.flush();
    m_st5E15.flush();
}

void CsvLogger::close() {
    if (!m_open) return;
    m_flushTimer->stop();
    m_stTele.flush(); m_fileTele.close();
    m_st5A42.flush(); m_file5A42.close();
    m_st5U44.flush(); m_file5U44.close();
    m_st5I41.flush(); m_file5I41.close();
    m_st5E15.flush(); m_file5E15.close();
    m_open = false;
}

bool CsvLogger::isOpen() const { return m_open; }

void CsvLogger::writeTelemetry(const DataTelemetry& d) {
    if (!m_open) return;
    m_stTele << d.client << ", " << d.index << ", "
             << d.infor5I41.adc26 << ", " << d.infor5I41.adc36 << ", "
             << d.infor5I41.adc115 << ", " << d.infor5I41.freq << ", "
             << d.xGyro << ", " << d.yGyro << ", " << d.zGyro << ", "
             << d.xAccl << ", " << d.yAccl << ", " << d.zAccl << ", "
             << (int)d.status5A << ", " << (int)d.status5U << "\n";
}

// Column order matches C# Infor5A42BlockModel.ToString():
// adc26, ml1-3, sp1-3, k1-2, duc1-3, duk, omega1-3, kernelGama, uforsGama, kernelOffset, uforsOffset,
// then status-byte1 group (systemInit..uforsSensor), then status-byte0 group (bOmega1..bCtrlML3),
// then kernel+ufors status bytes (11 extra fields), then sIndex
void CsvLogger::write5A42(const Data5A42& a) {
    if (!m_open) return;
    m_st5A42 << a.client << ", " << a.index << ", "
             << a.adc26 << ", "
             << a.ml1   << ", " << a.ml2   << ", " << a.ml3   << ", "
             << a.sp1   << ", " << a.sp2   << ", " << a.sp3   << ", "
             << a.k1    << ", " << a.k2    << ", "
             << a.duc1  << ", " << a.duc2  << ", " << a.duc3  << ", " << a.duk << ", "
             << a.omega1 << ", " << a.omega2 << ", " << a.omega3 << ", "
             << a.kernelGama << ", " << a.uforsGama << ", "
             << a.kernelOffset << ", " << a.uforsOffset << ", "
             // Status byte 1 → C# BFlashConfig group (systemInit = BFlashConfig in C#)
             << (int)a.systemInit    << ", " << (int)a.adcSP       << ", " << (int)a.adcFB << ", "
             << (int)a.ratioIntegral << ", " << (int)a.gamaEnable  << ", "
             << (int)a.b5uDigital   << ", " << (int)a.kernelSensor << ", " << (int)a.uforsSensor << ", "
             // Status byte 0 → C# BOmega group
             << (int)a.bOmega1 << ", " << (int)a.bOmega2 << ", " << (int)a.bOmega3 << ", "
             << (int)a.bGama   << ", " << (int)a.bADC26  << ", "
             << (int)a.bCtrlML1 << ", " << (int)a.bCtrlML2 << ", " << (int)a.bCtrlML3 << ", "
             // Kernel status byte [offset 46] — 6 bits
             << (int)a.sensorComm   << ", " << (int)a.sensorConfig << ", "
             << (int)a.xRate        << ", " << (int)a.yRate        << ", " << (int)a.zRate << ", "
             << (int)a.tempOk       << ", "
             // Ufors status byte [offset 47] — 5 bits
             << (int)a.errorIn          << ", " << (int)a.tempWarn        << ", "
             << (int)a.controlLoopError << ", " << (int)a.hardwareError   << ", "
             << (int)a.rangeExceed      << ", "
             << a.sIndex << "\n";
}

void CsvLogger::write5U44(const Data5U44& u) {
    if (!m_open) return;
    m_st5U44 << u.client << ", " << u.index << ", "
             << u.soiDot << ", " << u.aNode << ", "
             << u.tachTangVao << ", " << u.tachTangRa << ", "
             << u.ranh << ", " << u.chuyenDoDoc << ", "
             << u.k3 << ", " << u.k6 << ", " << u.k7 << ", "
             << u.rxLock << ", " << u.txLock << ", " << u.phach << ", "
             << u.suyHao << ", " << u.congSuat << ", " << u.xungHoi << ", "
             << u.k1 << ", " << u.k2 << ", "
             << u.ad9643 << ", " << u.ad9523 << ", " << u.adf4360 << ", " << u.adf5355 << ", "
             << u.voltage26 << ", " << u.currentTxRx << ", " << u.current5VFPGA << ", "
             << u.voltage40 << ", " << u.voltage575 << ", " << u.connection5A << ", "
             << u.sIndex << "\n";
}

// One row per received packet; groupId indicates which 6 measurements are valid this packet
void CsvLogger::write5I41(const Data5I41Block& d) {
    if (!m_open) return;
    m_st5I41 << d.client << ", " << d.index << ", " << (int)d.groupId << ", "
              << d.u26VPWR   << ", " << d.u26VML    << ", " << d.u26V5U44  << ", " << d.u26V5A42  << ", "
              << d.u115V5U44 << ", " << d.u115V5A42 << ", "
              << d.u115V5E15 << ", " << d.u36VPWR   << ", " << d.freqPWR   << ", "
              << d.i26VPWR   << ", " << d.i26VML    << ", " << d.i26V5U44  << ", "
              << d.i26V5A42  << ", " << d.i26VMF    << ", " << d.i115V5U44 << ", " << d.i115V5E15 << ", "
              << d.i36VAPWR  << ", " << d.i36VBPWR  << ", "
              << d.thKhBkkn  << ", " << d.thKhPn    << ", " << d.th26VMfss << ", "
              << d.thChTltp  << ", " << d.thFbCb    << ", " << d.thThaoHam << ", "
              << d.thK3      << ", " << d.thKhDcp   << ", " << d.thCo      << ", "
              << d.thCcy     << ", " << d.thSsp     << ", " << d.thDp      << ", "
              << (int)d.sampleTime << "\n";
}

void CsvLogger::write5E15(const Data5E15& e, const QString& client, uint32_t index) {
    if (!m_open) return;
    // 165 columns = client + time + index + 162 model fields (positions [0]..[161])
    // Real data positions: [1]=nhietdoDsp  [2]=dinap150vNguon
    //   [19]=detonated  [21]=k3  [24]=nlc  [32]=soxungphatxaTxduCS
    // All other positions are 0 — matches C# Infor5E15BlockModel which also only populates 6 fields.
    m_st5E15
        << client << ", "
        << QDateTime::currentDateTime().toString("HH:mm:ss.zzz") << ", "
        << index << ", "
        // model[0] nhietdoRx
        << 0 << ", "
        // model[1] nhietdoDsp
        << e.nhietDoDsp << ", "
        // model[2] dinap150vNguon
        << e.dienAp150V << ", "
        // model[3..18] — dong5vNguon … chophepkichno (16 zeros)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
        // model[19] detonated
        << (int)e.detonated << ", "
        // model[20] detonate
        << 0 << ", "
        // model[21] k3
        << (int)e.k3 << ", "
        // model[22..23] release, k6
        << "0, 0, "
        // model[24] nlc
        << (int)e.nlc << ", "
        // model[25..31] — power_on … sochukytichluytoida (7 zeros)
        << "0, 0, 0, 0, 0, 0, 0, "
        // model[32] soxungphatxaTxduCS
        << e.soXungPhatXa << ", "
        // model[33..161] — soxungphatxaTxkhongduCS … crc (129 zeros)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 33-42
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 43-52  (IF_X_0..18)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 53-62  (IF_X2)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 63-72  (IF_X4)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 73-82  (IF_X6)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 83-92  (IF_X8)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 93-102 (IF_Y)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 103-112 (IF_Y2)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 113-122 (IF_Y4)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 123-132 (IF_Y6)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 133-142 (IF_Y8)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "  // 143-152 (tichluy0-9)
        << "0, 0, 0, 0, 0, 0, 0, 0, 0"        // 153-161 (tichluy10-17, crc)
        << "\n";
}

} // namespace GDT
