#include "LedIndicator.h"
#include <QPainter>
#include <QRadialGradient>

namespace GDT {

LedIndicator::LedIndicator(QWidget* parent) : QWidget(parent) {
    setFixedSize(18, 18);
}

void LedIndicator::setOn(bool on) {
    if (m_on != on) { m_on = on; update(); }
}

void LedIndicator::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QColor base = m_on ? m_onColor : m_offColor;
    QRadialGradient grad(7, 6, 9, 5, 4);
    grad.setColorAt(0, base.lighter(150));
    grad.setColorAt(1, base.darker(150));
    p.setBrush(QBrush(grad));
    p.setPen(Qt::NoPen);
    p.drawEllipse(1, 1, width()-2, height()-2);
}

} // namespace GDT
