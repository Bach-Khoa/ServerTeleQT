#include "MultiChartWidget.h"
#include <QGridLayout>

namespace GDT {

MultiChartWidget::MultiChartWidget(QWidget* parent) : QWidget(parent) {

    // ── 5U44 (top-left): K1 / K2 / Xung Hỏi ─────────────────────────────
    // K1, K2 range ±1.1 (normalized command); XungHoi = echo pulse 0-255
    QVector<OverlaidChart::SignalDef> sigs5U44 = {
        { "K1",       QColor("#cd5c5c") },   // IndianRed
        { "K2",       QColor("#ff8c00") },   // DarkOrange
        { "Xung Hỏi", QColor("#7b2d8b") },  // Purple
    };
    m_chart5U44 = new OverlaidChart(
        "5U44 — Thu RF (K1 / K2 / Xung Hỏi)",
        sigs5U44, -2.0, 50.0, this);

    // ── 5A42 (top-right): 13 signals — full C# match ─────────────────────
    // Ranges: K1/K2 ≈ ±1; ML/SP ≈ ±32; omega/DUK ≈ ±80; ADC26V ≈ 24V
    QVector<OverlaidChart::SignalDef> sigs5A42 = {
        { "K1",      QColor("#cd5c5c") },   // IndianRed
        { "K2",      QColor("#ff8c00") },   // DarkOrange
        { "ω1",      QColor("#9370db") },   // Purple
        { "ω2",      QColor("#006400") },   // DarkGreen
        { "ω3",      QColor("#4682b4") },   // SteelBlue
        { "DUK",     QColor("#6b8e00") },   // YellowGreen-dark
        { "ADC26V",  QColor("#20b2aa") },   // LightSeaGreen
        { "ML1",     QColor("#8b4513") },   // SaddleBrown
        { "ML2",     QColor("#4169e1") },   // RoyalBlue
        { "ML3",     QColor("#b22222") },   // Firebrick
        { "SP1",     QColor("#b8860b") },   // DarkGoldenrod
        { "SP2",     QColor("#696969") },   // DimGray
        { "SP3",     QColor("#c71585") },   // MediumVioletRed
    };
    m_chart5A42 = new OverlaidChart(
        "5A42 — Máy lái / Điều chỉnh",
        sigs5A42, -100.0, 100.0, this);

    // ── 5E15 (bottom-left): Điện áp 150V / Nhiệt độ DSP / Số xung ────────
    // soXungPhatXa scaled ×1e-3 to bring large counts into view range
    QVector<OverlaidChart::SignalDef> sigs5E15 = {
        { "Điện áp 150V",   QColor("#cd5c5c") },           // IndianRed
        { "Nhiệt độ DSP",   QColor("#ff8c00") },           // DarkOrange
        { "Xung ×10³",      QColor("#7b2d8b"), 1e-3 },    // Purple, /1000
    };
    m_chart5E15 = new OverlaidChart(
        "5E15 — Phát TX (Điện áp / Nhiệt độ / Xung)",
        sigs5E15, 0.0, 160.0, this);

    // ── 5I41 (bottom-right): voltages + freq + temperatures ───────────────
    // freqPWR passed as /10 so 50 Hz → 5; temps in °C (0-100 typical)
    QVector<OverlaidChart::SignalDef> sigs5I41 = {
        { "26VDC",    QColor("#cd5c5c") },   // IndianRed
        { "36VAC",    QColor("#ff8c00") },   // DarkOrange
        { "115VAC",   QColor("#7b2d8b") },   // Purple
        { "Tần số/10",QColor("#006400") },   // DarkGreen
        { "Th CCY",   QColor("#4682b4") },   // SteelBlue
        { "Th CO",    QColor("#6b8e00") },   // YellowGreen-dark
        { "Th SSP",   QColor("#20b2aa") },   // LightSeaGreen
    };
    m_chart5I41 = new OverlaidChart(
        "5I41 — Nguồn / Nhiệt (26V / 36V / 115V)",
        sigs5I41, 0.0, 130.0, this);

    // 2×2 grid — mirrors C# reference layout
    auto* grid = new QGridLayout(this);
    grid->setContentsMargins(2, 2, 2, 2);
    grid->setSpacing(4);
    grid->addWidget(m_chart5U44, 0, 0);
    grid->addWidget(m_chart5A42, 0, 1);
    grid->addWidget(m_chart5E15, 1, 0);
    grid->addWidget(m_chart5I41, 1, 1);
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
}

void MultiChartWidget::addData5U44(const Data5U44& d) {
    m_chart5U44->addSamples({ d.k1, d.k2, (double)d.xungHoi });
}

void MultiChartWidget::addData5A42(const Data5A42& d) {
    // Order must match sigs5A42: K1 K2 ω1 ω2 ω3 DUK ADC26V ML1 ML2 ML3 SP1 SP2 SP3
    m_chart5A42->addSamples({
        d.k1,     d.k2,
        d.omega1, d.omega2, d.omega3,
        d.duk,    d.adc26,
        d.ml1,    d.ml2,    d.ml3,
        d.sp1,    d.sp2,    d.sp3
    });
}

void MultiChartWidget::addData5E15(const Data5E15& d) {
    m_chart5E15->addSamples({
        d.dienAp150V,
        d.nhietDoDsp,
        (double)d.soXungPhatXa   // scaled ×1e-3 in SignalDef
    });
}

void MultiChartWidget::addData5I41(const Data5I41Block& d) {
    // freqPWR /10 here to keep it on the 0-130 axis with voltages & temps
    m_chart5I41->addSamples({
        d.u26VPWR,
        d.u36VPWR,
        d.u115V5U44,
        d.freqPWR / 10.0,
        d.thCcy,
        d.thCo,
        d.thSsp
    });
}

void MultiChartWidget::reset() {
    m_chart5U44->reset();
    m_chart5A42->reset();
    m_chart5E15->reset();
    m_chart5I41->reset();
}

} // namespace GDT
