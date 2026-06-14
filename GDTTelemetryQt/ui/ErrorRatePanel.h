#pragma once
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QCheckBox>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

namespace GDT {

// CANoe-style stacked lanes: each packet type gets its own horizontal lane
// with independent Y-axis. All lanes share the same scrolling X-axis (time).
// A signal-list panel on the left shows checkbox + color + name per lane.
class ErrorRatePanel : public QWidget {
    Q_OBJECT
public:
    explicit ErrorRatePanel(QWidget* parent = nullptr);
    // e0=Tele, e1=5A42, e2=5U44, e3=5I41, e4=5E15
    void addSample(double e0, double e1, double e2, double e3, double e4);
    void reset();

private slots:
    void refreshChart();

private:
    static constexpr int    N_CH      = 5;
    static constexpr int    MAX_PTS   = 1800;   // 60 s at 30 Hz
    static constexpr double TIME_STEP = 0.033;

    struct Lane {
        QWidget*     container = nullptr;   // row = [leftW | chartView]
        QChart*      chart     = nullptr;
        QChartView*  view      = nullptr;
        QLineSeries* series    = nullptr;
        QValueAxis*  axisX     = nullptr;
        QValueAxis*  axisY     = nullptr;
        QCheckBox*   check     = nullptr;
        QVector<double> ring;
        bool visible = true;
    };

    void initLane(int ch, bool isBottom);

    Lane            m_lanes[N_CH];
    QVector<double> m_times;
    int    m_wPos    = 0;
    bool   m_full    = false;
    bool   m_dirty   = false;
    double m_timeNow = 0.0;

    QTimer* m_timer = nullptr;
};

} // namespace GDT
