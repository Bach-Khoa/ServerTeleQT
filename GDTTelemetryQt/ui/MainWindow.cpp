#include "MainWindow.h"
#include "Panel5A42.h"
#include "Panel5U44.h"
#include "Panel5E15.h"
#include "Panel5I41.h"
#include "PanelTelemetry.h"
#include "MultiChartWidget.h"
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
#include <QFormLayout>
#include <QApplication>
#include <QPushButton>
#include "SignalDragLabel.h"

using GDT::SignalDragLabel;

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

    // Ask for log name as soon as the window is visible
    QTimer::singleShot(350, this, &MainWindow::promptAndStartLog);
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
    m_multiChart     = new MultiChartWidget;
    m_errorRatePanel = new ErrorRatePanel;

    // ── Full-panel detail dialogs (Qt::Tool — float above main, no taskbar) ──
    auto wrapDlg = [&](const QString& title, QWidget* panel, QSize sz) -> QDialog* {
        auto* dlg = new QDialog(this);
        dlg->setWindowTitle(title);
        dlg->setWindowFlags(Qt::Tool);
        dlg->setMinimumSize(sz);
        auto* vl = new QVBoxLayout(dlg);
        vl->setContentsMargins(4, 4, 4, 4);
        vl->addWidget(panel);
        return dlg;
    };
    m_dlgTele = wrapDlg("Telemetry Stream",    m_panelTele,  {280, 180});
    m_dlg5A42 = wrapDlg("5A42 - Control Unit", m_panel5A42, {360, 540});
    m_dlg5I41 = wrapDlg("5I41 - Inertial",     m_panel5I41, {280, 340});
    m_dlg5U44 = wrapDlg("5U44 - RF",           m_panel5U44, {340, 400});
    m_dlg5E15 = wrapDlg("5E15 - Transmitter",  m_panel5E15, {280, 180});

    // ── Compact sidebar helpers ──────────────────────────────────────────────
    auto mkVal = [](QLabel*& out) {
        out = new QLabel("---");
        out->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        out->setStyleSheet("color:#002244; font-weight:bold; font-size:9pt;"
                           "background:#f0f4ff; border:1px solid #ccd;"
                           "border-radius:2px; padding:1px 5px;");
    };
    auto mkDrag = [](QLabel*& out, const QString& sig) {
        auto* lbl = new SignalDragLabel(sig);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lbl->setStyleSheet(
            "color:#002244; font-weight:bold; font-size:9pt;"
            "background:#e8eeff; border:1px solid #99aacc;"
            "border-radius:2px; padding:1px 5px;");
        out = lbl;
    };
    auto addRow = [](QFormLayout* fl, const QString& name, QLabel* val) {
        auto* lbl = new QLabel(name + ":");
        lbl->setStyleSheet("color:#555; font-size:8pt;");
        fl->addRow(lbl, val);
    };

    // Card builder: colored header + key-value rows + "↗" detail button
    auto makeCard = [&](const QString& title, QColor hdrClr, QDialog* dlg,
                        const std::function<void(QFormLayout*)>& addRows) -> QWidget* {
        auto* card = new QFrame;
        card->setStyleSheet("QFrame{border:1px solid #bbb;border-radius:3px;background:white;}");
        auto* vl = new QVBoxLayout(card);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(0);
        // Header
        auto* hdr = new QWidget;
        hdr->setFixedHeight(24);
        hdr->setStyleSheet(QString("background:%1;border-radius:2px 2px 0 0;").arg(hdrClr.name()));
        auto* hHl = new QHBoxLayout(hdr);
        hHl->setContentsMargins(6, 0, 4, 0); hHl->setSpacing(4);
        auto* hTitle = new QLabel(title);
        hTitle->setStyleSheet("color:white; font-weight:bold; font-size:9pt;"
                              "border:none; background:transparent;");
        hHl->addWidget(hTitle, 1);
        if (dlg) {
            auto* btn = new QPushButton("↗");
            btn->setFixedSize(16, 16);
            btn->setStyleSheet(
                "QPushButton{color:white;background:rgba(255,255,255,0.25);"
                "border:1px solid rgba(255,255,255,0.55);border-radius:2px;font-size:8pt;}"
                "QPushButton:hover{background:rgba(255,255,255,0.45);}");
            btn->setToolTip("Xem chi tiet");
            connect(btn, &QPushButton::clicked, [dlg](){
                dlg->show(); dlg->raise(); dlg->activateWindow();
            });
            hHl->addWidget(btn);
        }
        vl->addWidget(hdr);
        // Body
        auto* body = new QWidget;
        auto* fl = new QFormLayout(body);
        fl->setContentsMargins(6, 4, 6, 4);
        fl->setSpacing(3); fl->setHorizontalSpacing(6);
        fl->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        addRows(fl);
        vl->addWidget(body);
        return card;
    };

    // Create all compact labels
    mkVal(m_cTeleIdx); mkVal(m_cTeleSt5A); mkVal(m_cTeleSt5U);
    mkDrag(m_c5A42K1, "K1");         mkDrag(m_c5A42K2,  "K2");
    mkDrag(m_c5A42Duk, "DUK");       mkDrag(m_c5A42Adc, "ADC26V");
    mkDrag(m_c5I41Adc26, "26VDC");       mkDrag(m_c5I41Adc115, "115VAC");
    mkDrag(m_c5I41Freq,  "Tần số/10");
    mkDrag(m_c5U44K1, "K1");             mkDrag(m_c5U44K2,  "K2");
    mkDrag(m_c5U44Xung,  "Xung Hỏi");
    mkDrag(m_c5E15Volt,  "Điện áp 150V"); mkDrag(m_c5E15Temp, "Nhiệt độ DSP");

    auto* sidebar = new QWidget;
    sidebar->setFixedWidth(195);
    auto* sideVl = new QVBoxLayout(sidebar);
    sideVl->setContentsMargins(4, 4, 4, 4);
    sideVl->setSpacing(4);

    sideVl->addWidget(makeCard("Telemetry", QColor("#004488"), m_dlgTele,
        [&](QFormLayout* fl){ addRow(fl,"Index",m_cTeleIdx);
                               addRow(fl,"St.5A",m_cTeleSt5A);
                               addRow(fl,"St.5U",m_cTeleSt5U); }));
    sideVl->addWidget(makeCard("5A42", QColor("#006633"), m_dlg5A42,
        [&](QFormLayout* fl){ addRow(fl,"K1",    m_c5A42K1);
                               addRow(fl,"K2",    m_c5A42K2);
                               addRow(fl,"DUK",   m_c5A42Duk);
                               addRow(fl,"ADC26V",m_c5A42Adc); }));
    sideVl->addWidget(makeCard("5I41", QColor("#663300"), m_dlg5I41,
        [&](QFormLayout* fl){ addRow(fl,"26V",  m_c5I41Adc26);
                               addRow(fl,"115V", m_c5I41Adc115);
                               addRow(fl,"Freq", m_c5I41Freq); }));
    sideVl->addWidget(makeCard("5U44", QColor("#550066"), m_dlg5U44,
        [&](QFormLayout* fl){ addRow(fl,"K1",   m_c5U44K1);
                               addRow(fl,"K2",   m_c5U44K2);
                               addRow(fl,"Xung", m_c5U44Xung); }));
    sideVl->addWidget(makeCard("5E15", QColor("#555500"), m_dlg5E15,
        [&](QFormLayout* fl){ addRow(fl,"DA 150V",m_c5E15Volt);
                               addRow(fl,"Nhiet", m_c5E15Temp); }));
    sideVl->addStretch();

    // Right: 4-chart widget takes full height
    auto* rightWidget = new QWidget;
    auto* rightVl = new QVBoxLayout(rightWidget);
    rightVl->setContentsMargins(0, 0, 0, 0);
    rightVl->setSpacing(0);
    rightVl->addWidget(m_multiChart, 1);

    // ErrorRatePanel lives in a floating dialog (shown on demand)
    m_errRateDlg = new QDialog(this);
    m_errRateDlg->setWindowTitle("Tỷ lệ lỗi theo thời gian");
    m_errRateDlg->setMinimumSize(720, 420);
    auto* dlgVl = new QVBoxLayout(m_errRateDlg);
    dlgVl->setContentsMargins(4, 4, 4, 4);
    dlgVl->addWidget(m_errorRatePanel);
    m_errRateDlg->hide();

    // Horizontal splitter: compact sidebar left | charts right
    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(sidebar);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({200, 1080});

    // Info bar: error rate labels (larger, color-coded)
    auto* infoBar = new QWidget;
    infoBar->setFixedHeight(38);
    infoBar->setStyleSheet("background:#ddeeff; border-bottom:1px solid #aac;");
    auto* infoLayout = new QHBoxLayout(infoBar);
    infoLayout->setContentsMargins(8, 2, 8, 2);
    infoLayout->setSpacing(12);

    static const QString kErrStyleOk  = "color:#005500; font-weight:bold; font-size:12px;"
                                         "background:#d4edda; border:1px solid #66bb88;"
                                         "border-radius:4px; padding:2px 8px;";
    static const QString kErrStyleInit= "color:#555555; font-weight:bold; font-size:12px;"
                                         "background:#eeeeee; border:1px solid #bbbbbb;"
                                         "border-radius:4px; padding:2px 8px;";

    auto makeLbl = [&](QLabel*& lbl, const QString& init) {
        lbl = new QLabel(init);
        lbl->setStyleSheet(kErrStyleInit);
        infoLayout->addWidget(lbl);
    };

    // Client count — prominent at left
    m_connLabel = new QLabel("Clients: 0");
    m_connLabel->setStyleSheet(
        "color:#005500; font-weight:bold; font-size:12px;"
        "background:#ccffcc; border:1px solid #44aa44;"
        "border-radius:4px; padding:2px 8px;");
    infoLayout->addWidget(m_connLabel);

    // Separator
    auto* sep = new QFrame;
    sep->setFrameShape(QFrame::VLine);
    sep->setFrameShadow(QFrame::Sunken);
    sep->setStyleSheet("color:#aac;");
    infoLayout->addWidget(sep);

    makeLbl(m_errTele, "Tele: ---");
    makeLbl(m_err5A42, "5A42: ---");
    makeLbl(m_err5U44, "5U44: ---");
    makeLbl(m_err5I41, "5I41: ---");
    makeLbl(m_err5E15, "5E15: ---");
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
    tb->addSeparator();
    auto* tbErrChart = tb->addAction("📈 Tỷ lệ lỗi");
    tbErrChart->setCheckable(true);
    tbErrChart->setToolTip("Hiển thị/ẩn đồ thị tỷ lệ lỗi");

    connect(tbStart,    &QAction::triggered, this, &MainWindow::onStartServer);
    connect(tbStop,     &QAction::triggered, this, &MainWindow::onStopServer);
    connect(tbReset,    &QAction::triggered, this, &MainWindow::onResetServer);
    connect(tbErrChart, &QAction::triggered, this, &MainWindow::onToggleErrorRatePanel);
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
    m_multiChart->reset();
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

    // Prompt for new log name after reset (close old log first inside)
    promptAndStartLog();
}

void MainWindow::onStartLog() {
    promptAndStartLog();
}

void MainWindow::promptAndStartLog() {
    // Close any currently open log before asking for a new one
    if (m_logger.isOpen()) {
        m_logger.close();
        updateStatusBar("Đã đóng log cũ");
    }

    bool ok;
    QString name = QInputDialog::getText(
        this, "Set name  / Log",
        "Enter Log name (leave empty or cancel = don't save log):",
        QLineEdit::Normal,
        m_config.flightName, &ok);

    if (!ok || name.trimmed().isEmpty()) {
        updateStatusBar("Không ghi log (bỏ qua)");
        return;
    }

    m_config.flightName = name.trimmed();
    if (!m_logger.open(m_config.flightName)) {
        QMessageBox::warning(this, "Lỗi", "Không thể tạo file CSV!");
        return;
    }
    updateStatusBar("Đang ghi dữ liệu: " + m_config.flightName);
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
    m_multiChart->reset();
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

void MainWindow::onToggleErrorRatePanel() {
    if (m_errRateDlg->isVisible()) {
        m_errRateDlg->hide();
    } else {
        m_errRateDlg->show();
        m_errRateDlg->raise();
        m_errRateDlg->activateWindow();
    }
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
    m_latest5I41 = d;
    m_has5I41    = true;
    if (m_logger.isOpen()) m_logger.write5I41(d);
}

void MainWindow::on5E15Received(GDT::Data5E15 d) {
    m_stats5E15.update(m_idx5E15);
    m_panel5E15->update(d);
    m_latest5E15 = d;
    m_has5E15    = true;
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
        // Compact Telemetry card
        m_cTeleIdx ->setText(QString::number(m_latestTele.index));
        m_cTeleSt5A->setText(QString("0x%1").arg(m_latestTele.status5A, 2, 16, QChar('0')).toUpper());
        m_cTeleSt5U->setText(QString("0x%1").arg(m_latestTele.status5U, 2, 16, QChar('0')).toUpper());
        m_hasTele = false;
    }
    if (m_has5I41) {
        m_multiChart->addData5I41(m_latest5I41);
        // Compact 5I41 card
        m_c5I41Adc26 ->setText(QString("%1V").arg(m_latest5I41.u26VPWR,   0, 'f', 2));
        m_c5I41Adc115->setText(QString("%1V").arg(m_latest5I41.u115V5U44, 0, 'f', 1));
        m_c5I41Freq  ->setText(QString("%1").arg((int)m_latest5I41.freqPWR));
        m_has5I41 = false;
    }
    if (m_has5A42) {
        m_panel5A42->update(m_latest5A42);
        m_multiChart->addData5A42(m_latest5A42);
        // Compact 5A42 card
        m_c5A42K1 ->setText(QString("%1").arg(m_latest5A42.k1,   0, 'f', 3));
        m_c5A42K2 ->setText(QString("%1").arg(m_latest5A42.k2,   0, 'f', 3));
        m_c5A42Duk->setText(QString("%1").arg(m_latest5A42.duk,  0, 'f', 3));
        m_c5A42Adc->setText(QString("%1V").arg(m_latest5A42.adc26,0,'f',2));
        m_has5A42 = false;
    }
    if (m_has5U44) {
        m_panel5U44->update(m_latest5U44);
        m_multiChart->addData5U44(m_latest5U44);
        // Compact 5U44 card
        m_c5U44K1  ->setText(QString("%1").arg(m_latest5U44.k1,      0, 'f', 3));
        m_c5U44K2  ->setText(QString("%1").arg(m_latest5U44.k2,      0, 'f', 3));
        m_c5U44Xung->setText(QString("%1").arg(m_latest5U44.xungHoi));
        m_has5U44 = false;
    }
    if (m_has5E15) {
        m_multiChart->addData5E15(m_latest5E15);
        // Compact 5E15 card
        m_c5E15Volt->setText(QString("%1V").arg(m_latest5E15.dienAp150V,  0, 'f', 1));
        m_c5E15Temp->setText(QString("%1C").arg(m_latest5E15.nhietDoDsp,  0, 'f', 1));
        m_has5E15 = false;
    }

    // Update error rate display (color-coded) and feed the floating chart
    auto applyErrLabel = [](QLabel* lbl, const QString& tag, const PktStats& s) {
        QString styleOk   = "color:#005500; font-weight:bold; font-size:12px;"
                            "background:#d4edda; border:1px solid #66bb88;"
                            "border-radius:4px; padding:2px 8px;";
        QString styleWarn = "color:#664400; font-weight:bold; font-size:12px;"
                            "background:#fff3cd; border:1px solid #ffaa44;"
                            "border-radius:4px; padding:2px 8px;";
        QString styleErr  = "color:#7a0000; font-weight:bold; font-size:12px;"
                            "background:#f8d7da; border:1px solid #f44;"
                            "border-radius:4px; padding:2px 8px;";
        QString styleInit = "color:#555555; font-weight:bold; font-size:12px;"
                            "background:#eeeeee; border:1px solid #bbbbbb;"
                            "border-radius:4px; padding:2px 8px;";
        if (s.received == 0) {
            lbl->setStyleSheet(styleInit);
            lbl->setText(tag + ": ---");
        } else {
            double r = s.errorRate();
            lbl->setText(tag + ": " + QString("%1%").arg(r, 0, 'f', 4));
            if      (r < 0.1)  lbl->setStyleSheet(styleOk);
            else if (r < 1.0)  lbl->setStyleSheet(styleWarn);
            else               lbl->setStyleSheet(styleErr);
        }
    };
    applyErrLabel(m_errTele, "Tele",  m_statsTele);
    applyErrLabel(m_err5A42, "5A42",  m_stats5A42);
    applyErrLabel(m_err5U44, "5U44",  m_stats5U44);
    applyErrLabel(m_err5I41, "5I41",  m_stats5I41);
    applyErrLabel(m_err5E15, "5E15",  m_stats5E15);

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
