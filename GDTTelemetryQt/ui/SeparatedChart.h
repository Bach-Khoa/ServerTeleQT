#pragma once
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QColor>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

namespace GDT {

// CANoe-style chart: each signal occupies its own horizontal lane.
// Signals are normalized to [-1, +1] within their lane and stacked top-to-bottom.
// ch=0 appears at the top lane, ch=N-1 at the bottom.
class SeparatedChart : public QWidget {
    Q_OBJECT
public:
    struct SignalDef {
        QString name;
        QColor  color;
        double  yMin;   // expected minimum physical value
        double  yMax;   // expected maximum physical value
    };

    explicit SeparatedChart(const QString& title,
                            const QVector<SignalDef>& sigDefs,
                            QWidget* parent = nullptr);

    // Push one sample for every channel simultaneously (must be called on UI thread)
    void addSamples(const QVector<double>& values);
    void reset();

private slots:
    void refreshChart();

private:
    static constexpr int    MAX_POINTS  = 500;
    static constexpr double LANE_H      = 2.0;  // Y units per lane
    static constexpr int    MAX_CH      = 12;

    int m_nCh = 0;
    QVector<SignalDef> m_sigs;

    QChart*      m_chart  = nullptr;
    QChartView*  m_view   = nullptr;
    QLineSeries* m_series[MAX_CH]   = {};  // signal traces
    QLineSeries* m_zeroLine[MAX_CH] = {};  // dashed centre line per lane
    QValueAxis*  m_axisX  = nullptr;
    QValueAxis*  m_axisY  = nullptr;

    // Ring buffers — one per channel
    QVector<double> m_ring[MAX_CH];
    int  m_wPos  = 0;
    bool m_full  = false;
    bool m_dirty = false;

    QTimer* m_timer = nullptr;

    // Map raw value to plot Y coordinate for channel ch
    double toPlotY(double val, int ch) const;
};

} // namespace GDT
