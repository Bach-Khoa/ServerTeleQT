#pragma once
#include <QWidget>
#include "OverlaidChart.h"
#include "models/TelemetryData.h"

namespace GDT {

// 2×2 grid of OverlaidCharts matching C# reference layout:
//   [5U44]  K1 / K2 / Xung Hỏi              [5A42]  K1/K2/ω1-3/DUK/ADC26/ML1-3/SP1-3
//   [5E15]  Điện áp 150V / Nhiệt độ / Xung  [5I41]  26V/36V/115V/Freq/Th×3
class MultiChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit MultiChartWidget(QWidget* parent = nullptr);

    void addData5I41(const Data5I41Block& d);
    void addData5A42(const Data5A42& d);
    void addData5U44(const Data5U44& d);
    void addData5E15(const Data5E15& d);
    void reset();

private:
    OverlaidChart* m_chart5U44  = nullptr;
    OverlaidChart* m_chart5A42  = nullptr;
    OverlaidChart* m_chart5E15  = nullptr;
    OverlaidChart* m_chart5I41  = nullptr;
};

} // namespace GDT
