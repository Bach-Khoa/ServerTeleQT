#include "OverlaidChart.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPainter>
#include <QFont>
#include <QPushButton>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QMimeData>
#include <algorithm>
#include <cmath>

namespace GDT {

OverlaidChart::OverlaidChart(const QString& title,
                             const QVector<SignalDef>& sigDefs,
                             double yMin, double yMax,
                             QWidget* parent)
    : QWidget(parent), m_nCh(sigDefs.size()), m_sigs(sigDefs)
{
    Q_ASSERT(m_nCh > 0 && m_nCh <= MAX_CH);

    // ── Chart ────────────────────────────────────────────────────────────
    m_chart = new QChart;
    m_chart->setBackgroundBrush(Qt::white);
    m_chart->setPlotAreaBackgroundBrush(Qt::white);
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->legend()->setVisible(false);
    m_chart->setMargins(QMargins(2, 2, 8, 2));

    m_axisX = new QValueAxis;
    m_axisX->setRange(0, MAX_POINTS);
    m_axisX->setLabelsVisible(false);
    m_axisX->setTitleVisible(false);
    m_axisX->setGridLineColor(QColor("#e0e0e0"));
    m_axisX->setMinorGridLineVisible(false);
    m_axisX->setLineVisible(false);

    m_axisY = new QValueAxis;
    m_axisY->setRange(yMin, yMax);
    m_axisY->setTickCount(6);
    m_axisY->setLabelFormat("%.0f");
    m_axisY->setLabelsFont(QFont("Arial", 8));
    m_axisY->setGridLineColor(QColor("#e0e0e0"));
    m_axisY->setMinorGridLineVisible(false);
    m_axisY->setTitleVisible(false);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // Zero-reference line when range spans zero
    if (yMin < 0.0 && yMax > 0.0) {
        auto* zero = new QLineSeries;
        QPen zp(QColor("#aaaaaa"));
        zp.setStyle(Qt::DashLine);
        zp.setWidthF(0.8);
        zero->setPen(zp);
        zero->append(0, 0);
        zero->append(MAX_POINTS, 0);
        m_chart->addSeries(zero);
        zero->attachAxis(m_axisX);
        zero->attachAxis(m_axisY);
    }

    for (int ch = 0; ch < m_nCh; ++ch) {
        m_series[ch] = new QLineSeries;
        m_series[ch]->setName(m_sigs[ch].name);
        QPen sp(m_sigs[ch].color);
        sp.setWidth(1);
        sp.setStyle(Qt::DashLine);  // dashed until first packet received
        m_series[ch]->setPen(sp);
        m_series[ch]->setUseOpenGL(false);
        m_chart->addSeries(m_series[ch]);
        m_series[ch]->attachAxis(m_axisX);
        m_series[ch]->attachAxis(m_axisY);

        m_ring[ch].resize(MAX_POINTS, 0.0);
    }
    m_dirty = true;  // draw placeholder on first timer tick

    m_view = new QChartView(m_chart, this);
    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setStyleSheet("background: white; border: none;");

    // Drag events go to viewport() on QGraphicsView, not the outer widget
    m_view->setAcceptDrops(true);
    m_view->viewport()->setAcceptDrops(true);
    m_view->viewport()->installEventFilter(this);

    // ── Checkbox row ─────────────────────────────────────────────────────
    auto* cbWidget = new QWidget;
    cbWidget->setFixedHeight(28);
    auto* cbLayout = new QHBoxLayout(cbWidget);
    cbLayout->setContentsMargins(4, 0, 4, 0);
    cbLayout->setSpacing(6);

    for (int ch = 0; ch < m_nCh; ++ch) {
        auto* cb = new QCheckBox(m_sigs[ch].name);
        cb->setChecked(true);
        cb->setFont(QFont("Arial", 9, QFont::Bold));
        cb->setStyleSheet(QString(
            "QCheckBox { color: %1; spacing: 4px; }"
            "QCheckBox::indicator { width: 12px; height: 12px; }"
        ).arg(m_sigs[ch].color.name()));
        cb->setToolTip(m_sigs[ch].name);

        connect(cb, &QCheckBox::toggled, this, [this, ch](bool on) {
            m_series[ch]->setVisible(on);
            m_dirty = true;
        });
        m_checks.append(cb);
        cbLayout->addWidget(cb);
    }

    cbLayout->addStretch();

    // Thin separator before control buttons
    auto* sep = new QFrame;
    sep->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    sep->setFixedWidth(1);
    cbLayout->addWidget(sep);

    // Control button factory
    static const QString kBtnStyle =
        "QPushButton{"
        "  background:#f0f0f0; border:1px solid #bbb; border-radius:2px;"
        "  font-size:8pt; padding:0 4px;"
        "}"
        "QPushButton:hover{background:#ddeeff; border-color:#88aadd;}"
        "QPushButton:checked{"
        "  background:#ffe8a0; border-color:#cc9900; font-weight:bold;"
        "}";

    auto mkBtn = [&](const QString& text, const QString& tip, bool checkable = false) {
        auto* b = new QPushButton(text);
        b->setFixedHeight(20);
        b->setMinimumWidth(36);
        b->setCheckable(checkable);
        b->setToolTip(tip);
        b->setStyleSheet(kBtnStyle);
        return b;
    };

    auto* btnAll  = mkBtn("✓ Tất", "Hiện tất cả signals");
    auto* btnNone = mkBtn("✗ Xóa", "Ẩn tất cả signals");
    m_btnPause    = mkBtn("⏸", "Dừng đồ thị\n"
                               "• Kéo chuột trái để zoom vùng\n"
                               "• Cuộn chuột để zoom in/out\n"
                               "• Chuột phải để reset zoom", true);

    cbLayout->addWidget(btnAll);
    cbLayout->addWidget(btnNone);
    cbLayout->addWidget(m_btnPause);

    connect(btnAll, &QPushButton::clicked, this, &OverlaidChart::showAll);
    connect(btnNone, &QPushButton::clicked, this, &OverlaidChart::clearAll);

    connect(m_btnPause, &QPushButton::toggled, this, [this](bool paused) {
        m_paused = paused;
        if (paused) {
            m_btnPause->setText("▶");
            m_btnPause->setToolTip("Tiếp tục cập nhật đồ thị");
            // Tint background to indicate frozen state
            m_chart->setBackgroundBrush(QColor("#fffde8"));
            m_chart->setPlotAreaBackgroundBrush(QColor("#fffde8"));
            // Enable rubber-band zoom while paused
            m_view->setRubberBand(QChartView::RectangleRubberBand);
        } else {
            m_btnPause->setText("⏸");
            m_btnPause->setToolTip("Dừng đồ thị\n"
                                   "• Kéo chuột trái để zoom vùng\n"
                                   "• Cuộn chuột để zoom in/out\n"
                                   "• Chuột phải để reset zoom");
            m_chart->setBackgroundBrush(Qt::white);
            m_chart->setPlotAreaBackgroundBrush(Qt::white);
            m_view->setRubberBand(QChartView::NoRubberBand);
            m_chart->zoomReset();
            m_dirty = true;  // re-render with fresh auto-scale
        }
    });

    // ── GroupBox wrapper ──────────────────────────────────────────────────
    auto* gb = new QGroupBox(title);
    gb->setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold; font-size: 10pt;"
        "  border: 1px solid #aaaaaa;"
        "  border-radius: 3px;"
        "  margin-top: 10px;"
        "  background: white;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 8px;"
        "  color: #333333;"
        "}"
    );
    auto* gbLayout = new QVBoxLayout(gb);
    gbLayout->setContentsMargins(2, 2, 2, 2);
    gbLayout->setSpacing(1);
    gbLayout->addWidget(cbWidget);
    gbLayout->addWidget(m_view, 1);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(gb);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &OverlaidChart::refreshChart);
    m_timer->start(40);   // 25 Hz refresh
}

void OverlaidChart::addSamples(const QVector<double>& values) {
    if (!m_hasData) {
        m_hasData = true;
        // First data received — switch all series from dashed to solid
        for (int ch = 0; ch < m_nCh; ++ch) {
            QPen p = m_series[ch]->pen();
            p.setStyle(Qt::SolidLine);
            m_series[ch]->setPen(p);
        }
    }
    int n = std::min((int)values.size(), m_nCh);
    for (int ch = 0; ch < n; ++ch) {
        double raw = values[ch];
        m_ring[ch][m_wPos] = raw * m_sigs[ch].scale;
        // Live value shown in checkbox tooltip
        m_checks[ch]->setToolTip(
            QString("%1\n▶ %2").arg(m_sigs[ch].name).arg(raw, 0, 'f', 3));
    }
    m_wPos = (m_wPos + 1) % MAX_POINTS;
    if (m_wPos == 0) m_full = true;
    m_dirty = true;
}

void OverlaidChart::refreshChart() {
    if (m_paused) return;   // display frozen — don't clear m_dirty so we refresh on resume
    if (!m_dirty) return;
    m_dirty = false;

    if (!m_hasData) {
        // No packet received yet — flat dashed placeholder at y=0
        for (int ch = 0; ch < m_nCh; ++ch) {
            if (!m_series[ch]->isVisible()) continue;
            m_series[ch]->replace(
                QList<QPointF>{{0.0, 0.0}, {double(MAX_POINTS - 1), 0.0}});
        }
        m_axisX->setRange(0, MAX_POINTS - 1);
        return;
    }

    int count = m_full ? MAX_POINTS : m_wPos;
    if (count < 2) return;

    // Collect visible samples for plotting and percentile Y-scale
    QVector<double> allSamples;
    allSamples.reserve(m_nCh * count);
    bool anyVisible = false;

    for (int ch = 0; ch < m_nCh; ++ch) {
        if (!m_series[ch]->isVisible()) continue;
        anyVisible = true;
        QList<QPointF> pts;
        pts.reserve(count);
        for (int j = 0; j < count; ++j) {
            int ri = m_full ? (m_wPos + j) % MAX_POINTS : j;
            double v = m_ring[ch][ri];
            pts.append(QPointF(j, v));
            allSamples.append(v);
        }
        m_series[ch]->replace(pts);
    }
    m_axisX->setRange(0, count - 1);

    if (anyVisible && !allSamples.isEmpty()) {
        // 5th–95th percentile to suppress outlier spikes
        std::sort(allSamples.begin(), allSamples.end());
        int n    = allSamples.size();
        double yLo = allSamples[qMax(0,   (int)(n * 0.05))];
        double yHi = allSamples[qMin(n-1, (int)(n * 0.95))];

        double range = yHi - yLo;
        double pad;
        if (range > 1e-9) {
            pad = range * 0.15;
        } else {
            pad = std::abs(yLo) * 0.10;
            if (pad < 0.01) pad = 0.01;
        }
        yLo -= pad;
        yHi += pad;
        m_axisY->setRange(yLo, yHi);

        double displayRange = yHi - yLo;
        if      (displayRange < 0.02)  m_axisY->setLabelFormat("%.4f");
        else if (displayRange < 0.2)   m_axisY->setLabelFormat("%.3f");
        else if (displayRange < 2.0)   m_axisY->setLabelFormat("%.2f");
        else if (displayRange < 20.0)  m_axisY->setLabelFormat("%.1f");
        else                           m_axisY->setLabelFormat("%.0f");
    }
}

void OverlaidChart::clearAll() {
    for (int ch = 0; ch < m_nCh; ++ch) {
        m_checks[ch]->setChecked(false);
        m_series[ch]->setVisible(false);
    }
    m_dirty = true;
}

void OverlaidChart::showAll() {
    for (int ch = 0; ch < m_nCh; ++ch) {
        m_checks[ch]->setChecked(true);
        m_series[ch]->setVisible(true);
    }
    m_dirty = true;
}

bool OverlaidChart::eventFilter(QObject* obj, QEvent* e) {
    if (obj != m_view->viewport()) return false;

    // ── Drag-drop: enable signal from sidebar/detail panel ───────────────
    if (e->type() == QEvent::DragEnter) {
        auto* de = static_cast<QDragEnterEvent*>(e);
        if (de->mimeData()->hasFormat("application/x-gdt-signal")) {
            de->acceptProposedAction(); return true;
        }
    }
    if (e->type() == QEvent::DragMove) {
        // Must accept DragMove or Qt refuses the Drop
        auto* de = static_cast<QDragMoveEvent*>(e);
        if (de->mimeData()->hasFormat("application/x-gdt-signal")) {
            de->acceptProposedAction(); return true;
        }
    }
    if (e->type() == QEvent::Drop) {
        auto* de = static_cast<QDropEvent*>(e);
        if (de->mimeData()->hasFormat("application/x-gdt-signal")) {
            QString sig = QString::fromUtf8(de->mimeData()->data("application/x-gdt-signal"));
            for (int ch = 0; ch < m_nCh; ++ch) {
                if (m_sigs[ch].name == sig) {
                    m_checks[ch]->setChecked(true);
                    m_series[ch]->setVisible(true);
                    m_dirty = true;
                    break;
                }
            }
            de->acceptProposedAction(); return true;
        }
    }

    // ── Zoom (only when paused) ───────────────────────────────────────────
    if (m_paused) {
        if (e->type() == QEvent::Wheel) {
            auto* we = static_cast<QWheelEvent*>(e);
            double factor = we->angleDelta().y() > 0 ? 1.15 : (1.0 / 1.15);
            m_chart->zoom(factor);
            return true;
        }
        if (e->type() == QEvent::MouseButtonPress) {
            auto* me = static_cast<QMouseEvent*>(e);
            if (me->button() == Qt::RightButton) {
                m_chart->zoomReset();
                return true;
            }
        }
    }

    return false;
}

void OverlaidChart::reset() {
    // Unfreeze display
    m_paused = false;
    if (m_btnPause) {
        m_btnPause->blockSignals(true);
        m_btnPause->setChecked(false);
        m_btnPause->setText("⏸");
        m_btnPause->blockSignals(false);
    }
    m_chart->setBackgroundBrush(Qt::white);
    m_chart->setPlotAreaBackgroundBrush(Qt::white);
    m_view->setRubberBand(QChartView::NoRubberBand);
    m_chart->zoomReset();

    m_wPos    = 0;
    m_full    = false;
    m_hasData = false;
    m_dirty   = true;

    for (int ch = 0; ch < m_nCh; ++ch) {
        m_ring[ch].fill(0.0);
        m_series[ch]->clear();
        QPen p = m_series[ch]->pen();
        p.setStyle(Qt::DashLine);
        m_series[ch]->setPen(p);
        // Reset tooltip to signal name only (no stale value)
        m_checks[ch]->setToolTip(m_sigs[ch].name);
    }
    m_axisX->setRange(0, MAX_POINTS - 1);
}

} // namespace GDT
