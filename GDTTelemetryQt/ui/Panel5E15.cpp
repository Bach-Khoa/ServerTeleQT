#include "Panel5E15.h"
#include "LedIndicator.h"
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace GDT {

Panel5E15::Panel5E15(QWidget* parent) : QWidget(parent) {
    auto* box = new QGroupBox("5E15 - Transmitter");
    box->setStyleSheet("QGroupBox{color:#333;font-weight:bold;border:1px solid #aaa;margin-top:6px;}"
                       "QGroupBox::title{subcontrol-origin:margin;left:8px;}");
    auto* g = new QGridLayout; g->setSpacing(4);

    auto mkVal = [&]() -> QLabel* {
        auto* l = new QLabel("---");
        l->setStyleSheet("background:#fff8f0;color:#804400;border:1px solid #bbb;padding:2px 4px;");
        l->setAlignment(Qt::AlignRight|Qt::AlignVCenter); l->setMinimumWidth(70); return l;
    };
    int r = 0;
    g->addWidget(new QLabel("NhietDo DSP [°C]:"), r, 0); m_lTemp    = mkVal(); g->addWidget(m_lTemp,   r++, 1);
    g->addWidget(new QLabel("DienAp 150V:"),      r, 0); m_lDienAp  = mkVal(); g->addWidget(m_lDienAp, r++, 1);
    g->addWidget(new QLabel("SoXungPhatXa:"),     r, 0); m_lXung    = mkVal(); g->addWidget(m_lXung,   r++, 1);

    auto addLed = [&](const QString& n, LedIndicator*& led, QColor c) {
        auto* h = new QHBoxLayout; led = new LedIndicator; led->setOnColor(c);
        h->addWidget(led); h->addWidget(new QLabel(n)); h->setSpacing(3);
        auto* w = new QWidget; w->setLayout(h);
        g->addWidget(w, r++, 0, 1, 2);
    };
    addLed("Detonated", m_ledDetonated, Qt::red);
    addLed("K3",        m_ledK3,        Qt::green);
    addLed("NLC",       m_ledNlc,       Qt::yellow);

    box->setLayout(g);
    auto* vl = new QVBoxLayout(this); vl->setContentsMargins(2,2,2,2); vl->addWidget(box);
}

void Panel5E15::update(const GDT::Data5E15& d) {
    m_lTemp->setText(QString::number(d.nhietDoDsp, 'f', 2));
    m_lDienAp->setText(QString::number(d.dienAp150V, 'f', 3));
    m_lXung->setText(QString::number(d.soXungPhatXa));
    m_ledDetonated->setOn(d.detonated);
    m_ledK3->setOn(d.k3);
    m_ledNlc->setOn(d.nlc);
}

} // namespace GDT
