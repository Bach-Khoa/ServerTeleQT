#pragma once
#include <QObject>
#include <QUdpSocket>
#include "models/TelemetryData.h"
#include "core/PacketParser.h"

namespace GDT {

class MulticastReceiver : public QObject {
    Q_OBJECT
public:
    explicit MulticastReceiver(QObject* parent = nullptr);
    ~MulticastReceiver();

    bool start(const QString& mcastIp, int port);
    void stop();
    bool isRunning() const;

signals:
    void data5E15Received(GDT::Data5E15 data);
    void error(QString message);

private slots:
    void onReadyRead();

private:
    QUdpSocket* m_socket;
    PacketParser m_parser;
    uint32_t     m_index = 0;
};

} // namespace GDT
