#pragma once
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "models/TelemetryData.h"

namespace GDT {

// Live-monitor panel: shows ML1/2/3 and DUC1/2/3 from 5A42 data.
// Uses ring buffer + 30Hz refresh timer to avoid O(n) remove(0) lag.
class ChartPanel : public QWidget {
    Q_OBJECT
public:
    explicit ChartPanel(QWidget* parent = nullptr);

public slots:
    void addData5A42(const GDT::Data5A42& d);
    void reset();

private slots:
    void refreshChart();

private:
    static constexpr int N_SERIES   = 6;
    static constexpr int MAX_POINTS = 500;

    QChartView*  m_view;
    QChart*      m_chart;
    QLineSeries* m_series[N_SERIES];
    QValueAxis*  m_axisX;
    QValueAxis*  m_axisY;

    // Ring buffer: write O(1), refresh O(n) at 30 Hz max
    QVector<double> m_ring[N_SERIES];
    int  m_wPos  = 0;
    bool m_full  = false;
    bool m_dirty = false;

    QTimer* m_timer;
};

} // namespace GDT
