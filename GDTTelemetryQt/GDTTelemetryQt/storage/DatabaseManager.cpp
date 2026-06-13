#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace GDT {

DatabaseManager::DatabaseManager() {}
DatabaseManager::~DatabaseManager() {
    QSqlDatabase::removeDatabase(DB_CONN);
}

bool DatabaseManager::open(const QString& path) {
    m_dbPath = path;
    if (m_dbPath.isEmpty()) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        m_dbPath = dir + "/gdt_telemetry.db";
    }

    auto db = QSqlDatabase::addDatabase("QSQLITE", DB_CONN);
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qWarning() << "DB open failed:" << db.lastError().text();
        return false;
    }
    return createTables();
}

bool DatabaseManager::createTables() {
    QSqlDatabase db = QSqlDatabase::database(DB_CONN);
    QSqlQuery q(db);

    bool ok = q.exec(R"(CREATE TABLE IF NOT EXISTS server_config (
        id TEXT PRIMARY KEY,
        server_ip TEXT, server_port INTEGER,
        is_primary INTEGER,
        client_ip TEXT, client_port INTEGER,
        mcast_ip TEXT, mcast_port INTEGER
    ))");

    ok &= q.exec(R"(CREATE TABLE IF NOT EXISTS calib_5a42 (
        id TEXT PRIMARY KEY,
        duc1_min REAL, duc1_max REAL,
        duc2_min REAL, duc2_max REAL,
        duc3_min REAL, duc3_max REAL,
        ml1_min  REAL, ml1_max  REAL,
        ml2_min  REAL, ml2_max  REAL,
        ml3_min  REAL, ml3_max  REAL,
        duk_min  REAL, duk_max  REAL
    ))");

    if (!ok) qWarning() << "createTables error:" << q.lastError().text();
    return ok;
}

bool DatabaseManager::saveServerConfig(const ServerConfig& cfg) {
    QSqlDatabase db = QSqlDatabase::database(DB_CONN);
    QSqlQuery q(db);
    q.prepare(R"(INSERT OR REPLACE INTO server_config VALUES (
        'TCPSERVERPARAM', :sip, :sport, :prim, :cip, :cport, :mip, :mport
    ))");
    q.bindValue(":sip",   cfg.serverIp);
    q.bindValue(":sport", cfg.serverPort);
    q.bindValue(":prim",  cfg.isPrimary ? 1 : 0);
    q.bindValue(":cip",   cfg.clientIp);
    q.bindValue(":cport", cfg.clientPort);
    q.bindValue(":mip",   cfg.mcastIp);
    q.bindValue(":mport", cfg.mcastPort);
    return q.exec();
}

bool DatabaseManager::loadServerConfig(ServerConfig& cfg) {
    QSqlDatabase db = QSqlDatabase::database(DB_CONN);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM server_config WHERE id='TCPSERVERPARAM'");
    if (!q.exec() || !q.next()) return false;
    cfg.serverIp   = q.value("server_ip").toString();
    cfg.serverPort = q.value("server_port").toInt();
    cfg.isPrimary  = q.value("is_primary").toInt() != 0;
    cfg.clientIp   = q.value("client_ip").toString();
    cfg.clientPort = q.value("client_port").toInt();
    cfg.mcastIp    = q.value("mcast_ip").toString();
    cfg.mcastPort  = q.value("mcast_port").toInt();
    return true;
}

bool DatabaseManager::saveCalib5A42(const Calib5A42& c) {
    QSqlDatabase db = QSqlDatabase::database(DB_CONN);
    QSqlQuery q(db);
    q.prepare(R"(INSERT OR REPLACE INTO calib_5a42 VALUES (
        '5A42PARAM',
        :duc1mn,:duc1mx, :duc2mn,:duc2mx, :duc3mn,:duc3mx,
        :ml1mn,:ml1mx,   :ml2mn,:ml2mx,   :ml3mn,:ml3mx,
        :dukMn,:dukMx
    ))");
    q.bindValue(":duc1mn", c.adc_duc1_min); q.bindValue(":duc1mx", c.adc_duc1_max);
    q.bindValue(":duc2mn", c.adc_duc2_min); q.bindValue(":duc2mx", c.adc_duc2_max);
    q.bindValue(":duc3mn", c.adc_duc3_min); q.bindValue(":duc3mx", c.adc_duc3_max);
    q.bindValue(":ml1mn",  c.adc_ml1_min);  q.bindValue(":ml1mx",  c.adc_ml1_max);
    q.bindValue(":ml2mn",  c.adc_ml2_min);  q.bindValue(":ml2mx",  c.adc_ml2_max);
    q.bindValue(":ml3mn",  c.adc_ml3_min);  q.bindValue(":ml3mx",  c.adc_ml3_max);
    q.bindValue(":dukMn",  c.adc_duk_min);  q.bindValue(":dukMx",  c.adc_duk_max);
    return q.exec();
}

bool DatabaseManager::loadCalib5A42(Calib5A42& c) {
    QSqlDatabase db = QSqlDatabase::database(DB_CONN);
    QSqlQuery q(db);
    q.prepare("SELECT * FROM calib_5a42 WHERE id='5A42PARAM'");
    if (!q.exec() || !q.next()) return false;
    c.adc_duc1_min = q.value("duc1_min").toDouble(); c.adc_duc1_max = q.value("duc1_max").toDouble();
    c.adc_duc2_min = q.value("duc2_min").toDouble(); c.adc_duc2_max = q.value("duc2_max").toDouble();
    c.adc_duc3_min = q.value("duc3_min").toDouble(); c.adc_duc3_max = q.value("duc3_max").toDouble();
    c.adc_ml1_min  = q.value("ml1_min").toDouble();  c.adc_ml1_max  = q.value("ml1_max").toDouble();
    c.adc_ml2_min  = q.value("ml2_min").toDouble();  c.adc_ml2_max  = q.value("ml2_max").toDouble();
    c.adc_ml3_min  = q.value("ml3_min").toDouble();  c.adc_ml3_max  = q.value("ml3_max").toDouble();
    c.adc_duk_min  = q.value("duk_min").toDouble();  c.adc_duk_max  = q.value("duk_max").toDouble();
    return true;
}

} // namespace GDT
