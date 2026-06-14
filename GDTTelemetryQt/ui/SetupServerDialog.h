#pragma once
#include <QDialog>
#include "models/AppConfig.h"

class QLineEdit;
class QCheckBox;
class QSpinBox;

namespace GDT {

class SetupServerDialog : public QDialog {
    Q_OBJECT
public:
    explicit SetupServerDialog(const ServerConfig& cfg, QWidget* parent = nullptr);
    ServerConfig config() const;

private:
    QLineEdit* m_edServerIp;
    QSpinBox*  m_spServerPort;
    QCheckBox* m_chkPrimary;
    QLineEdit* m_edMcastIp;
    QSpinBox*  m_spMcastPort;
};

} // namespace GDT
