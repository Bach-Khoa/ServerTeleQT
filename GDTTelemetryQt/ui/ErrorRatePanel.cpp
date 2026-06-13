#include "ErrorRatePanel.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QFrame>

namespace GDT {

ErrorRatePanel::ErrorRatePanel(QWidget* parent) : QWidget(parent) {
    auto* hl = new QHBoxLayout(this);
    hl->setContentsMargins(2, 2, 2, 2);
    hl->setSpacing(3);

    const char* names[N_CHARTS] = {"Telemetry", "5A42VT", "5U44VT", "5I41VT", "5E15"};
    const QColor colors[N_CHARTS] = {
        QColor("#cc0000"),
        QColor("#0055cc"),
        QColor("#007700"),
        QColor("#884400"),
        QColor("#6600aa")
    };

    for (int i = 0; i < N_CHARTS; ++i) {
        initSubChart(i, names[i], colors[i]);
        hl->addWidget(m_sub[i].view);
    }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ErrorRatePanel::refreshChart);
    m_timer->start(33);
}

void ErrorRatePanel::initSubChart(int idx, const QString& title, const QColor& color) {
    auto& sc = m_sub[idx];

    // Upper series: the actual data — NOT added to chart directly
    sc.upper = new QLineSeries;
    QPen linePen(color);
    linePen.setWidthF(1.5);
    sc.upper->setPen(linePen);

    // Area series: fill between upper and y=0
    sc.area = new QAreaSeries(sc.upper);
    QPen borderPen(color);
    borderPen.setWidthF(1.5);
    sc.area->setPen(borderPen);
    QColor fill = color;
    fill.setAlpha(70);
    sc.area->setBrush(QBrush(fill));

    sc.chart = new QChart;
    sc.chart->setTitle(title);
    sc.chart->setBackgroundBrush(QBrush(Qt::white));
    sc.chart->setTitleBrush(QBrush(color));
    sc.chart->setTitleFont(QFont("", 8, QFont::Bold));
    sc.chart->legend()->hide();
    sc.chart->setMargins(QMargins(1, 4, 1, 1));
    sc.chart->addSeries(sc.area);

    QFont small("", 7);

    sc.axisX = new QValueAxis;
    sc.axisX->setRange(0, 60);
    sc.axisX->setLabelsFont(small);
    sc.axisX->setTickCount(4);
    sc.axisX->setLabelFormat("%.0f");
    sc.axisX->setGridLineColor(QColor("#ececec"));
    sc.axisX->setLabelsBrush(QBrush(QColor("#666")));

    sc.axisY = new QValueAxis;
    sc.axisY->setRange(0, 5);
    sc.axisY->setLabelsFont(small);
    sc.axisY->setTickCount(3);
    sc.axisY->setLabelFormat("%.1f");
    sc.axisY->setTitleText("%");
    sc.axisY->setTitleFont(QFont("", 7));
    sc.axisY->setTitleBrush(QBrush(QColor("#666")));
    sc.axisY->setGridLineColor(QColor("#ececec"));
    sc.axisY->setLabelsBrush(QBrush(QColor("#666")));

    sc.chart->addAxis(sc.axisX, Qt::AlignBottom);
    sc.chart->addAxis(sc.axisY, Qt::AlignLeft);
    sc.area->attachAxis(sc.axisX);
    sc.area->attachAxis(sc.axisY);

    sc.view = new QChartView(sc.chart);
    sc.view->setRenderHint(QPainter::Antialiasing);
    sc.view->setMinimumHeight(100);
    sc.view->setFrameShape(QFrame::StyledPanel);

    sc.ring.resize(MAX_PTS, 0.0);
    sc.times.resize(MAX_PTS, 0.0);
}

void ErrorRatePanel::appendSample(int idx, double value) {
    auto& sc = m_sub[idx];
    sc.times[sc.wPos] = sc.timeNow;
    sc.ring[sc.wPos]  = value;
    sc.wPos = (sc.wPos + 1) % MAX_PTS;
    if (sc.wPos == 0) sc.full = true;
    sc.timeNow += TIME_STEP;
    sc.dirty = true;
}

void ErrorRatePanel::addSample(double e0, double e1, double e2, double e3, double e4) {
    appendSample(0, e0);
    appendSample(1, e1);
    appendSample(2, e2);
    appendSample(3, e3);
    appendSample(4, e4);
}

void ErrorRatePanel::refreshSubChart(int idx) {
    auto& sc = m_sub[idx];
    if (!sc.dirty) return;
    sc.dirty = false;

    int count = sc.full ? MAX_PTS : sc.wPos;
    if (count < 2) return;

    QList<QPointF> pts;
    pts.reserve(count);
    double yMax = 0.0;
    for (int j = 0; j < count; ++j) {
        int ri = sc.full ? (sc.wPos + j) % MAX_PTS : j;
        double y = sc.ring[ri];
        pts.append(QPointF(sc.times[ri], y));
        yMax = qMax(yMax, y);
    }
    sc.upper->replace(pts);

    // Auto-scale Y: at least 5%, 20% headroom above observed max
    double newMax = qMin(100.0, qMax(5.0, yMax * 1.25));
    sc.axisY->setRange(0, newMax);

    double tEnd   = sc.timeNow;
    double tStart = tEnd > 60.0 ? tEnd - 60.0 : 0.0;
    sc.axisX->setRange(tStart, tEnd < 60.0 ? 60.0 : tEnd);
}

void ErrorRatePanel::refreshChart() {
    for (int i = 0; i < N_CHARTS; ++i)
        refreshSubChart(i);
}

void ErrorRatePanel::reset() {
    for (int i = 0; i < N_CHARTS; ++i) {
        auto& sc = m_sub[i];
        sc.wPos = 0; sc.full = false; sc.dirty = false; sc.timeNow = 0.0;
        sc.ring.fill(0.0);
        sc.times.fill(0.0);
        sc.upper->clear();
        sc.axisX->setRange(0, 60);
        sc.axisY->setRange(0, 5);
    }
}

} // namespace GDT
