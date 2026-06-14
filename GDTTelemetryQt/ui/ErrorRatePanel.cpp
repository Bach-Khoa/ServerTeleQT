#include "ErrorRatePanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QFrame>

namespace GDT {

static const char*  kNames[5]  = {"Tele", "5A42", "5U44", "5I41", "5E15"};
static const QColor kColors[5] = {
    QColor("#cc0000"),
    QColor("#0055cc"),
    QColor("#007700"),
    QColor("#cc7700"),
    QColor("#7700bb")
};

ErrorRatePanel::ErrorRatePanel(QWidget* parent) : QWidget(parent) {
    m_times.resize(MAX_PTS, 0.0);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    for (int ch = 0; ch < N_CH; ++ch) {
        initLane(ch, ch == N_CH - 1);
        root->addWidget(m_lanes[ch].container, 1);

        if (ch < N_CH - 1) {
            auto* div = new QFrame;
            div->setFrameShape(QFrame::HLine);
            div->setStyleSheet("color: #dddddd;");
            div->setFixedHeight(1);
            root->addWidget(div);
        }
    }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ErrorRatePanel::refreshChart);
    m_timer->start(100);   // 10 Hz is sufficient for trend view
}

void ErrorRatePanel::initLane(int ch, bool isBottom) {
    auto& ln = m_lanes[ch];
    ln.ring.resize(MAX_PTS, 0.0);

    // ── Series ─────────────────────────────────────────────────────────────
    ln.series = new QLineSeries;
    QPen p(kColors[ch]);
    p.setWidthF(1.5);
    ln.series->setPen(p);
    ln.series->setUseOpenGL(false);

    // ── Axes ────────────────────────────────────────────────────────────────
    QFont tiny("Arial", 6);

    ln.axisX = new QValueAxis;
    ln.axisX->setRange(0, 60);
    ln.axisX->setLabelsVisible(isBottom);
    ln.axisX->setLabelsFont(tiny);
    ln.axisX->setTickCount(isBottom ? 7 : 2);
    ln.axisX->setGridLineColor(QColor("#ebebeb"));
    ln.axisX->setMinorGridLineVisible(false);
    ln.axisX->setLineVisible(true);
    ln.axisX->setLabelFormat(isBottom ? "%.0fs" : "");

    ln.axisY = new QValueAxis;
    ln.axisY->setRange(0, 5);
    ln.axisY->setLabelsFont(tiny);
    ln.axisY->setTickCount(3);
    ln.axisY->setLabelFormat("%.2f");
    ln.axisY->setGridLineColor(QColor("#ebebeb"));
    ln.axisY->setMinorGridLineVisible(false);
    ln.axisY->setTitleVisible(false);

    // ── Chart ───────────────────────────────────────────────────────────────
    ln.chart = new QChart;
    ln.chart->setBackgroundBrush(Qt::white);
    ln.chart->setPlotAreaBackgroundBrush(QColor("#fafafa"));
    ln.chart->setPlotAreaBackgroundVisible(true);
    ln.chart->legend()->hide();
    // bottom margin larger on last lane to give X labels space; all others minimal
    ln.chart->setMargins(QMargins(0, 1, 4, isBottom ? 4 : 1));
    ln.chart->addSeries(ln.series);
    ln.chart->addAxis(ln.axisX, Qt::AlignBottom);
    ln.chart->addAxis(ln.axisY, Qt::AlignLeft);
    ln.series->attachAxis(ln.axisX);
    ln.series->attachAxis(ln.axisY);

    // ── Chart view ──────────────────────────────────────────────────────────
    ln.view = new QChartView(ln.chart);
    ln.view->setRenderHint(QPainter::Antialiasing, true);
    ln.view->setStyleSheet("border: none; background: white;");
    ln.view->setMinimumHeight(55);

    // ── Left signal-list row ────────────────────────────────────────────────
    auto* leftW = new QWidget;
    leftW->setFixedWidth(88);
    leftW->setStyleSheet("background: #f4f4f4;");

    auto* leftHl = new QHBoxLayout(leftW);
    leftHl->setContentsMargins(6, 0, 4, 0);
    leftHl->setSpacing(4);

    ln.check = new QCheckBox;
    ln.check->setChecked(true);
    ln.check->setFixedSize(14, 14);

    // Colored square indicator
    auto* sq = new QLabel;
    sq->setFixedSize(11, 11);
    sq->setStyleSheet(QString(
        "background: %1;"
        "border: 1px solid %2;"
        "border-radius: 1px;")
        .arg(kColors[ch].name())
        .arg(kColors[ch].darker(140).name()));

    auto* nameLbl = new QLabel(kNames[ch]);
    nameLbl->setFont(QFont("Arial", 8, QFont::Bold));
    nameLbl->setStyleSheet(QString("color: %1;").arg(kColors[ch].name()));

    leftHl->addWidget(ln.check);
    leftHl->addWidget(sq);
    leftHl->addWidget(nameLbl);
    leftHl->addStretch();

    connect(ln.check, &QCheckBox::toggled, this, [this, ch](bool on) {
        m_lanes[ch].visible = on;
        m_lanes[ch].view->setVisible(on);
        m_dirty = true;
    });

    // ── Lane container: [leftW | chartView] ─────────────────────────────────
    ln.container = new QWidget;
    auto* laneHl = new QHBoxLayout(ln.container);
    laneHl->setContentsMargins(0, 0, 0, 0);
    laneHl->setSpacing(0);

    // Thin left border line between signal list and chart area
    auto* divV = new QFrame;
    divV->setFrameShape(QFrame::VLine);
    divV->setStyleSheet("color: #cccccc;");
    divV->setFixedWidth(1);

    laneHl->addWidget(leftW);
    laneHl->addWidget(divV);
    laneHl->addWidget(ln.view, 1);
}

void ErrorRatePanel::addSample(double e0, double e1, double e2, double e3, double e4) {
    m_times[m_wPos]         = m_timeNow;
    m_lanes[0].ring[m_wPos] = e0;
    m_lanes[1].ring[m_wPos] = e1;
    m_lanes[2].ring[m_wPos] = e2;
    m_lanes[3].ring[m_wPos] = e3;
    m_lanes[4].ring[m_wPos] = e4;
    m_wPos = (m_wPos + 1) % MAX_PTS;
    if (m_wPos == 0) m_full = true;
    m_timeNow += TIME_STEP;
    m_dirty = true;
}

void ErrorRatePanel::refreshChart() {
    if (!m_dirty) return;
    m_dirty = false;

    int count = m_full ? MAX_PTS : m_wPos;
    if (count < 2) return;

    // Shared X window (all lanes synchronized)
    double tEnd   = m_timeNow;
    double tStart = tEnd > 60.0 ? tEnd - 60.0 : 0.0;
    double tMax   = tEnd < 60.0 ? 60.0 : tEnd;

    for (int ch = 0; ch < N_CH; ++ch) {
        auto& ln = m_lanes[ch];
        if (!ln.visible) continue;

        QList<QPointF> pts;
        pts.reserve(count);
        double yMax = 0.0;

        for (int j = 0; j < count; ++j) {
            int ri = m_full ? (m_wPos + j) % MAX_PTS : j;
            double t = m_times[ri];
            double v = ln.ring[ri];
            pts.append(QPointF(t, v));
            if (t >= tStart) yMax = qMax(yMax, v);
        }
        ln.series->replace(pts);

        // Synchronized X
        ln.axisX->setRange(tStart, tMax);

        // Per-lane auto Y: at least 0.5%, 30% headroom
        double newYMax = qMax(0.5, yMax * 1.30);
        ln.axisY->setRange(0.0, newYMax);

        // Adaptive Y format
        if      (newYMax < 0.01)  ln.axisY->setLabelFormat("%.4f");
        else if (newYMax < 0.1)   ln.axisY->setLabelFormat("%.3f");
        else if (newYMax < 1.0)   ln.axisY->setLabelFormat("%.2f");
        else if (newYMax < 10.0)  ln.axisY->setLabelFormat("%.1f");
        else                      ln.axisY->setLabelFormat("%.0f");
    }
}

void ErrorRatePanel::reset() {
    m_wPos = 0; m_full = false; m_dirty = false; m_timeNow = 0.0;
    m_times.fill(0.0);
    for (int ch = 0; ch < N_CH; ++ch) {
        m_lanes[ch].ring.fill(0.0);
        m_lanes[ch].series->clear();
        m_lanes[ch].axisX->setRange(0, 60);
        m_lanes[ch].axisY->setRange(0, 5);
    }
}

} // namespace GDT
