#include "DatabaseManager.h"
#include "ConfigManager.h"
#include <QSqlError>
#include <QDebug>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() {
    // Инициализируем параметры подключения из конфигурационного файла
    ConfigManager& config = ConfigManager::instance();

    hostName = config.getValue("db_host", "localhost", "Database");
    port = config.getValue("db_port", 5432, "Database").toInt();
    databaseName = config.getValue("db_name", "game_collection_db", "Database");
    userName = config.getValue("db_user", "postgres", "Database");
    password = config.getValue("db_password", "password", "Database");

    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(hostName);
    db.setPort(port);
    db.setDatabaseName(databaseName);
    db.setUserName(userName);
    db.setPassword(password);

    if (!openConnection()) {
        qDebug() << "Не удалось подключиться к базе данных:" << db.lastError().text();
    } else {
        qDebug() << "Подключение к базе данных успешно установлено.";
    }
}

DatabaseManager::~DatabaseManager() {
    closeConnection();
}

bool DatabaseManager::openConnection() {
    if (!db.open()) {
        qDebug() << "Ошибка при открытии базы данных:" << db.lastError().text();
        return false;
    }
    return true;
}

void DatabaseManager::closeConnection() {
    if (db.isOpen()) {
        db.close();
        qDebug() << "Подключение к базе данных закрыто.";
    }
}

QSqlDatabase DatabaseManager::getDB() const {
    return db;
}
