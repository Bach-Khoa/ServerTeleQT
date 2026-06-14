#pragma once
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QString>
#include <QColor>
#include <QCheckBox>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class QPushButton;

namespace GDT {

// Overlaid chart: all signals drawn on a single shared chart area.
// Shows physical Y-axis values. Each signal can be toggled via checkbox.
class OverlaidChart : public QWidget {
    Q_OBJECT
public:
    struct SignalDef {
        QString name;
        QColor  color;
        double  scale = 1.0;   // multiplied before storing in ring buffer
    };

    explicit OverlaidChart(const QString& title,
                           const QVector<SignalDef>& sigDefs,
                           double yMin, double yMax,
                           QWidget* parent = nullptr);

    void addSamples(const QVector<double>& values);
    void reset();

public slots:
    void clearAll();   // hide all series
    void showAll();    // show all series

private slots:
    void refreshChart();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    static constexpr int MAX_POINTS = 600;
    static constexpr int MAX_CH     = 16;

    int                m_nCh  = 0;
    QVector<SignalDef> m_sigs;

    QChart*      m_chart  = nullptr;
    QChartView*  m_view   = nullptr;
    QLineSeries* m_series[MAX_CH] = {};
    QValueAxis*  m_axisX  = nullptr;
    QValueAxis*  m_axisY  = nullptr;

    QVector<QCheckBox*> m_checks;

    QVector<double> m_ring[MAX_CH];
    int  m_wPos    = 0;
    bool m_full    = false;
    bool m_dirty   = false;
    bool m_hasData = false;  // false until first addSamples() call
    bool m_paused  = false;  // freeze display (rubber-band zoom active when paused)

    QPushButton* m_btnPause = nullptr;

    QTimer* m_timer = nullptr;
};

} // namespace GDT
