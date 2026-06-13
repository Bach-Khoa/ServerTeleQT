#include "Panel5A42.h"
#include "LedIndicator.h"
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace GDT {

static QLabel* mkLabel(const QString& t, bool bold = false) {
    auto* l = new QLabel(t);
    if (bold) { QFont f = l->font(); f.setBold(true); l->setFont(f); }
    return l;
}

QLabel* Panel5A42::makeValueLabel(const QString& t) {
    auto* l = new QLabel(t);
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    l->setStyleSheet("background:#f0f4ff; color:#003080; border:1px solid #bbb; padding:2px 4px;");
    l->setMinimumWidth(70);
    return l;
}

Panel5A42::Panel5A42(QWidget* parent) : QWidget(parent) { buildUi(); }

void Panel5A42::buildUi() {
    auto* box   = new QGroupBox("5A42 - Control Unit");
    box->setStyleSheet("QGroupBox{color:#333; font-weight:bold; border:1px solid #aaa; margin-top:6px;}"
                       "QGroupBox::title{subcontrol-origin:margin; left:8px;}");

    auto* grid = new QGridLayout;
    grid->setSpacing(4);

    auto addRow = [&](int row, const QString& name, QLabel*& val) {
        grid->addWidget(mkLabel(name + ":"), row, 0);
        val = makeValueLabel();
        grid->addWidget(val, row, 1);
    };

    int r = 0;
    addRow(r++, "K1",      m_lK1);
    addRow(r++, "K2",      m_lK2);
    addRow(r++, "ADC26[V]",m_lAdc26);
    addRow(r++, "ML1[°]",  m_lMl1);
    addRow(r++, "ML2[°]",  m_lMl2);
    addRow(r++, "ML3[°]",  m_lMl3);
    addRow(r++, "DUC1",    m_lDuc1);
    addRow(r++, "DUC2",    m_lDuc2);
    addRow(r++, "DUC3",    m_lDuc3);
    addRow(r++, "DUK",     m_lDuk);
    addRow(r++, "SetPt1",  m_lSp1);
    addRow(r++, "SetPt2",  m_lSp2);
    addRow(r++, "SetPt3",  m_lSp3);
    addRow(r++, "Index",   m_lIndex);

    // Status LEDs
    auto addLed = [&](int row, int col, const QString& name, LedIndicator*& led, QColor onC) {
        auto* h = new QHBoxLayout;
        led = new LedIndicator;
        led->setOnColor(onC);
        h->addWidget(led);
        h->addWidget(mkLabel(name));
        h->setSpacing(3);
        auto* w = new QWidget; w->setLayout(h);
        grid->addWidget(w, row, col);
    };

    addLed(r,   0, "DieuKhien", m_ledDieuKhien, Qt::green);
    addLed(r++, 1, "ThaoHam",   m_ledThaoHam,   Qt::yellow);
    addLed(r,   0, "KetNoi5U",  m_ledKetNoi5U,  Qt::cyan);
    addLed(r++, 1, "SelfTest",  m_ledSelfTest,  Qt::green);
    addLed(r,   0, "FlashCfg",  m_ledFlash,  Qt::green);
    addLed(r++, 1, "ADCCamBien",m_ledADC,    Qt::green);
    addLed(r,   0, "ADCML",     m_ledML,     Qt::green);
    addLed(r++, 1, "DUC1",      m_ledDUC1,   Qt::green);
    addLed(r,   0, "DUC2",      m_ledDUC2,   Qt::green);
    addLed(r++, 1, "DUC3",      m_ledDUC3,   Qt::green);
    addLed(r,   0, "DUK",       m_ledDUK,    Qt::green);
    addLed(r++, 1, "V26",       m_ledV26,    Qt::green);
    addLed(r,   0, "ML1",       m_ledML1,    Qt::green);
    addLed(r++, 1, "ML2",       m_ledML2,    Qt::green);
    addLed(r,   0, "ML3",       m_ledML3,    Qt::green);

    m_btnDetail = new QPushButton("Đồ thị chi tiết...");
    m_btnDetail->setStyleSheet("QPushButton{background:#e8f0fe;color:#1a56aa;border:1px solid #aaa;padding:4px;}"
                               "QPushButton:hover{background:#c8dcfc;}");
    grid->addWidget(m_btnDetail, ++r, 0, 1, 2);
    connect(m_btnDetail, &QPushButton::clicked, this, &Panel5A42::detailRequested);

    box->setLayout(grid);
    auto* vl = new QVBoxLayout(this);
    vl->setContentsMargins(2,2,2,2);
    vl->addWidget(box);
}

void Panel5A42::update(const GDT::Data5A42& d) {
    m_lK1->setText(QString::number(d.k1, 'f', 3));
    m_lK2->setText(QString::number(d.k2, 'f', 3));
    m_lAdc26->setText(QString::number(d.adc26, 'f', 3));
    m_lMl1->setText(QString::number(d.ml1, 'f', 2));
    m_lMl2->setText(QString::number(d.ml2, 'f', 2));
    m_lMl3->setText(QString::number(d.ml3, 'f', 2));
    m_lDuc1->setText(QString::number(d.duc1, 'f', 2));
    m_lDuc2->setText(QString::number(d.duc2, 'f', 2));
    m_lDuc3->setText(QString::number(d.duc3, 'f', 2));
    m_lDuk->setText(QString::number(d.duk, 'f', 2));
    m_lSp1->setText(QString::number(d.sp1, 'f', 2));
    m_lSp2->setText(QString::number(d.sp2, 'f', 2));
    m_lSp3->setText(QString::number(d.sp3, 'f', 2));
    m_lIndex->setText(QString::number(d.sIndex));

    m_ledDieuKhien->setOn(d.bCtrlML1);
    m_ledThaoHam->setOn(d.gamaEnable);
    m_ledKetNoi5U->setOn(d.b5uDigital);
    m_ledSelfTest->setOn(d.systemInit);
    m_ledFlash->setOn(d.adcSP);
    m_ledADC->setOn(d.adcFB);
    m_ledML->setOn(d.bADC26);
    m_ledDUC1->setOn(d.bOmega1);
    m_ledDUC2->setOn(d.bOmega2);
    m_ledDUC3->setOn(d.bOmega3);
    m_ledDUK->setOn(d.bGama);
    m_ledV26->setOn(d.kernelSensor);
    m_ledML1->setOn(d.bCtrlML1);
    m_ledML2->setOn(d.bCtrlML2);
    m_ledML3->setOn(d.bCtrlML3);
}

} // namespace GDT
