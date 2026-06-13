#include "MulticastReceiver.h"
#include "core/Constants.h"
#include <QNetworkInterface>

namespace GDT {

MulticastReceiver::MulticastReceiver(QObject* parent)
    : QObject(parent), m_socket(new QUdpSocket(this))
{
    connect(m_socket, &QUdpSocket::readyRead, this, &MulticastReceiver::onReadyRead);
}

MulticastReceiver::~MulticastReceiver() { stop(); }

bool MulticastReceiver::start(const QString& mcastIp, int port) {
    if (!m_socket->bind(QHostAddress::AnyIPv4, (quint16)port,
                        QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit error("Cannot bind UDP: " + m_socket->errorString());
        return false;
    }
    QHostAddress group(mcastIp);
    if (!m_socket->joinMulticastGroup(group)) {
        emit error("Cannot join multicast: " + m_socket->errorString());
        m_socket->close();
        return false;
    }
    return true;
}

void MulticastReceiver::stop() {
    m_socket->close();
}

bool MulticastReceiver::isRunning() const {
    return m_socket->state() == QAbstractSocket::BoundState;
}

void MulticastReceiver::onReadyRead() {
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize((int)m_socket->pendingDatagramSize());
        m_socket->readDatagram(datagram.data(), datagram.size());

        if (datagram.size() < 7) continue;

        // Validate 5E15 frame header: AB CD EF FE
        if ((uint8_t)datagram[0] != HEADER1 || (uint8_t)datagram[1] != HEADER2 ||
            (uint8_t)datagram[2] != HEADER3 || (uint8_t)datagram[3] != HEADER4) continue;

        // Validate tailer: E1 E2 E3 (at len-5, len-4, len-3 before CRC)
        int n = datagram.size();
        bool isEvent = (n == LEN_EVENT_5E15);
        bool isVT    = (n == LEN_5E15VT);
        if (!isEvent && !isVT) continue;

        if (!m_parser.checkCrc16_1021(datagram)) continue;

        emit data5E15Received(m_parser.parse5E15VT(datagram));
    }
}

} // namespace GDT
