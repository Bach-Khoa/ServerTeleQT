#include "SetupServerDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>

namespace GDT {

SetupServerDialog::SetupServerDialog(const ServerConfig& cfg, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Cấu hình TCP Server");
    setMinimumWidth(350);

    auto* mainLay = new QVBoxLayout(this);

    // Server group
    auto* grpServer = new QGroupBox("Server (Primary)");
    auto* fServer   = new QFormLayout(grpServer);
    m_edServerIp   = new QLineEdit(cfg.serverIp);
    m_spServerPort = new QSpinBox; m_spServerPort->setRange(1, 65535); m_spServerPort->setValue(cfg.serverPort);
    m_chkPrimary   = new QCheckBox("Chế độ Primary"); m_chkPrimary->setChecked(cfg.isPrimary);
    fServer->addRow("Server IP:",   m_edServerIp);
    fServer->addRow("Server Port:", m_spServerPort);
    fServer->addRow("",             m_chkPrimary);
    mainLay->addWidget(grpServer);

    // Forward client group
    auto* grpClient = new QGroupBox("Forward Client (Secondary)");
    auto* fClient   = new QFormLayout(grpClient);
    m_edClientIp   = new QLineEdit(cfg.clientIp);
    m_spClientPort = new QSpinBox; m_spClientPort->setRange(1, 65535); m_spClientPort->setValue(cfg.clientPort);
    fClient->addRow("Client IP:",   m_edClientIp);
    fClient->addRow("Client Port:", m_spClientPort);
    mainLay->addWidget(grpClient);

    // Multicast group
    auto* grpMcast = new QGroupBox("Multicast 5E15");
    auto* fMcast   = new QFormLayout(grpMcast);
    m_edMcastIp   = new QLineEdit(cfg.mcastIp);
    m_spMcastPort = new QSpinBox; m_spMcastPort->setRange(1, 65535); m_spMcastPort->setValue(cfg.mcastPort);
    fMcast->addRow("Multicast IP:",   m_edMcastIp);
    fMcast->addRow("Multicast Port:", m_spMcastPort);
    mainLay->addWidget(grpMcast);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLay->addWidget(buttons);
}

ServerConfig SetupServerDialog::config() const {
    ServerConfig c;
    c.serverIp   = m_edServerIp->text().trimmed();
    c.serverPort = m_spServerPort->value();
    c.isPrimary  = m_chkPrimary->isChecked();
    c.clientIp   = m_edClientIp->text().trimmed();
    c.clientPort = m_spClientPort->value();
    c.mcastIp    = m_edMcastIp->text().trimmed();
    c.mcastPort  = m_spMcastPort->value();
    return c;
}

} // namespace GDT
