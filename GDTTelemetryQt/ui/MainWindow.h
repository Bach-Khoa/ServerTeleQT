#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QDialog>
#include "models/AppConfig.h"
#include "network/TelemetryServer.h"
#include "network/MulticastReceiver.h"
#include "storage/DatabaseManager.h"
#include "storage/CsvLogger.h"

namespace GDT {

class Panel5A42;
class Panel5U44;
class Panel5E15;
class Panel5I41;
class PanelTelemetry;
class MultiChartWidget;
class ErrorRatePanel;
class Detail5A42Dialog;

// Per-packet-type error rate tracker (20-bit sequence index).
// Formula matches C# server: errRate = (1 - received / span) * 100
// where span = lastIdx - firstIdx + 1.
// Duplicates and backward jumps (within 2048) are ignored to avoid
// false gap explosions when packets arrive out-of-order or from
// a reconnecting client that resets its counter.
struct PktStats {
    static constexpr uint32_t NONE = 0xFFFFFFFF;
    uint32_t firstIdx = NONE;
    uint32_t lastIdx  = NONE;
    uint64_t received = 0;

    void update(uint32_t idx) {
        if (lastIdx != NONE) {
            // Forward distance in 20-bit ring
            uint32_t fwd = (idx - lastIdx) & 0xFFFFF;
            // fwd == 0          → exact duplicate
            // fwd > 0xFF800     → backward jump (within ~2048) → late/dup
            if (fwd == 0 || fwd > (0x100000 - 2048)) return;
        }
        if (firstIdx == NONE) firstIdx = idx;
        lastIdx = idx;
        ++received;
    }

    double errorRate() const {
        if (received < 2 || firstIdx == NONE) return 0.0;
        uint32_t span = (lastIdx - firstIdx + 1) & 0xFFFFF;
        if (span == 0) return 0.0;
        double rate = (1.0 - received / static_cast<double>(span)) * 100.0;
        return rate < 0.0 ? 0.0 : rate;  // clamp: dedup can make received > span
    }

    void reset() { firstIdx = lastIdx = NONE; received = 0; }
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onStartServer();
    void onStopServer();
    void onResetServer();
    void onStartLog();
    void onStopLog();
    void onSetupServer();
    void onSetup5A42();
    void onReset();
    void onShow5A42Detail();
    void onToggleErrorRatePanel();
    void promptAndStartLog();       // ask flight name → close old log → open new log

    // Data receive slots — lightweight: store + log only, NO panel/chart update
    void onTelemetryReceived(GDT::DataTelemetry d);
    void on5A42Received(GDT::Data5A42 d);
    void on5U44Received(GDT::Data5U44 d);
    void on5I41Received(GDT::Data5I41Block d);
    void on5E15Received(GDT::Data5E15 d);
    void onClientConnected(QString addr);
    void onClientDisconnected(QString addr);

    // Rate-limited UI refresh at ~30 Hz
    void onUiTimer();

private:
    void buildUi();
    void buildMenuBar();
    void updateStatusBar(const QString& msg);
    void loadConfig();
    void saveConfig();

    AppConfig           m_config;
    TelemetryServer*    m_server;
    MulticastReceiver*  m_mcast;
    DatabaseManager     m_db;
    CsvLogger           m_logger;

    // Panels
    Panel5A42*        m_panel5A42;
    Panel5U44*        m_panel5U44;
    Panel5E15*        m_panel5E15;
    Panel5I41*        m_panel5I41;
    PanelTelemetry*   m_panelTele;
    MultiChartWidget* m_multiChart;
    ErrorRatePanel*   m_errorRatePanel;

    // Detail windows (non-modal, lazily created)
    Detail5A42Dialog* m_detail5A42    = nullptr;
    QDialog*          m_errRateDlg    = nullptr;

    // Full-panel dialogs (Qt::Tool, shown on demand from compact sidebar)
    QDialog* m_dlgTele = nullptr;
    QDialog* m_dlg5A42 = nullptr;
    QDialog* m_dlg5I41 = nullptr;
    QDialog* m_dlg5U44 = nullptr;
    QDialog* m_dlg5E15 = nullptr;

    // Compact sidebar key-value labels
    QLabel *m_cTeleIdx  = nullptr, *m_cTeleSt5A = nullptr, *m_cTeleSt5U = nullptr;
    QLabel *m_c5A42K1   = nullptr, *m_c5A42K2   = nullptr;
    QLabel *m_c5A42Duk  = nullptr, *m_c5A42Adc  = nullptr;
    QLabel *m_c5I41Adc26= nullptr, *m_c5I41Adc115 = nullptr, *m_c5I41Freq = nullptr;
    QLabel *m_c5U44K1   = nullptr, *m_c5U44K2   = nullptr, *m_c5U44Xung  = nullptr;
    QLabel *m_c5E15Volt = nullptr, *m_c5E15Temp = nullptr;

    // Status bar labels
    QLabel* m_statusLabel;
    QLabel* m_connLabel;

    // Error rate info bar labels
    QLabel* m_errTele;
    QLabel* m_err5A42;
    QLabel* m_err5U44;
    QLabel* m_err5I41;
    QLabel* m_err5E15;

    // Rate limiting: latest received data per type
    GDT::DataTelemetry  m_latestTele;
    GDT::Data5A42       m_latest5A42;
    GDT::Data5U44       m_latest5U44;
    GDT::Data5I41Block  m_latest5I41;
    GDT::Data5E15       m_latest5E15;
    bool m_hasTele = false;
    bool m_has5A42 = false;
    bool m_has5U44 = false;
    bool m_has5I41 = false;
    bool m_has5E15 = false;

    // Error rate stats
    PktStats m_statsTele;
    PktStats m_stats5A42;
    PktStats m_stats5U44;
    PktStats m_stats5I41;
    PktStats m_stats5E15;

    QTimer*  m_uiTimer;
    uint32_t m_idx5E15     = 0;
    int      m_clientCount = 0;
};

} // namespace GDT
