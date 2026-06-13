#include "TelemetryServer.h"
#include "core/Constants.h"
#include <QHostAddress>
#include <QMetaObject>

namespace GDT {

// ============================================================
// ClientWorker
// ============================================================
ClientWorker::ClientWorker(QTcpSocket* socket, const PacketParser& parser, QObject* parent)
    : QObject(parent), m_socket(socket), m_parser(parser)
{
    m_address = socket->peerAddress().toString() + ":" + QString::number(socket->peerPort());
}

void ClientWorker::stop() {
    // disconnectFromHost sends TCP FIN → GND receives clean disconnect and can reconnect.
    // abort() would send RST which may confuse some clients.
    if (m_socket)
        QMetaObject::invokeMethod(m_socket, "disconnectFromHost", Qt::QueuedConnection);
}

void ClientWorker::updateCalib(GDT::Calib5A42 calib) {
    m_parser.setCalib(calib);
}

void ClientWorker::onDisconnected() {
    emit clientDisconnected(m_address);
}

void ClientWorker::onReadyRead() {
    m_buffer.append(m_socket->readAll());
    processBuffer();
}

void ClientWorker::processBuffer() {
    // Frame: [DE][AD][type][len_H][len_L][data:dlen][EE][ED][CRC_H][CRC_L]
    while (m_buffer.size() >= 9) {
        int start = -1;
        for (int i = 0; i <= m_buffer.size() - 2; ++i) {
            if ((uint8_t)m_buffer[i] == TCP_HDR1 && (uint8_t)m_buffer[i+1] == TCP_HDR2) {
                start = i; break;
            }
        }
        if (start < 0) { m_buffer.clear(); return; }
        if (start > 0)  m_buffer.remove(0, start);

        if (m_buffer.size() < 5) return;

        uint16_t dlen    = (uint16_t)(((uint8_t)m_buffer[3] << 8) | (uint8_t)m_buffer[4]);
        int      frameLen = 2 + 1 + 2 + (int)dlen + 2 + 2;

        if (m_buffer.size() < frameLen) return;

        int tailerPos = 5 + (int)dlen;
        if ((uint8_t)m_buffer[tailerPos]     != TCP_TLR1 ||
            (uint8_t)m_buffer[tailerPos + 1] != TCP_TLR2) {
            m_buffer.remove(0, 1);
            continue;
        }

        // C# server does NOT check CRC16-8005 of the outer TCP frame.
        // TLDK35 occasionally sends frames with wrong CRC bytes but valid sub-packet data.
        // Skipping this check matches C# behavior and eliminates 1-3% false packet loss.

        uint8_t    type    = (uint8_t)m_buffer[2];
        QByteArray payload = m_buffer.mid(5, (int)dlen);
        m_buffer.remove(0, frameLen);

        if (type == TCP_TYPE_TELE)
            parseSubPackets(payload);
    }
}

void ClientWorker::parseSubPackets(const QByteArray& payload) {
    int i = 0;
    while (i < payload.size()) {
        if ((uint8_t)payload[i] != HEADER) { ++i; continue; }
        if (i + 2 > payload.size()) break;

        uint8_t typeBits = ((uint8_t)payload[i + 1] >> 5) & 0x07;
        int len;
        switch (typeBits) {
            case 0: len = LEN_5A42;      break;
            case 1: len = LEN_5U44;      break;
            case 2: len = LEN_TELEMETRY; break;
            case 3: len = LEN_5I41;      break;
            default: ++i; continue;
        }

        if (i + len > payload.size()) break;
        QByteArray pkt = payload.mid(i, len);
        // On tailer or CRC8 mismatch, advance by full `len` (matching C# behavior).
        // Advancing only 1 byte risks misaligning all subsequent sub-packets.
        if ((uint8_t)pkt[len - 2] != TAILER) { i += len; continue; }
        if (!m_parser.checkCrc8(pkt))         { i += len; continue; }

        uint32_t idx = (((uint32_t)(uint8_t)pkt[1] & 0x0F) << 16) |
                       ((uint32_t)(uint8_t)pkt[2] << 8) |
                        (uint32_t)(uint8_t)pkt[3];

        switch (typeBits) {
            case 0: emit data5A42Received (m_parser.parse5A42(pkt, idx, m_address));      break;
            case 1: emit data5U44Received (m_parser.parse5U44(pkt, idx, m_address));      break;
            case 2: emit telemetryReceived(m_parser.parseTelemetry(pkt, idx, m_address)); break;
            case 3: emit data5I41Received (m_parser.parse5I41(pkt, idx, m_address));      break;
        }
        i += len;
    }
}

// ============================================================
// TelemetryServer
// ============================================================
TelemetryServer::TelemetryServer(QObject* parent)
    : QObject(parent), m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &TelemetryServer::onNewConnection);
}

TelemetryServer::~TelemetryServer() { stop(); }

bool TelemetryServer::start(const ServerConfig& cfg) {
    QHostAddress addr(cfg.serverIp);
    if (!m_server->listen(addr, (quint16)cfg.serverPort)) {
        emit serverError(m_server->errorString());
        return false;
    }
    return true;
}

void TelemetryServer::stop() {
    m_server->close();

    // Snapshot before clearing so we can iterate safely.
    // Clear maps first so the clientDisconnected lambda (if it fires) is a no-op.
    QList<ClientWorker*> workers = m_workers.values();
    QList<QThread*>      threads = m_threads.values();
    m_workers.clear();
    m_threads.clear();

    // Gracefully close each socket (sends FIN, not RST)
    for (auto* w : workers) w->stop();

    for (auto* t : threads) {
        t->quit();
        if (!t->wait(2000)) t->terminate(); // last resort; finished may not emit after this
    }
    // worker + socket cleanup is handled by thread::finished → deleteLater connections
    // set up in onNewConnection. No qDeleteAll needed.
}

bool TelemetryServer::isRunning() const {
    return m_server->isListening();
}

void TelemetryServer::updateCalib(const Calib5A42& calib) {
    m_parser.setCalib(calib);
    emit calibUpdated(calib);
}

void TelemetryServer::onNewConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        QString addr = socket->peerAddress().toString() + ":" + QString::number(socket->peerPort());

        auto* thread = new QThread(this);
        auto* worker = new ClientWorker(socket, m_parser);

        socket->setParent(nullptr);
        socket->moveToThread(thread);
        worker->moveToThread(thread);

        // Auto-cleanup when thread finishes: socket and worker deleted on their thread,
        // thread self-deletes on main thread. No manual qDeleteAll needed.
        connect(thread, &QThread::finished, socket, &QObject::deleteLater);
        connect(thread, &QThread::finished, worker, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);

        connect(socket, &QTcpSocket::readyRead,    worker, &ClientWorker::onReadyRead,    Qt::QueuedConnection);
        connect(socket, &QTcpSocket::disconnected, worker, &ClientWorker::onDisconnected, Qt::QueuedConnection);

        connect(worker, &ClientWorker::telemetryReceived, this, &TelemetryServer::telemetryReceived, Qt::QueuedConnection);
        connect(worker, &ClientWorker::data5A42Received,  this, &TelemetryServer::data5A42Received,  Qt::QueuedConnection);
        connect(worker, &ClientWorker::data5U44Received,  this, &TelemetryServer::data5U44Received,  Qt::QueuedConnection);
        connect(worker, &ClientWorker::data5I41Received,  this, &TelemetryServer::data5I41Received,  Qt::QueuedConnection);

        connect(this, &TelemetryServer::calibUpdated, worker, &ClientWorker::updateCalib, Qt::QueuedConnection);

        // Normal client disconnect: remove from maps, quit thread.
        // Do NOT call t->wait() here — that would block the main thread.
        // The thread::finished connections above handle socket/worker/thread cleanup.
        connect(worker, &ClientWorker::clientDisconnected, this,
            [this, addr, thread](const QString&) {
                emit clientDisconnected(addr);
                m_workers.remove(addr);
                m_threads.remove(addr);
                thread->quit(); // triggers finished → deleteLater chain
            }, Qt::QueuedConnection);

        m_workers[addr] = worker;
        m_threads[addr] = thread;
        thread->start();

        emit clientConnected(addr);
    }
}

} // namespace GDT
