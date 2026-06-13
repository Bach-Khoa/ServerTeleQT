#pragma once
#include <QWidget>
#include "models/TelemetryData.h"

class QLabel;
namespace GDT { class LedIndicator; }

namespace GDT {

class Panel5E15 : public QWidget {
    Q_OBJECT
public:
    explicit Panel5E15(QWidget* parent = nullptr);
public slots:
    void update(const GDT::Data5E15& d);
private:
    QLabel* m_lTemp, *m_lDienAp, *m_lXung;
    LedIndicator* m_ledDetonated, *m_ledK3, *m_ledNlc;
};

} // namespace GDT
