#include "Panel5I41.h"
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>

namespace GDT {

Panel5I41::Panel5I41(QWidget* parent) : QWidget(parent) {
    auto* box = new QGroupBox("5I41 - Inertial Unit");
    box->setStyleSheet("QGroupBox{color:#333;font-weight:bold;border:1px solid #aaa;margin-top:6px;}"
                       "QGroupBox::title{subcontrol-origin:margin;left:8px;}");
    auto* g = new QGridLayout; g->setSpacing(4);

    auto mkVal = [&]() -> QLabel* {
        auto* l = new QLabel("---");
        l->setStyleSheet("background:#fffaf0;color:#664400;border:1px solid #bbb;padding:2px 4px;");
        l->setAlignment(Qt::AlignRight|Qt::AlignVCenter); l->setMinimumWidth(65); return l;
    };

    int r = 0;
    auto row = [&](const QString& n, QLabel*& v) {
        g->addWidget(new QLabel(n+":"), r, 0);
        v = mkVal(); g->addWidget(v, r++, 1);
    };
    row("ADC26 [V]",  m_lAdc26);
    row("ADC36 [V]",  m_lAdc36);
    row("ADC115 [V]", m_lAdc115);
    row("Freq (Hz)",  m_lFreq);
    row("X Gyro",     m_lXGyro);
    row("Y Gyro",     m_lYGyro);
    row("Z Gyro",     m_lZGyro);
    row("X Accl",     m_lXAccl);
    row("Y Accl",     m_lYAccl);
    row("Z Accl",     m_lZAccl);
    row("Index",      m_lIndex);

    box->setLayout(g);
    auto* vl = new QVBoxLayout(this); vl->setContentsMargins(2,2,2,2); vl->addWidget(box);
}

void Panel5I41::update(const GDT::DataTelemetry& d) {
    m_lAdc26->setText(QString::number(d.infor5I41.adc26, 'f', 3));
    m_lAdc36->setText(QString::number(d.infor5I41.adc36, 'f', 3));
    m_lAdc115->setText(QString::number(d.infor5I41.adc115, 'f', 3));
    m_lFreq->setText(QString::number(d.infor5I41.freq));
    m_lXGyro->setText(QString::number(d.xGyro));
    m_lYGyro->setText(QString::number(d.yGyro));
    m_lZGyro->setText(QString::number(d.zGyro));
    m_lXAccl->setText(QString::number(d.xAccl));
    m_lYAccl->setText(QString::number(d.yAccl));
    m_lZAccl->setText(QString::number(d.zAccl));
    m_lIndex->setText(QString::number(d.index));
}

} // namespace GDT
