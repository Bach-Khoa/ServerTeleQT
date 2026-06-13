#include "Detail5A42Dialog.h"
#include <QGridLayout>
#include <QPainter>

namespace GDT {

// ----------------------------------------------------------------
// Factory: one QChart + QChartView
// ----------------------------------------------------------------
Detail5A42Dialog::SubChart Detail5A42Dialog::makeChart(
        const QString&     title,
        const QStringList& names,
        const QList<QColor>& colors,
        double yMin, double yMax)
{
    SubChart sc;
    sc.chart = new QChart;
    sc.chart->setTitle(title);
    sc.chart->setBackgroundBrush(QBrush(Qt::white));
    sc.chart->setTitleBrush(QBrush(QColor("#222")));
    sc.chart->legend()->setLabelColor(QColor("#222"));
    sc.chart->setMargins(QMargins(4, 4, 4, 4));

    for (int i = 0; i < names.size(); ++i) {
        auto* s = new QLineSeries;
        s->setName(names[i]);
        s->setColor(colors[i]);
        sc.chart->addSeries(s);
        sc.series.append(s);
    }

    sc.axisX = new QValueAxis;
    sc.axisX->setTitleText("Thời gian (s)");
    sc.axisX->setLabelsBrush(QBrush(QColor("#333")));
    sc.axisX->setTitleBrush(QBrush(QColor("#333")));
    sc.axisX->setGridLineColor(QColor("#ddd"));
    sc.axisX->setRange(0, 10);

    sc.axisY = new QValueAxis;
    sc.axisY->setRange(yMin, yMax);
    sc.axisY->setLabelsBrush(QBrush(QColor("#333")));
    sc.axisY->setTitleBrush(QBrush(QColor("#333")));
    sc.axisY->setGridLineColor(QColor("#ddd"));

    sc.chart->addAxis(sc.axisX, Qt::AlignBottom);
    sc.chart->addAxis(sc.axisY, Qt::AlignLeft);
    for (auto* s : sc.series) {
        s->attachAxis(sc.axisX);
        s->attachAxis(sc.axisY);
    }

    sc.view = new QChartView(sc.chart);
    sc.view->setRenderHint(QPainter::Antialiasing);
    sc.view->setMinimumHeight(200);
    return sc;
}

void Detail5A42Dialog::initRing(SubChart& sc, int nSeries) {
    sc.ring.resize(nSeries);
    for (auto& r : sc.ring) r.resize(MAX_PTS, 0.0);
    sc.times.resize(MAX_PTS, 0.0);
    sc.wPos = 0; sc.full = false; sc.dirty = false; sc.timeNow = 0.0;
}

// ----------------------------------------------------------------
// Write one sample into the ring buffer — O(1)
// ----------------------------------------------------------------
void Detail5A42Dialog::appendToRing(SubChart& sc, const QList<double>& values) {
    sc.times[sc.wPos] = sc.timeNow;
    for (int i = 0; i < values.size() && i < sc.ring.size(); ++i)
        sc.ring[i][sc.wPos] = values[i];
    sc.wPos = (sc.wPos + 1) % MAX_PTS;
    if (sc.wPos == 0) sc.full = true;
    sc.timeNow += TIME_STEP;
    sc.dirty = true;
}

// ----------------------------------------------------------------
// Refresh one sub-chart from its ring — called at 30 Hz
// ----------------------------------------------------------------
void Detail5A42Dialog::refreshSubChart(SubChart& sc) {
    if (!sc.dirty) return;
    sc.dirty = false;

    int count = sc.full ? MAX_PTS : sc.wPos;
    if (count < 2) return;

    for (int ch = 0; ch < sc.series.size(); ++ch) {
        QList<QPointF> pts;
        pts.reserve(count);
        for (int j = 0; j < count; ++j) {
            int ri = sc.full ? (sc.wPos + j) % MAX_PTS : j;
            pts.append(QPointF(sc.times[ri], sc.ring[ch][ri]));
        }
        sc.series[ch]->replace(pts);
    }

    double tEnd   = sc.timeNow;
    double tStart = tEnd > 10.0 ? tEnd - 10.0 : 0.0;
    sc.axisX->setRange(tStart, tEnd < 10.0 ? 10.0 : tEnd);
}

// ----------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------
Detail5A42Dialog::Detail5A42Dialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Chi tiết 5A42 - Control Unit");
    resize(1280, 720);
    setWindowFlag(Qt::Window);

    m_chartLenh = makeChart("Lệnh K1 / K2",
        {"K1(5A42)", "K1(5U44)", "K2(5A42)", "K2(5U44)"},
        {QColor("#cc0000"), QColor("#cc44aa"), QColor("#006600"), QColor("#006699")},
        -1.1, 1.1);

    m_chartMayLai = makeChart("Máy lái ML1 / ML2 / ML3",
        {"ML1", "SP1", "ML2", "SP2", "ML3", "SP3"},
        {QColor("#cc2222"), QColor("#880000"),
         QColor("#007700"), QColor("#004400"),
         QColor("#2222cc"), QColor("#000088")},
        -30, 30);

    m_chartDUC = makeChart("Điều chỉnh DUC / DUK",
        {"DUC1", "DUC2", "DUC3", "DUK"},
        {QColor("#cc5500"), QColor("#888800"), QColor("#006688"), QColor("#6600cc")},
        -30, 30);

    m_chartADC26 = makeChart("Điện áp 26V",
        {"ADC26 [V]"},
        {QColor("#006600")},
        0, 30);

    initRing(m_chartLenh,   4);
    initRing(m_chartMayLai, 6);
    initRing(m_chartDUC,    4);
    initRing(m_chartADC26,  1);

    auto* grid = new QGridLayout(this);
    grid->setSpacing(2);
    grid->setContentsMargins(4, 4, 4, 4);
    grid->setColumnStretch(0, 2);
    grid->setColumnStretch(1, 1);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 1);

    grid->addWidget(m_chartLenh.view,    0, 0);
    grid->addWidget(m_chartMayLai.view,  1, 0);
    grid->addWidget(m_chartDUC.view,     0, 1);
    grid->addWidget(m_chartADC26.view,   1, 1);

    // Refresh charts at ~30 Hz — decoupled from data rate
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Detail5A42Dialog::refreshCharts);
    m_timer->start(33);
}

// ----------------------------------------------------------------
void Detail5A42Dialog::addData5A42(const GDT::Data5A42& d) {
    appendToRing(m_chartLenh,   {d.k1,  m_k15u,  d.k2,  m_k25u});
    appendToRing(m_chartMayLai, {d.ml1, d.sp1, d.ml2, d.sp2, d.ml3, d.sp3});
    appendToRing(m_chartDUC,    {d.duc1, d.duc2, d.duc3, d.duk});
    appendToRing(m_chartADC26,  {d.adc26});
}

void Detail5A42Dialog::refreshCharts() {
    refreshSubChart(m_chartLenh);
    refreshSubChart(m_chartMayLai);
    refreshSubChart(m_chartDUC);
    refreshSubChart(m_chartADC26);
}

void Detail5A42Dialog::updateK5U(double k15u, double k25u) {
    m_k15u = k15u;
    m_k25u = k25u;
}

void Detail5A42Dialog::reset() {
    for (SubChart* sc : {&m_chartLenh, &m_chartMayLai, &m_chartDUC, &m_chartADC26}) {
        for (auto& r : sc->ring) r.fill(0.0);
        sc->times.fill(0.0);
        sc->wPos = 0; sc->full = false; sc->dirty = false; sc->timeNow = 0.0;
        for (auto* s : sc->series) s->clear();
        sc->axisX->setRange(0, 10);
    }
    m_k15u = 0.0;
    m_k25u = 0.0;
}

} // namespace GDT
