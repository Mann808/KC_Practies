#ifndef LOG_H
#define LOG_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QSqlQuery>
#include <QSqlError>
#include <QNetworkInterface>
#include <QDebug>
#include "../utils/DatabaseManager.h"

class Log {
public:
    Log();
    Log(int logId, int userId, const QString& action, const QDateTime& timestamp,
        const QString& details, const QString& ipAddress = QString(),
        const QString& deviceInfo = QString());

    // Геттеры
    int getLogId() const { return logId; }
    int getUserId() const { return userId; }
    QString getAction() const { return action; }
    QDateTime getTimestamp() const { return timestamp; }
    QString getDetails() const { return details; }
    QString getIpAddress() const { return ipAddress; }
    QString getDeviceInfo() const { return deviceInfo; }

    // Сеттеры
    void setLogId(int value) { logId = value; }
    void setUserId(int value) { userId = value; }
    void setAction(const QString& value) { action = value; }
    void setTimestamp(const QDateTime& value) { timestamp = value; }
    void setDetails(const QString& value) { details = value; }
    void setIpAddress(const QString& value) { ipAddress = value; }
    void setDeviceInfo(const QString& value) { deviceInfo = value; }

    static QList<Log> getAllLogs();
    static bool addLog(const Log& log);

    static QList<Log> getLogsByDateRange(const QDate& startDate, const QDate& endDate);
    static bool deleteLogs(const QDate& startDate, const QDate& endDate);
    static bool beginTransaction();
    static bool commitTransaction();
    static bool rollbackTransaction();

private:
    int logId;
    int userId;
    QString action;
    QDateTime timestamp;
    QString details;
    QString ipAddress;
    QString deviceInfo;
};

#endif // LOG_H
