#pragma once
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include "models/TelemetryData.h"

namespace GDT {

class CsvLogger : public QObject {
    Q_OBJECT
public:
    explicit CsvLogger(QObject* parent = nullptr);
    ~CsvLogger();

    bool open(const QString& flightName);
    void close();
    bool isOpen() const;

    void writeTelemetry(const DataTelemetry& d);
    void write5A42(const Data5A42& d);
    void write5U44(const Data5U44& d);
    void write5E15(const Data5E15& d, const QString& client, uint32_t index);

private slots:
    void flushAll();

private:
    static QString logDir();
    bool openFile(QFile& f, QTextStream& s, const QString& path, const QString& header);

    QFile       m_fileTele,  m_file5A42, m_file5U44, m_file5E15;
    QTextStream m_stTele,    m_st5A42,   m_st5U44,   m_st5E15;
    bool        m_open = false;
    QTimer*     m_flushTimer = nullptr;
};

} // namespace GDT
