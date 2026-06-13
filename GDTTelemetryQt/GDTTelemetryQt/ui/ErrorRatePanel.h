#pragma once
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QValueAxis>

namespace GDT {

// 5 mini charts side-by-side, one per packet type.
// Each chart shows error rate % over time with a filled area series.
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
    static constexpr int    N_CHARTS  = 5;
    static constexpr int    MAX_PTS   = 1800;  // 60 s at 30 Hz
    static constexpr double TIME_STEP = 0.033;

    struct SubChart {
        QChart*      chart  = nullptr;
        QChartView*  view   = nullptr;
        QLineSeries* upper  = nullptr;  // actual data line (upper bound of area)
        QAreaSeries* area   = nullptr;  // filled area — lower bound = y0
        QValueAxis*  axisX  = nullptr;
        QValueAxis*  axisY  = nullptr;
        QVector<double> ring;
        QVector<double> times;
        int    wPos    = 0;
        bool   full    = false;
        bool   dirty   = false;
        double timeNow = 0.0;
    };

    void initSubChart(int idx, const QString& title, const QColor& color);
    void appendSample(int idx, double value);
    void refreshSubChart(int idx);

    SubChart m_sub[N_CHARTS];
    QTimer*  m_timer;
};

} // namespace GDT
