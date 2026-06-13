#include "ChartPanel.h"
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <QPainter>

namespace GDT {

ChartPanel::ChartPanel(QWidget* parent) : QWidget(parent) {
    m_chart = new QChart;
    m_chart->setTitle("Live Monitor — Máy lái / Điều chỉnh");
    m_chart->setBackgroundBrush(QBrush(Qt::white));
    m_chart->setTitleBrush(QBrush(QColor("#222")));
    m_chart->legend()->setLabelColor(QColor("#222"));

    const QString names[N_SERIES] = {"ML1","ML2","ML3","DUC1","DUC2","DUC3"};
    const QColor  colors[N_SERIES] = {
        QColor("#cc2222"), QColor("#007700"), QColor("#2222cc"),
        QColor("#cc5500"), QColor("#888800"), QColor("#006688")
    };
    for (int i = 0; i < N_SERIES; ++i) {
        m_series[i] = new QLineSeries;
        m_series[i]->setName(names[i]);
        m_series[i]->setColor(colors[i]);
        m_chart->addSeries(m_series[i]);
    }

    m_axisX = new QValueAxis;
    m_axisX->setTitleText("Sample");
    m_axisX->setLabelsBrush(QBrush(QColor("#333")));
    m_axisX->setTitleBrush(QBrush(QColor("#333")));
    m_axisX->setGridLineColor(QColor("#ddd"));
    m_axisX->setRange(0, MAX_POINTS);

    m_axisY = new QValueAxis;
    m_axisY->setTitleText("Value");
    m_axisY->setRange(-30, 30);
    m_axisY->setLabelsBrush(QBrush(QColor("#333")));
    m_axisY->setTitleBrush(QBrush(QColor("#333")));
    m_axisY->setGridLineColor(QColor("#ddd"));

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    for (int i = 0; i < N_SERIES; ++i) {
        m_series[i]->attachAxis(m_axisX);
        m_series[i]->attachAxis(m_axisY);
    }

    m_view = new QChartView(m_chart, this);
    m_view->setRenderHint(QPainter::Antialiasing);

    auto* vl = new QVBoxLayout(this);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->addWidget(m_view);

    // Pre-allocate ring buffers
    for (auto& r : m_ring) r.resize(MAX_POINTS, 0.0);

    // Refresh chart at ~30 Hz — decoupled from data arrival rate
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ChartPanel::refreshChart);
    m_timer->start(33);
}

void ChartPanel::addData5A42(const GDT::Data5A42& d) {
    // O(1) write into ring buffer
    m_ring[0][m_wPos] = d.ml1;
    m_ring[1][m_wPos] = d.ml2;
    m_ring[2][m_wPos] = d.ml3;
    m_ring[3][m_wPos] = d.duc1;
    m_ring[4][m_wPos] = d.duc2;
    m_ring[5][m_wPos] = d.duc3;
    m_wPos = (m_wPos + 1) % MAX_POINTS;
    if (m_wPos == 0) m_full = true;
    m_dirty = true;
}

void ChartPanel::refreshChart() {
    if (!m_dirty) return;
    m_dirty = false;

    int count = m_full ? MAX_POINTS : m_wPos;
    if (count < 2) return;

    // Build QList<QPointF> in oldest→newest order, then bulk-replace
    for (int ch = 0; ch < N_SERIES; ++ch) {
        QList<QPointF> pts;
        pts.reserve(count);
        for (int j = 0; j < count; ++j) {
            int ri = m_full ? (m_wPos + j) % MAX_POINTS : j;
            pts.append(QPointF(j, m_ring[ch][ri]));
        }
        m_series[ch]->replace(pts);
    }
    m_axisX->setRange(0, count - 1);
}

void ChartPanel::reset() {
    m_wPos  = 0;
    m_full  = false;
    m_dirty = false;
    for (auto& r : m_ring) r.fill(0.0);
    for (int i = 0; i < N_SERIES; ++i) m_series[i]->clear();
    m_axisX->setRange(0, MAX_POINTS);
}

} // namespace GDT
