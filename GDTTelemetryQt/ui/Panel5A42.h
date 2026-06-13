#pragma once
#include <QWidget>
#include "models/TelemetryData.h"

class QLabel;
class QPushButton;
namespace GDT { class LedIndicator; }

namespace GDT {

class Panel5A42 : public QWidget {
    Q_OBJECT
public:
    explicit Panel5A42(QWidget* parent = nullptr);

public slots:
    void update(const GDT::Data5A42& d);

signals:
    void detailRequested();

private:
    void buildUi();
    QLabel* makeValueLabel(const QString& initText = "---");

    // Value labels
    QLabel* m_lK1, *m_lK2, *m_lAdc26;
    QLabel* m_lMl1, *m_lMl2, *m_lMl3;
    QLabel* m_lDuc1, *m_lDuc2, *m_lDuc3, *m_lDuk;
    QLabel* m_lSp1, *m_lSp2, *m_lSp3;
    QLabel* m_lIndex;

    // LED status
    LedIndicator* m_ledDieuKhien, *m_ledThaoHam, *m_ledKetNoi5U;
    LedIndicator* m_ledFlash, *m_ledADC, *m_ledML;
    LedIndicator* m_ledDUC1, *m_ledDUC2, *m_ledDUC3;
    LedIndicator* m_ledDUK, *m_ledV26;
    LedIndicator* m_ledML1, *m_ledML2, *m_ledML3;
    LedIndicator* m_ledSelfTest;
    QPushButton*  m_btnDetail;
};

} // namespace GDT
