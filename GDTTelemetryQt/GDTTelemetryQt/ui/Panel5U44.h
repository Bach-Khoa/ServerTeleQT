#pragma once
#include <QWidget>
#include "models/TelemetryData.h"

class QLabel;
namespace GDT { class LedIndicator; }

namespace GDT {

class Panel5U44 : public QWidget {
    Q_OBJECT
public:
    explicit Panel5U44(QWidget* parent = nullptr);

public slots:
    void update(const GDT::Data5U44& d);

private:
    void buildUi();
    QLabel* makeValueLabel();

    QLabel* m_lK1, *m_lK2, *m_lSuyHao, *m_lCongSuat, *m_lXungHoi, *m_lPhach, *m_lIndex;

    LedIndicator* m_ledSoiDot, *m_ledAnode, *m_ledTachTangVao, *m_ledTachTangRa;
    LedIndicator* m_ledRanh, *m_ledChuyen, *m_ledK3, *m_ledK6, *m_ledK7;
    LedIndicator* m_ledRxLock, *m_ledTxLock;
    LedIndicator* m_ledAD9643, *m_ledAD9523, *m_ledADF4360, *m_ledADF5355;
    LedIndicator* m_ledV26, *m_ledCurTxRx, *m_ledCur5V, *m_ledV40, *m_ledV575, *m_ledConn5A;
};

} // namespace GDT
