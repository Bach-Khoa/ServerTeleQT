#include "Setup5A42Dialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>

namespace GDT {

static QDoubleSpinBox* makeSpin(double v) {
    auto* s = new QDoubleSpinBox;
    s->setRange(-999999, 999999); s->setDecimals(1); s->setValue(v);
    return s;
}

Setup5A42Dialog::Setup5A42Dialog(const Calib5A42& c, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Cài đặt ADC 5A42");
    setMinimumWidth(400);
    auto* vl = new QVBoxLayout(this);

    auto* grp = new QGroupBox("Min / Max ADC");
    auto* f   = new QFormLayout(grp);

    auto addRow = [&](const QString& name, QDoubleSpinBox*& mn, QDoubleSpinBox*& mx, double vMn, double vMx) {
        mn = makeSpin(vMn); mx = makeSpin(vMx);
        auto* h = new QHBoxLayout;
        h->addWidget(new QLabel("Min:")); h->addWidget(mn);
        h->addWidget(new QLabel("Max:")); h->addWidget(mx);
        f->addRow(name, h);
    };

    addRow("DUC1:", m_duc1mn, m_duc1mx, c.adc_duc1_min, c.adc_duc1_max);
    addRow("DUC2:", m_duc2mn, m_duc2mx, c.adc_duc2_min, c.adc_duc2_max);
    addRow("DUC3:", m_duc3mn, m_duc3mx, c.adc_duc3_min, c.adc_duc3_max);
    addRow("ML1:",  m_ml1mn,  m_ml1mx,  c.adc_ml1_min,  c.adc_ml1_max);
    addRow("ML2:",  m_ml2mn,  m_ml2mx,  c.adc_ml2_min,  c.adc_ml2_max);
    addRow("ML3:",  m_ml3mn,  m_ml3mx,  c.adc_ml3_min,  c.adc_ml3_max);
    addRow("DUK:",  m_dukMn,  m_dukMx,  c.adc_duk_min,  c.adc_duk_max);

    vl->addWidget(grp);
    auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    vl->addWidget(btns);
}

Calib5A42 Setup5A42Dialog::calib() const {
    Calib5A42 c;
    c.adc_duc1_min = m_duc1mn->value(); c.adc_duc1_max = m_duc1mx->value();
    c.adc_duc2_min = m_duc2mn->value(); c.adc_duc2_max = m_duc2mx->value();
    c.adc_duc3_min = m_duc3mn->value(); c.adc_duc3_max = m_duc3mx->value();
    c.adc_ml1_min  = m_ml1mn->value();  c.adc_ml1_max  = m_ml1mx->value();
    c.adc_ml2_min  = m_ml2mn->value();  c.adc_ml2_max  = m_ml2mx->value();
    c.adc_ml3_min  = m_ml3mn->value();  c.adc_ml3_max  = m_ml3mx->value();
    c.adc_duk_min  = m_dukMn->value();  c.adc_duk_max  = m_dukMx->value();
    return c;
}

} // namespace GDT
