#include "CsvLogger.h"
#include <QStandardPaths>
#include <QDir>

namespace GDT {

static const char* HDR_TELE =
    "client, index, 5i_adc26, 5i_adc36, 5i_adc115, 5i_freq, "
    "tele_x_gyro, tele_y_gyro, tele_z_gyro, tele_x_accl, tele_y_accl, tele_z_accl, "
    "5a_status, 5u_status";

static const char* HDR_5A42 =
    "client, index, 5a_k1, 5a_k2, 5a_adc26, 5a_ml1, 5a_ml2, 5a_ml3, "
    "5a_duc1, 5a_duc2, 5a_duc3, 5a_duk, 5a_sp1, 5a_sp2, 5a_sp3, "
    "5a_omega1, 5a_omega2, 5a_omega3, "
    "5a_kernelGama, 5a_uforsGama, 5a_kernelOffset, 5a_uforsOffset, "
    "5a_bOmega1, 5a_bOmega2, 5a_bOmega3, 5a_bGama, 5a_bADC26, 5a_bCtrlML1, 5a_bCtrlML2, 5a_bCtrlML3, "
    "5a_systemInit, 5a_adcSP, 5a_adcFB, 5a_ratioIntegral, 5a_gamaEnable, 5a_b5uDigital, 5a_kernelSensor, 5a_uforsSensor, "
    "5a_index";

static const char* HDR_5U44 =
    "client, index, 5u_soidot, 5u_anode, 5u_tachtangvao, 5u_tachtangra, 5u_ranh, "
    "5u_chuyendodoc, 5u_k3, 5u_k6, 5u_k7, 5u_rx_lock, 5u_tx_lock, 5u_phach, "
    "5u_suyhao, 5u_congsuat, 5u_xunghoi, 5u_k1, 5u_k2, "
    "5u_ad9643, 5u_ad9523, 5u_adf4360, 5u_adf5355, "
    "5u_voltage26, 5u_currentTxRx, 5u_current5VFpga, 5u_voltage40, 5u_voltage575, "
    "5u_connection5a, 5u_index";

static const char* HDR_5E15 =
    "client, time, index, nhietdoDsp, dienap150v, soXungPhatXa, detonated, k3, nlc";

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
    m_st5E15.flush();
}

void CsvLogger::close() {
    if (!m_open) return;
    m_flushTimer->stop();
    m_stTele.flush(); m_fileTele.close();
    m_st5A42.flush(); m_file5A42.close();
    m_st5U44.flush(); m_file5U44.close();
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

void CsvLogger::write5A42(const Data5A42& a) {
    if (!m_open) return;
    m_st5A42 << a.client << ", " << a.index << ", "
             << a.k1 << ", " << a.k2 << ", " << a.adc26 << ", "
             << a.ml1 << ", " << a.ml2 << ", " << a.ml3 << ", "
             << a.duc1 << ", " << a.duc2 << ", " << a.duc3 << ", " << a.duk << ", "
             << a.sp1 << ", " << a.sp2 << ", " << a.sp3 << ", "
             << a.omega1 << ", " << a.omega2 << ", " << a.omega3 << ", "
             << a.kernelGama << ", " << a.uforsGama << ", "
             << a.kernelOffset << ", " << a.uforsOffset << ", "
             << a.bOmega1 << ", " << a.bOmega2 << ", " << a.bOmega3 << ", "
             << a.bGama << ", " << a.bADC26 << ", "
             << a.bCtrlML1 << ", " << a.bCtrlML2 << ", " << a.bCtrlML3 << ", "
             << a.systemInit << ", " << a.adcSP << ", " << a.adcFB << ", "
             << a.ratioIntegral << ", " << a.gamaEnable << ", "
             << a.b5uDigital << ", " << a.kernelSensor << ", " << a.uforsSensor << ", "
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

void CsvLogger::write5E15(const Data5E15& e, const QString& client, uint32_t index) {
    if (!m_open) return;
    m_st5E15 << client << ", "
             << QDateTime::currentDateTime().toString("HH:mm:ss.zzz") << ", "
             << index << ", "
             << e.nhietDoDsp << ", " << e.dienAp150V << ", "
             << e.soXungPhatXa << ", "
             << e.detonated << ", " << e.k3 << ", " << e.nlc << "\n";
}

} // namespace GDT
