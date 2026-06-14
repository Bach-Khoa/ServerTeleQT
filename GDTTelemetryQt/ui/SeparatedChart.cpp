#include "SeparatedChart.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPen>
#include <algorithm>
#include <cmath>

namespace GDT {

SeparatedChart::SeparatedChart(const QString& title,
                               const QVector<SignalDef>& sigDefs,
                               QWidget* parent)
    : QWidget(parent), m_nCh(sigDefs.size()), m_sigs(sigDefs)
{
    Q_ASSERT(m_nCh > 0 && m_nCh <= MAX_CH);

    m_chart = new QChart;
    m_chart->setTitle(title);
    m_chart->setBackgroundBrush(QBrush(QColor("#1a1a2e")));
    m_chart->setTitleBrush(QBrush(QColor("#e0e0e0")));
    m_chart->legend()->setLabelColor(QColor("#cccccc"));
    m_chart->legend()->setAlignment(Qt::AlignRight);
    m_chart->setMargins(QMargins(4, 4, 4, 4));

    double totalY = m_nCh * LANE_H;

    // Build axes first so we can attach series to them
    m_axisX = new QValueAxis;
    m_axisX->setRange(0, MAX_POINTS);
    m_axisX->setLabelsBrush(QBrush(QColor("#888")));
    m_axisX->setGridLineColor(QColor("#2a2a3e"));
    m_axisX->setLabelsVisible(false);
    m_axisX->setTitleVisible(false);

    m_axisY = new QValueAxis;
    m_axisY->setRange(0, totalY);
    m_axisY->setLabelsVisible(false);
    m_axisY->setGridLineColor(QColor("#2a2a3e"));
    m_axisY->setTitleVisible(false);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // Lane separator lines (drawn at each LANE_H boundary, dark grey)
    for (int i = 1; i < m_nCh; ++i) {
        auto* sep = new QLineSeries;
        QPen sepPen(QColor("#444466"));
        sepPen.setWidth(1);
        sep->setPen(sepPen);
        sep->append(0,          i * LANE_H);
        sep->append(MAX_POINTS, i * LANE_H);
        sep->setName(QString());  // hide from legend
        m_chart->addSeries(sep);
        sep->attachAxis(m_axisX);
        sep->attachAxis(m_axisY);
    }

    // Per-channel zero-reference lines (dashed, same colour but dimmer)
    for (int ch = 0; ch < m_nCh; ++ch) {
        double cy = toPlotY(
            (m_sigs[ch].yMin + m_sigs[ch].yMax) * 0.5,
            ch);   // centre of lane = zero line

        m_zeroLine[ch] = new QLineSeries;
        QPen zp(m_sigs[ch].color.darker(200));
        zp.setStyle(Qt::DashLine);
        zp.setWidth(1);
        m_zeroLine[ch]->setPen(zp);
        m_zeroLine[ch]->append(0,          cy);
        m_zeroLine[ch]->append(MAX_POINTS, cy);
        m_zeroLine[ch]->setName(QString());
        m_chart->addSeries(m_zeroLine[ch]);
        m_zeroLine[ch]->attachAxis(m_axisX);
        m_zeroLine[ch]->attachAxis(m_axisY);
    }

    // Signal series (added last so they render on top)
    for (int ch = 0; ch < m_nCh; ++ch) {
        m_series[ch] = new QLineSeries;
        m_series[ch]->setName(m_sigs[ch].name);
        QPen sp(m_sigs[ch].color);
        sp.setWidth(1);
        m_series[ch]->setPen(sp);
        m_series[ch]->setUseOpenGL(false);  // consistent rendering
        m_chart->addSeries(m_series[ch]);
        m_series[ch]->attachAxis(m_axisX);
        m_series[ch]->attachAxis(m_axisY);

        m_ring[ch].resize(MAX_POINTS, 0.0);
    }

    m_view = new QChartView(m_chart, this);
    m_view->setRenderHint(QPainter::Antialiasing, false);  // off for speed

    auto* vl = new QVBoxLayout(this);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->addWidget(m_view);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SeparatedChart::refreshChart);
    m_timer->start(33);  // ~30 Hz
}

double SeparatedChart::toPlotY(double val, int ch) const {
    double yMin = m_sigs[ch].yMin;
    double yMax = m_sigs[ch].yMax;
    double halfRange = std::max(std::abs(yMax - yMin) * 0.5, 1e-12);
    double centre    = (yMin + yMax) * 0.5;
    // ch=0 → top lane, ch=N-1 → bottom lane
    double laneBottom = (m_nCh - 1 - ch) * LANE_H;
    double laneCentre = laneBottom + LANE_H * 0.5;
    double normalised = std::clamp((val - centre) / halfRange, -1.0, 1.0);
    return laneCentre + normalised * LANE_H * 0.45;
}

void SeparatedChart::addSamples(const QVector<double>& values) {
    int n = std::min((int)values.size(), m_nCh);
    for (int ch = 0; ch < n; ++ch)
        m_ring[ch][m_wPos] = values[ch];
    m_wPos = (m_wPos + 1) % MAX_POINTS;
    if (m_wPos == 0) m_full = true;
    m_dirty = true;
}

void SeparatedChart::refreshChart() {
    if (!m_dirty) return;
    m_dirty = false;

    int count = m_full ? MAX_POINTS : m_wPos;
    if (count < 2) return;

    for (int ch = 0; ch < m_nCh; ++ch) {
        QList<QPointF> pts;
        pts.reserve(count);
        for (int j = 0; j < count; ++j) {
            int ri = m_full ? (m_wPos + j) % MAX_POINTS : j;
            pts.append(QPointF(j, toPlotY(m_ring[ch][ri], ch)));
        }
        m_series[ch]->replace(pts);
    }
    m_axisX->setRange(0, count - 1);
}

void SeparatedChart::reset() {
    m_wPos  = 0;
    m_full  = false;
    m_dirty = false;
    for (int ch = 0; ch < m_nCh; ++ch) {
        m_ring[ch].fill(0.0);
        m_series[ch]->clear();
    }
    m_axisX->setRange(0, MAX_POINTS);
}

} // namespace GDT
