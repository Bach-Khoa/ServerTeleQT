#include "Panel5U44.h"
#include "LedIndicator.h"
#include "SignalDragLabel.h"
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace GDT {

static QLabel* mkL(const QString& t) { return new QLabel(t); }

QLabel* Panel5U44::makeValueLabel(const QString& sigName) {
    QLabel* l = sigName.isEmpty() ? new QLabel("---") : new SignalDragLabel(sigName);
    l->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    l->setStyleSheet("background:#f0f8ff; color:#004488; border:1px solid #bbb; padding:2px 4px;");
    l->setMinimumWidth(70);
    return l;
}

Panel5U44::Panel5U44(QWidget* parent) : QWidget(parent) { buildUi(); }

void Panel5U44::buildUi() {
    auto* box = new QGroupBox("5U44 - RF Receiver");
    box->setStyleSheet("QGroupBox{color:#333;font-weight:bold;border:1px solid #aaa;margin-top:6px;}"
                       "QGroupBox::title{subcontrol-origin:margin;left:8px;}");

    auto* g = new QGridLayout; g->setSpacing(4);
    int r = 0;
    auto addRow = [&](const QString& n, QLabel*& v, const QString& sigName = {}) {
        g->addWidget(mkL(n+":"), r, 0);
        v = makeValueLabel(sigName);
        g->addWidget(v, r++, 1);
    };
    addRow("K1",      m_lK1,      "K1");
    addRow("K2",      m_lK2,      "K2");
    addRow("SuyHao",  m_lSuyHao);
    addRow("CongSuat",m_lCongSuat);
    addRow("XungHoi", m_lXungHoi, "Xung Hỏi");
    addRow("Phach",   m_lPhach);
    addRow("Index",   m_lIndex);

    auto addLed = [&](int col, const QString& n, LedIndicator*& led, QColor c = Qt::green) {
        auto* h = new QHBoxLayout; led = new LedIndicator; led->setOnColor(c);
        h->addWidget(led); h->addWidget(mkL(n)); h->setSpacing(3);
        auto* w = new QWidget; w->setLayout(h);
        g->addWidget(w, r, col);
    };

    // IO flags
    addLed(0,"SoiDot",   m_ledSoiDot,     Qt::yellow); addLed(1,"Anode",   m_ledAnode,    Qt::yellow); r++;
    addLed(0,"TachTVao", m_ledTachTangVao, Qt::cyan);   addLed(1,"TachTRa", m_ledTachTangRa,Qt::cyan); r++;
    addLed(0,"Ranh",     m_ledRanh,        Qt::white);  addLed(1,"Chuyen",  m_ledChuyen,   Qt::white); r++;
    addLed(0,"K3",       m_ledK3,          Qt::green);  addLed(1,"K6",      m_ledK6,       Qt::green); r++;
    addLed(0,"K7",       m_ledK7,          Qt::green);  addLed(1,"RxLock",  m_ledRxLock,   Qt::cyan);  r++;
    addLed(0,"TxLock",   m_ledTxLock,      Qt::cyan);   r++;
    // SelfTest
    addLed(0,"AD9643",   m_ledAD9643,  Qt::green); addLed(1,"AD9523",  m_ledAD9523,  Qt::green); r++;
    addLed(0,"ADF4360",  m_ledADF4360, Qt::green); addLed(1,"ADF5355", m_ledADF5355, Qt::green); r++;
    // LiveTest
    addLed(0,"V26",      m_ledV26,     Qt::green); addLed(1,"CurTxRx", m_ledCurTxRx, Qt::green); r++;
    addLed(0,"Cur5VFPGA",m_ledCur5V,   Qt::green); addLed(1,"V40",     m_ledV40,     Qt::green); r++;
    addLed(0,"V575",     m_ledV575,    Qt::green); addLed(1,"Conn5A",  m_ledConn5A,  Qt::green); r++;

    box->setLayout(g);
    auto* vl = new QVBoxLayout(this); vl->setContentsMargins(2,2,2,2); vl->addWidget(box);
}

void Panel5U44::update(const GDT::Data5U44& d) {
    m_lK1->setText(QString::number(d.k1, 'f', 3));
    m_lK2->setText(QString::number(d.k2, 'f', 3));
    m_lSuyHao->setText(QString::number(d.suyHao));
    m_lCongSuat->setText(QString::number(d.congSuat));
    m_lXungHoi->setText(QString::number(d.xungHoi));
    m_lPhach->setText(QString::number(d.phach));
    m_lIndex->setText(QString::number(d.sIndex));

    m_ledSoiDot->setOn(d.soiDot);      m_ledAnode->setOn(d.aNode);
    m_ledTachTangVao->setOn(d.tachTangVao); m_ledTachTangRa->setOn(d.tachTangRa);
    m_ledRanh->setOn(d.ranh);          m_ledChuyen->setOn(d.chuyenDoDoc);
    m_ledK3->setOn(d.k3);              m_ledK6->setOn(d.k6);
    m_ledK7->setOn(d.k7);              m_ledRxLock->setOn(d.rxLock);
    m_ledTxLock->setOn(d.txLock);
    m_ledAD9643->setOn(d.ad9643);      m_ledAD9523->setOn(d.ad9523);
    m_ledADF4360->setOn(d.adf4360);    m_ledADF5355->setOn(d.adf5355);
    m_ledV26->setOn(d.voltage26);      m_ledCurTxRx->setOn(d.currentTxRx);
    m_ledCur5V->setOn(d.current5VFPGA);m_ledV40->setOn(d.voltage40);
    m_ledV575->setOn(d.voltage575);    m_ledConn5A->setOn(d.connection5A);
}

} // namespace GDT
