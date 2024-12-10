#include "Log.h"

Log::Log() : logId(-1), userId(-1) {}

Log::Log(int logId, int userId, const QString& action, const QDateTime& timestamp,
         const QString& details, const QString& ipAddress, const QString& deviceInfo)
    : logId(logId), userId(userId), action(action), timestamp(timestamp),
    details(details), ipAddress(ipAddress), deviceInfo(deviceInfo) {}

QList<Log> Log::getAllLogs() {
    QList<Log> logs;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    if (!query.exec("SELECT * FROM Logs ORDER BY timestamp DESC")) {
        qDebug() << "Ошибка получения логов:" << query.lastError().text();
        return logs;
    }

    while (query.next()) {
        Log log;
        log.setLogId(query.value("log_id").toInt());
        log.setUserId(query.value("user_id").toInt());
        log.setAction(query.value("action").toString());
        log.setTimestamp(query.value("timestamp").toDateTime());
        log.setDetails(query.value("details").toString());
        log.setIpAddress(query.value("ip_address").toString());
        log.setDeviceInfo(query.value("device_info").toString());
        logs.append(log);
    }

    return logs;
}

bool Log::addLog(const Log& log) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("INSERT INTO Logs (user_id, action, timestamp, details, ip_address, device_info) "
                  "VALUES (:user_id, :action, :timestamp, :details, :ip_address, :device_info)");

    query.bindValue(":user_id", log.getUserId());
    query.bindValue(":action", log.getAction());
    query.bindValue(":timestamp", log.getTimestamp());
    query.bindValue(":details", log.getDetails());
    query.bindValue(":ip_address", log.getIpAddress());
    query.bindValue(":device_info", log.getDeviceInfo());

    if (!query.exec()) {
        qDebug() << "Ошибка добавления лога:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<Log> Log::getLogsByDateRange(const QDate& startDate, const QDate& endDate) {
    QList<Log> logs;
    QSqlQuery query;
    query.prepare("SELECT * FROM Logs WHERE DATE(timestamp) BETWEEN :start_date AND :end_date");
    query.bindValue(":start_date", startDate);
    query.bindValue(":end_date", endDate);

    if (!query.exec()) {
        qDebug() << "Ошибка при получении логов:" << query.lastError().text();
        return logs;
    }

    while (query.next()) {
        Log log;
        log.setLogId(query.value("log_id").toInt());
        log.setUserId(query.value("user_id").toInt());
        log.setAction(query.value("action").toString());
        log.setTimestamp(query.value("timestamp").toDateTime());
        log.setDetails(query.value("details").toString());
        log.setIpAddress(query.value("ip_address").toString());
        log.setDeviceInfo(query.value("device_info").toString());
        logs.append(log);
    }

    return logs;
}

bool Log::deleteLogs(const QDate& startDate, const QDate& endDate) {
    QSqlQuery query;
    query.prepare("DELETE FROM Logs WHERE DATE(timestamp) BETWEEN :start_date AND :end_date");
    query.bindValue(":start_date", startDate);
    query.bindValue(":end_date", endDate);

    return query.exec();
}

bool Log::beginTransaction() {
    return QSqlDatabase::database().transaction();
}

bool Log::commitTransaction() {
    return QSqlDatabase::database().commit();
}

bool Log::rollbackTransaction() {
    return QSqlDatabase::database().rollback();
}
