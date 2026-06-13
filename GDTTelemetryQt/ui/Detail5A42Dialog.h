#pragma once
#include <QDialog>
#include <QTimer>
#include <QVector>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "models/TelemetryData.h"

namespace GDT {

class Detail5A42Dialog : public QDialog {
    Q_OBJECT
public:
    explicit Detail5A42Dialog(QWidget* parent = nullptr);

public slots:
    void addData5A42(const GDT::Data5A42& d);
    void updateK5U(double k15u, double k25u);
    void reset();

private slots:
    void refreshCharts();

private:
    struct SubChart {
        QChart*     chart  = nullptr;
        QChartView* view   = nullptr;
        QValueAxis* axisX  = nullptr;
        QValueAxis* axisY  = nullptr;
        QVector<QLineSeries*> series;

        // Ring buffer: times[pos] + ring[ch][pos]
        QVector<double>           times;
        QVector<QVector<double>>  ring;
        int    wPos    = 0;
        bool   full    = false;
        bool   dirty   = false;
        double timeNow = 0.0;
    };

    SubChart makeChart(const QString& title,
                       const QStringList& names,
                       const QList<QColor>& colors,
                       double yMin, double yMax);
    void initRing(SubChart& sc, int nSeries);
    void appendToRing(SubChart& sc, const QList<double>& values);
    void refreshSubChart(SubChart& sc);

    SubChart m_chartLenh;    // K1(5A42), K1(5U44), K2(5A42), K2(5U44)
    SubChart m_chartMayLai;  // ML1,SP1, ML2,SP2, ML3,SP3
    SubChart m_chartDUC;     // DUC1, DUC2, DUC3, DUK
    SubChart m_chartADC26;   // ADC26

    double m_k15u = 0.0;
    double m_k25u = 0.0;

    QTimer* m_timer;

    static constexpr int    MAX_PTS   = 3000;
    static constexpr double TIME_STEP = 0.0125;  // 80 Hz
};

} // namespace GDT
