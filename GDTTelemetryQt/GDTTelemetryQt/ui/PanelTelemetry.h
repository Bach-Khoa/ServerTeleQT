#pragma once
#include <QWidget>
#include "models/TelemetryData.h"

class QLabel;

namespace GDT {

class PanelTelemetry : public QWidget {
    Q_OBJECT
public:
    explicit PanelTelemetry(QWidget* parent = nullptr);
public slots:
    void update(const GDT::DataTelemetry& d);
private:
    QLabel* m_lClient, *m_lIndex, *m_lStatus5A, *m_lStatus5U;
    QLabel* m_lPacketCount;
    uint32_t m_count = 0;
};

} // namespace GDT
