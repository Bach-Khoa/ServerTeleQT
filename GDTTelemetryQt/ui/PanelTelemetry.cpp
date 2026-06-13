#include "PanelTelemetry.h"
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>

namespace GDT {

PanelTelemetry::PanelTelemetry(QWidget* parent) : QWidget(parent) {
    auto* box = new QGroupBox("Telemetry Stream");
    box->setStyleSheet("QGroupBox{color:#333;font-weight:bold;border:1px solid #aaa;margin-top:6px;}"
                       "QGroupBox::title{subcontrol-origin:margin;left:8px;}");
    auto* g = new QGridLayout; g->setSpacing(4);

    auto mkVal = [&]() -> QLabel* {
        auto* l = new QLabel("---");
        l->setStyleSheet("background:#f0fff0;color:#006600;border:1px solid #bbb;padding:2px 4px;");
        l->setAlignment(Qt::AlignRight|Qt::AlignVCenter); l->setMinimumWidth(80); return l;
    };
    int r = 0;
    auto row = [&](const QString& n, QLabel*& v) {
        g->addWidget(new QLabel(n+":"), r, 0);
        v = mkVal(); g->addWidget(v, r++, 1);
    };
    row("Client",      m_lClient);
    row("Index",       m_lIndex);
    row("Status 5A",   m_lStatus5A);
    row("Status 5U",   m_lStatus5U);
    row("Gói nhận",    m_lPacketCount);

    box->setLayout(g);
    auto* vl = new QVBoxLayout(this); vl->setContentsMargins(2,2,2,2); vl->addWidget(box);
}

void PanelTelemetry::update(const GDT::DataTelemetry& d) {
    m_lClient->setText(d.client);
    m_lIndex->setText(QString::number(d.index));
    m_lStatus5A->setText(QString("0x%1").arg(d.status5A, 2, 16, QChar('0')).toUpper());
    m_lStatus5U->setText(QString("0x%1").arg(d.status5U, 2, 16, QChar('0')).toUpper());
    m_lPacketCount->setText(QString::number(++m_count));
}

} // namespace GDT
