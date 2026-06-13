#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QByteArray>
#include <QMutex>
#include <QThread>
#include "models/TelemetryData.h"
#include "models/AppConfig.h"
#include "core/PacketParser.h"

namespace GDT {

// Worker: nhận data từ 1 TCP client, parse và emit signals
class ClientWorker : public QObject {
    Q_OBJECT
public:
    explicit ClientWorker(QTcpSocket* socket, const PacketParser& parser, QObject* parent = nullptr);
    void stop();

public slots:
    void updateCalib(GDT::Calib5A42 calib);
    void onReadyRead();
    void onDisconnected();

signals:
    void telemetryReceived (GDT::DataTelemetry data);
    void data5A42Received  (GDT::Data5A42 data);
    void data5U44Received  (GDT::Data5U44 data);
    void data5I41Received  (GDT::Data5I41Block data);
    void clientDisconnected(QString address);

private:
    void processBuffer();
    void parseSubPackets(const QByteArray& payload);

    QTcpSocket*  m_socket;
    PacketParser m_parser;  // own copy — no sharing across threads
    QByteArray   m_buffer;
    QString      m_address;
};

// TCP server chính
class TelemetryServer : public QObject {
    Q_OBJECT
public:
    explicit TelemetryServer(QObject* parent = nullptr);
    ~TelemetryServer();

    bool start(const ServerConfig& cfg);
    void stop();
    bool isRunning() const;
    void updateCalib(const Calib5A42& calib);

signals:
    void telemetryReceived (GDT::DataTelemetry data);
    void data5A42Received  (GDT::Data5A42 data);
    void data5U44Received  (GDT::Data5U44 data);
    void data5I41Received  (GDT::Data5I41Block data);
    void clientConnected   (QString address);
    void clientDisconnected(QString address);
    void serverError       (QString message);

    // internal signal to push calib update to worker threads
    void calibUpdated(GDT::Calib5A42 calib);

private slots:
    void onNewConnection();

private:
    QTcpServer*                  m_server;
    QMap<QString, ClientWorker*> m_workers;
    QMap<QString, QThread*>      m_threads;
    PacketParser                 m_parser;   // template parser for new workers
};

} // namespace GDT
