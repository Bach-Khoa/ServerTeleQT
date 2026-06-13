#pragma once
#include <QDialog>
#include "models/AppConfig.h"

class QDoubleSpinBox;

namespace GDT {

class Setup5A42Dialog : public QDialog {
    Q_OBJECT
public:
    explicit Setup5A42Dialog(const Calib5A42& calib, QWidget* parent = nullptr);
    Calib5A42 calib() const;

private:
    QDoubleSpinBox* m_duc1mn, *m_duc1mx;
    QDoubleSpinBox* m_duc2mn, *m_duc2mx;
    QDoubleSpinBox* m_duc3mn, *m_duc3mx;
    QDoubleSpinBox* m_ml1mn,  *m_ml1mx;
    QDoubleSpinBox* m_ml2mn,  *m_ml2mx;
    QDoubleSpinBox* m_ml3mn,  *m_ml3mx;
    QDoubleSpinBox* m_dukMn,  *m_dukMx;
};

} // namespace GDT
