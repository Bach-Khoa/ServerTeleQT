#pragma once
#include <QString>
#include "models/AppConfig.h"

namespace GDT {

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    bool open(const QString& path = QString());  // default: app data dir

    bool saveServerConfig(const ServerConfig& cfg);
    bool loadServerConfig(ServerConfig& cfg);

    bool saveCalib5A42(const Calib5A42& calib);
    bool loadCalib5A42(Calib5A42& calib);

private:
    bool createTables();
    QString m_dbPath;
    static constexpr const char* DB_CONN = "gdt_telemetry_db";
};

} // namespace GDT
