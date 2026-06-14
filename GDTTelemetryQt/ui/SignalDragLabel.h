#pragma once
#include <QLabel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QPainter>
#include <QFont>

namespace GDT {

// QLabel subclass that starts a drag with MIME type "application/x-gdt-signal"
// containing the signal name as UTF-8. Drop it onto an OverlaidChart to enable
// the matching series. Style is set by the caller — this class is style-neutral.
class SignalDragLabel : public QLabel {
    Q_OBJECT
public:
    explicit SignalDragLabel(const QString& sigName, QWidget* parent = nullptr)
        : QLabel("---", parent), m_sigName(sigName)
    {
        setCursor(Qt::OpenHandCursor);
        setToolTip("Kéo vào đồ thị để hiện: " + sigName);
    }

protected:
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) m_origin = e->pos();
        QLabel::mousePressEvent(e);
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (!(e->buttons() & Qt::LeftButton)) return;
        if ((e->pos() - m_origin).manhattanLength() < QApplication::startDragDistance()) return;
        auto* mime = new QMimeData;
        mime->setData("application/x-gdt-signal", m_sigName.toUtf8());
        auto* drag = new QDrag(this);
        drag->setMimeData(mime);
        int pw = qMax(90, fontMetrics().horizontalAdvance(m_sigName) + 16);
        QPixmap px(pw, 20);
        px.fill(QColor("#3366aa"));
        QPainter p(&px);
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 8, QFont::Bold));
        p.drawText(px.rect(), Qt::AlignCenter, m_sigName);
        drag->setPixmap(px);
        drag->setHotSpot(QPoint(pw / 2, 10));
        drag->exec(Qt::CopyAction);
    }

private:
    QString m_sigName;
    QPoint  m_origin;
};

} // namespace GDT
