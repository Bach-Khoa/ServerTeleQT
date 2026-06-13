#include "MainWindow.h"
#include "Panel5A42.h"
#include "Panel5U44.h"
#include "Panel5E15.h"
#include "Panel5I41.h"
#include "PanelTelemetry.h"
#include "ChartPanel.h"
#include "ErrorRatePanel.h"
#include "Detail5A42Dialog.h"
#include "SetupServerDialog.h"
#include "Setup5A42Dialog.h"
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QStatusBar>
#include <QScrollArea>
#include <QGridLayout>
#include <QWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFrame>

namespace GDT {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("GDT Telemetry TLDK35 - Qt C++");
    setMinimumSize(1280, 800);

    m_server = new TelemetryServer(this);
    m_mcast  = new MulticastReceiver(this);

    connect(m_server, &TelemetryServer::telemetryReceived, this, &MainWindow::onTelemetryReceived, Qt::QueuedConnection);
    connect(m_server, &TelemetryServer::data5A42Received,  this, &MainWindow::on5A42Received,      Qt::QueuedConnection);
    connect(m_server, &TelemetryServer::data5U44Received,  this, &MainWindow::on5U44Received,      Qt::QueuedConnection);
    connect(m_server, &TelemetryServer::data5I41Received,  this, &MainWindow::on5I41Received,      Qt::QueuedConnection);
    connect(m_server, &TelemetryServer::clientConnected,   this, &MainWindow::onClientConnected,   Qt::QueuedConnection);
    connect(m_server, &TelemetryServer::clientDisconnected,this, &MainWindow::onClientDisconnected,Qt::QueuedConnection);
    connect(m_mcast,  &MulticastReceiver::data5E15Received,this, &MainWindow::on5E15Received,      Qt::QueuedConnection);

    loadConfig();
    buildUi();
    buildMenuBar();

    // Connect panel button (must be after buildUi which creates m_panel5A42)
    connect(m_panel5A42, &Panel5A42::detailRequested, this, &MainWindow::onShow5A42Detail);

    // Rate-limit UI refresh to ~30 Hz
    m_uiTimer = new QTimer(this);
    connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::onUiTimer);
    m_uiTimer->start(33);
}

MainWindow::~MainWindow() {
    m_server->stop();
    m_mcast->stop();
    m_logger.close();
    saveConfig();
}

void MainWindow::buildUi() {
    m_panel5I41      = new Panel5I41;
    m_panel5A42      = new Panel5A42;
    m_panel5U44      = new Panel5U44;
    m_panel5E15      = new Panel5E15;
    m_panelTele      = new PanelTelemetry;
    m_chart          = new ChartPanel;
    m_errorRatePanel = new ErrorRatePanel;

    // Left: instrument panels in a scroll area
    auto* instrWidget = new QWidget;
    auto* grid = new QGridLayout(instrWidget);
    grid->setSpacing(4);
    grid->setContentsMargins(4, 4, 4, 4);
    grid->addWidget(m_panelTele,  0, 0);
    grid->addWidget(m_panel5I41,  1, 0);
    grid->addWidget(m_panel5A42,  0, 1, 2, 1);
    grid->addWidget(m_panel5U44,  0, 2, 2, 1);
    grid->addWidget(m_panel5E15,  0, 3, 2, 1);
    grid->setRowStretch(2, 1);

    auto* scroll = new QScrollArea;
    scroll->setWidget(instrWidget);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    // Right: live chart (top) + error rate chart (bottom)
    auto* rightWidget = new QWidget;
    auto* rightVl = new QVBoxLayout(rightWidget);
    rightVl->setContentsMargins(0, 0, 0, 0);
    rightVl->setSpacing(2);
    rightVl->addWidget(m_chart, 3);
    rightVl->addWidget(m_errorRatePanel, 1);

    // Horizontal splitter: panels left | charts right
    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(scroll);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({420, 860});

    // Info bar: error rate labels
    auto* infoBar = new QWidget;
    infoBar->setFixedHeight(28);
    infoBar->setStyleSheet("background:#ddeeff; border-bottom:1px solid #aac; font-size:9px;");
    auto* infoLayout = new QHBoxLayout(infoBar);
    infoLayout->setContentsMargins(8, 0, 8, 0);
    infoLayout->setSpacing(24);

    auto makeLbl = [&](QLabel*& lbl, const QString& init) {
        lbl = new QLabel(init);
        lbl->setStyleSheet("color:#003366;");
        infoLayout->addWidget(lbl);
    };

    // Client count — prominent at left
    m_connLabel = new QLabel("Clients: 0");
    m_connLabel->setStyleSheet(
        "color:#005500; font-weight:bold; font-size:10px;"
        "background:#ccffcc; border:1px solid #44aa44;"
        "border-radius:3px; padding:1px 6px;");
    infoLayout->addWidget(m_connLabel);

    // Separator
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Sunken);
    sep->setStyleSheet("color:#aac;");
    infoLayout->addWidget(sep);

    makeLbl(m_errTele, "Tỷ lệ lỗi Telemetry: --- %");
    makeLbl(m_err5A42, "Tỷ lệ lỗi 5A42VT: --- %");
    makeLbl(m_err5U44, "Tỷ lệ lỗi 5U44VT: --- %");
    makeLbl(m_err5I41, "Tỷ lệ lỗi 5I41VT: --- %");
    makeLbl(m_err5E15, "Tỷ lệ lỗi 5E15: --- %");
    infoLayout->addStretch();

    auto* central = new QWidget;
    auto* vl = new QVBoxLayout(central);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(0);
    vl->addWidget(infoBar);
    vl->addWidget(splitter);
    setCentralWidget(central);

    m_statusLabel = new QLabel("Chưa kết nối");
    statusBar()->addPermanentWidget(m_statusLabel);
}

void MainWindow::buildMenuBar() {
    auto* mServer = menuBar()->addMenu("Server");

    auto* actStart  = mServer->addAction("▶  Khởi động Server");
    auto* actStop   = mServer->addAction("■  Dừng Server");
    auto* actRstSrv = mServer->addAction("🔄  Reset Server");
    mServer->addSeparator();
    auto* actSetup = mServer->addAction("⚙  Cài đặt Server...");
    mServer->addSeparator();
    auto* actSetup5a42 = mServer->addAction("⚙  Cài đặt 5A42 ADC...");

    auto* mLog = menuBar()->addMenu("Ghi dữ liệu");
    auto* actStartLog = mLog->addAction("📂  Bắt đầu ghi CSV...");
    auto* actStopLog  = mLog->addAction("💾  Dừng ghi CSV");

    auto* mView = menuBar()->addMenu("Hiển thị");
    auto* actReset = mView->addAction("↺  Reset bộ đếm");

    connect(actStart,   &QAction::triggered, this, &MainWindow::onStartServer);
    connect(actStop,    &QAction::triggered, this, &MainWindow::onStopServer);
    connect(actRstSrv,  &QAction::triggered, this, &MainWindow::onResetServer);
    connect(actSetup,   &QAction::triggered, this, &MainWindow::onSetupServer);
    connect(actSetup5a42, &QAction::triggered, this, &MainWindow::onSetup5A42);
    connect(actStartLog,  &QAction::triggered, this, &MainWindow::onStartLog);
    connect(actStopLog,   &QAction::triggered, this, &MainWindow::onStopLog);
    connect(actReset,     &QAction::triggered, this, &MainWindow::onReset);

    // Toolbar for quick access to server controls
    auto* tb = addToolBar("Server");
    tb->setMovable(false);
    tb->setIconSize(QSize(16, 16));
    auto* tbStart  = tb->addAction("▶ Khởi động");
    auto* tbStop   = tb->addAction("■ Dừng");
    auto* tbReset  = tb->addAction("🔄 Reset Server");
    tbReset->setToolTip("Dừng server, reset bộ đếm và khởi động lại");

    connect(tbStart, &QAction::triggered, this, &MainWindow::onStartServer);
    connect(tbStop,  &QAction::triggered, this, &MainWindow::onStopServer);
    connect(tbReset, &QAction::triggered, this, &MainWindow::onResetServer);
}

// ============================================================
// Server control
// ============================================================
void MainWindow::onStartServer() {
    if (m_server->isRunning()) return;
    if (!m_server->start(m_config.server)) {
        updateStatusBar("Lỗi: Không thể khởi động server TCP!");
        return;
    }
    m_mcast->start(m_config.server.mcastIp, m_config.server.mcastPort);
    updateStatusBar(QString("Server đang chạy tại %1:%2")
                    .arg(m_config.server.serverIp).arg(m_config.server.serverPort));
}

void MainWindow::onStopServer() {
    m_server->stop();
    m_mcast->stop();
    // stop() deletes all workers before their clientDisconnected signals can fire,
    // so reset the count here manually.
    m_clientCount = 0;
    m_connLabel->setText("Clients: 0");
    updateStatusBar("Server đã dừng");
}

void MainWindow::onResetServer() {
    // Stop
    m_server->stop();
    m_mcast->stop();

    // Reset client count
    m_clientCount = 0;
    m_connLabel->setText("Clients: 0");

    // Reset all stats + charts (same as onReset)
    m_statsTele.reset();
    m_stats5A42.reset();
    m_stats5U44.reset();
    m_stats5I41.reset();
    m_stats5E15.reset();
    m_idx5E15 = 0;
    m_chart->reset();
    m_errorRatePanel->reset();
    if (m_detail5A42) m_detail5A42->reset();

    // Restart
    if (!m_server->start(m_config.server)) {
        updateStatusBar("Lỗi: Không thể khởi động lại server TCP!");
        return;
    }
    m_mcast->start(m_config.server.mcastIp, m_config.server.mcastPort);
    updateStatusBar(QString("Reset xong — Server đang chạy tại %1:%2")
                    .arg(m_config.server.serverIp).arg(m_config.server.serverPort));
}

void MainWindow::onStartLog() {
    bool ok;
    QString name = QInputDialog::getText(this, "Tên chuyến bay",
                                          "Nhập tên chuyến bay:", QLineEdit::Normal,
                                          m_config.flightName, &ok);
    if (!ok || name.isEmpty()) return;
    m_config.flightName = name;
    if (!m_logger.open(name)) {
        QMessageBox::warning(this, "Lỗi", "Không thể tạo file CSV!");
        return;
    }
    updateStatusBar("Đang ghi dữ liệu: " + name);
}

void MainWindow::onStopLog() {
    m_logger.close();
    updateStatusBar("Đã dừng ghi dữ liệu");
}

void MainWindow::onSetupServer() {
    SetupServerDialog dlg(m_config.server, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_config.server = dlg.config();
        m_db.saveServerConfig(m_config.server);
    }
}

void MainWindow::onSetup5A42() {
    Setup5A42Dialog dlg(m_config.calib, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_config.calib = dlg.calib();
        m_db.saveCalib5A42(m_config.calib);
        m_server->updateCalib(m_config.calib);
    }
}

void MainWindow::onReset() {
    m_statsTele.reset();
    m_stats5A42.reset();
    m_stats5U44.reset();
    m_stats5I41.reset();
    m_stats5E15.reset();
    m_chart->reset();
    m_errorRatePanel->reset();
    if (m_detail5A42) m_detail5A42->reset();
    updateStatusBar("Đã reset bộ đếm");
}

void MainWindow::onShow5A42Detail() {
    if (!m_detail5A42) {
        m_detail5A42 = new Detail5A42Dialog(this);
        m_detail5A42->setAttribute(Qt::WA_DeleteOnClose, false);
    }
    m_detail5A42->show();
    m_detail5A42->raise();
    m_detail5A42->activateWindow();
}

// ============================================================
// Data receive slots — LIGHTWEIGHT: store latest + log CSV only.
// Panel/chart updates are deferred to onUiTimer at 30 Hz.
// ============================================================
void MainWindow::onTelemetryReceived(GDT::DataTelemetry d) {
    m_statsTele.update(d.index);
    m_latestTele = d;
    m_hasTele    = true;
    if (m_logger.isOpen()) m_logger.writeTelemetry(d);
}

void MainWindow::on5A42Received(GDT::Data5A42 d) {
    m_stats5A42.update(d.index);
    m_latest5A42 = d;
    m_has5A42    = true;
    // Detail dialog accumulates every packet for accurate time axis
    if (m_detail5A42) m_detail5A42->addData5A42(d);
    if (m_logger.isOpen()) m_logger.write5A42(d);
}

void MainWindow::on5U44Received(GDT::Data5U44 d) {
    m_stats5U44.update(d.index);
    m_latest5U44 = d;
    m_has5U44    = true;
    if (m_detail5A42) m_detail5A42->updateK5U(d.k1, d.k2);
    if (m_logger.isOpen()) m_logger.write5U44(d);
}

void MainWindow::on5I41Received(GDT::Data5I41Block d) {
    m_stats5I41.update(d.index);
}

void MainWindow::on5E15Received(GDT::Data5E15 d) {
    m_stats5E15.update(m_idx5E15);
    m_panel5E15->update(d);
    if (m_logger.isOpen()) m_logger.write5E15(d, "multicast", m_idx5E15++);
}

void MainWindow::onClientConnected(QString addr) {
    m_connLabel->setText(QString("Clients: %1").arg(++m_clientCount));
    updateStatusBar("Client kết nối: " + addr);
}

void MainWindow::onClientDisconnected(QString addr) {
    if (m_clientCount > 0) --m_clientCount;
    m_connLabel->setText(QString("Clients: %1").arg(m_clientCount));
    updateStatusBar("Client ngắt kết nối: " + addr);
}

// ============================================================
// UI refresh timer — runs at ~30 Hz on the main thread.
// Updates panels from latest stored data and refreshes error rates.
// ============================================================
void MainWindow::onUiTimer() {
    if (m_hasTele) {
        m_panel5I41->update(m_latestTele);
        m_panelTele->update(m_latestTele);
        m_hasTele = false;
    }
    if (m_has5A42) {
        m_panel5A42->update(m_latest5A42);
        m_chart->addData5A42(m_latest5A42);
        m_has5A42 = false;
    }
    if (m_has5U44) {
        m_panel5U44->update(m_latest5U44);
        m_has5U44 = false;
    }

    // Update error rate display and feed error rate chart
    auto fmtRate = [](const PktStats& s) -> QString {
        if (s.received == 0) return "--- %";
        return QString("%1 %").arg(s.errorRate(), 0, 'f', 5);
    };
    m_errTele->setText("Tỷ lệ lỗi Telemetry: " + fmtRate(m_statsTele));
    m_err5A42->setText("Tỷ lệ lỗi 5A42VT: "    + fmtRate(m_stats5A42));
    m_err5U44->setText("Tỷ lệ lỗi 5U44VT: "    + fmtRate(m_stats5U44));
    m_err5I41->setText("Tỷ lệ lỗi 5I41VT: "    + fmtRate(m_stats5I41));
    m_err5E15->setText("Tỷ lệ lỗi 5E15: "       + fmtRate(m_stats5E15));

    m_errorRatePanel->addSample(
        m_statsTele.errorRate(), m_stats5A42.errorRate(),
        m_stats5U44.errorRate(), m_stats5I41.errorRate(),
        m_stats5E15.errorRate());
}

// ============================================================
void MainWindow::updateStatusBar(const QString& msg) {
    m_statusLabel->setText(msg);
}

void MainWindow::loadConfig() {
    if (!m_db.open()) return;
    m_db.loadServerConfig(m_config.server);
    m_db.loadCalib5A42(m_config.calib);
}

void MainWindow::saveConfig() {
    m_db.saveServerConfig(m_config.server);
    m_db.saveCalib5A42(m_config.calib);
}

} // namespace GDT
