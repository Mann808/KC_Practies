#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool openConnection();
    void closeConnection();
    QSqlDatabase getDB() const;

private:
    DatabaseManager();
    ~DatabaseManager();

    // Запрещаем копирование и присваивание
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase db;
    QString hostName;
    int port;
    QString databaseName;
    QString userName;
    QString password;
};

#endif // DATABASEMANAGER_H
