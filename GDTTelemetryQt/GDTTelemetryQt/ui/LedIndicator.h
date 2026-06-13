#pragma once
#include <QWidget>
#include <QColor>

namespace GDT {

// LED indicator widget - thay thế cho LedControl WPF
class LedIndicator : public QWidget {
    Q_OBJECT
    Q_PROPERTY(bool on READ isOn WRITE setOn)
    Q_PROPERTY(QColor onColor  READ onColor  WRITE setOnColor)
    Q_PROPERTY(QColor offColor READ offColor WRITE setOffColor)
public:
    explicit LedIndicator(QWidget* parent = nullptr);

    bool   isOn()    const { return m_on; }
    QColor onColor() const { return m_onColor; }
    QColor offColor()const { return m_offColor; }

public slots:
    void setOn(bool on);
    void setOnColor (QColor c) { m_onColor  = c; update(); }
    void setOffColor(QColor c) { m_offColor = c; update(); }

protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override { return {18, 18}; }

private:
    bool   m_on       = false;
    QColor m_onColor  = Qt::green;
    QColor m_offColor = Qt::darkGray;
};

} // namespace GDT
