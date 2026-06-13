#pragma once
#include <QWidget>
#include "models/TelemetryData.h"

class QLabel;

namespace GDT {

class Panel5I41 : public QWidget {
    Q_OBJECT
public:
    explicit Panel5I41(QWidget* parent = nullptr);
public slots:
    void update(const GDT::DataTelemetry& d);
private:
    QLabel* m_lAdc26, *m_lAdc36, *m_lAdc115, *m_lFreq;
    QLabel* m_lXGyro, *m_lYGyro, *m_lZGyro;
    QLabel* m_lXAccl, *m_lYAccl, *m_lZAccl;
    QLabel* m_lIndex;
};

} // namespace GDT
