#include "AdminController.h"
#include <QDebug>
#include <QJsonValue>
#include <QIODevice>
#include <QNetworkInterface>
#include <QSysInfo>
#include <QSqlRecord>
#include <QSqlField>

AdminController::AdminController(QObject *parent) : QObject(parent) {}

QList<User*> AdminController::getAllUsers() {
    return User::getAllUsers();
}

bool AdminController::blockUser(int userId) {
    if (User::blockUser(userId)) {
        // Записываем действие в лог
        Log log;
        log.setUserId(currentUserId);
        log.setAction("UserBlocked");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Пользователь %1 был заблокирован администратором").arg(userId));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        return true;
    } else {
        emit errorOccurred("Ошибка при блокировке пользователя.");
        return false;
    }
}

bool AdminController::unblockUser(int userId) {
    if (User::unblockUser(userId)) {
        // Записываем действие в лог
        Log log;
        log.setUserId(currentUserId);
        log.setAction("UnblockUser");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Разблокирован пользователь ID: %1").arg(userId));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        return true;
    } else {
        emit errorOccurred("Ошибка при разблокировке пользователя.");
        return false;
    }
}

QList<Log> AdminController::getAllLogs() {
    return Log::getAllLogs();
}

bool AdminController::addGenre(const QString& genreName) {
    Genre genre;
    genre.setName(genreName);
    if (Genre::addGenre(genre)) {
        Log log;
        log.setUserId(currentUserId);
        log.setAction("AddGenre");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Добавлен новый жанр: %1").arg(genreName));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);
        return true;
    } else {
        emit errorOccurred("Ошибка при добавлении жанра.");
        return false;
    }
}

bool AdminController::updateGenre(const Genre& genre) {
    if (Genre::updateGenre(genre)) {
        Log log;
        log.setUserId(currentUserId);
        log.setAction("UpdateGenre");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Обновлен жанр ID: %1 на: %2").arg(genre.getGenreId()).arg(genre.getName()));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);
        return true;
    } else {
        emit errorOccurred("Ошибка при обновлении жанра.");
        return false;
    }
}

bool AdminController::deleteGenre(int genreId) {
    Genre genre = Genre::getGenreById(genreId);
    if (Genre::deleteGenre(genreId)) {
        Log log;
        log.setUserId(currentUserId);
        log.setAction("DeleteGenre");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Удален жанр: %1").arg(genre.getName()));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);
        return true;
    } else {
        emit errorOccurred("Ошибка при удалении жанра.");
        return false;
    }
}

QList<Genre> AdminController::getAllGenres() {
    return Genre::getAllGenres();
}

QList<Log> AdminController::getLogsByDateRange(const QDate& startDate, const QDate& endDate) {
    return Log::getLogsByDateRange(startDate, endDate);
}

bool AdminController::archiveLogs(const QDate& startDate, const QDate& endDate, const QString& archiveFilePath) {
    // Получаем логи за указанный период
    QList<Log> logs = getLogsByDateRange(startDate, endDate);
    if (logs.isEmpty()) {
        emit errorOccurred("Нет логов за указанный период");
        return false;
    }

    // Создаем JSON массив с логами
    QJsonArray logsArray;
    for (const Log& log : logs) {
        QJsonObject logObject;
        logObject["log_id"] = log.getLogId();
        logObject["user_id"] = log.getUserId();
        logObject["action"] = log.getAction();
        logObject["timestamp"] = log.getTimestamp().toString(Qt::ISODate);
        logObject["details"] = log.getDetails();
        logObject["ip_address"] = log.getIpAddress();
        logObject["device_info"] = log.getDeviceInfo();
        logsArray.append(logObject);
    }

    QJsonDocument jsonDoc(logsArray);
    QByteArray jsonData = jsonDoc.toJson();

    // Генерируем ключ шифрования
    QByteArray key = QCryptographicHash::hash("LogArchiveKey", QCryptographicHash::Sha256);
    QByteArray encryptedData = encryptData(jsonData, key);

    // Сохраняем зашифрованные данные в файл
    QFile file(archiveFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Не удалось открыть файл для записи");
        return false;
    }

    if (file.write(encryptedData) == -1) {
        emit errorOccurred("Ошибка при записи данных в файл");
        file.close();
        return false;
    }
    file.close();

    // После успешной архивации удаляем логи из базы данных
    if (!deleteLogs(startDate, endDate)) {
        emit errorOccurred("Ошибка при удалении архивированных логов из базы данных");
        return false;
    }

    // Логируем действие архивации
    Log archiveLog;
    archiveLog.setUserId(currentUserId);
    archiveLog.setAction("LogsArchived");
    archiveLog.setTimestamp(QDateTime::currentDateTime());
    archiveLog.setDetails(QString("Архивированы логи с %1 по %2").arg(startDate.toString("dd.MM.yyyy")).arg(endDate.toString("dd.MM.yyyy")));
    archiveLog.setIpAddress(QNetworkInterface::allAddresses().first().toString());
    archiveLog.setDeviceInfo(QSysInfo::prettyProductName());
    Log::addLog(archiveLog);

    return true;
}

bool AdminController::importLogs(const QString& archiveFilePath) {
    qDebug() << "Начало импорта логов из файла:" << archiveFilePath;

    QFile file(archiveFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QString error = "Не удалось открыть файл архива: " + file.errorString();
        qDebug() << error;
        emit errorOccurred(error);
        return false;
    }

    QByteArray encryptedData = file.readAll();
    file.close();

    // Генерируем ключ расшифровки
    QByteArray key = QCryptographicHash::hash("LogArchiveKey", QCryptographicHash::Sha256);
    QByteArray decryptedData = decryptData(encryptedData, key);

    QJsonDocument jsonDoc = QJsonDocument::fromJson(decryptedData);
    if (jsonDoc.isNull() || !jsonDoc.isArray()) {
        emit errorOccurred("Некорректный формат архива");
        return false;
    }

    QJsonArray logsArray = jsonDoc.array();

    // Начинаем транзакцию
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        emit errorOccurred("Не удалось начать транзакцию");
        return false;
    }

    try {
        QSqlQuery query(db);
        query.prepare("INSERT INTO Logs (user_id, action, timestamp, details, ip_address, device_info) "
                      "VALUES (:user_id, :action, :timestamp, :details, :ip_address, :device_info)");

        for (const QJsonValue& value : logsArray) {
            QJsonObject logObject = value.toObject();

            query.bindValue(":user_id", logObject["user_id"].toInt());
            query.bindValue(":action", logObject["action"].toString());
            query.bindValue(":timestamp", QDateTime::fromString(logObject["timestamp"].toString(), Qt::ISODate));
            query.bindValue(":details", logObject["details"].toString());
            query.bindValue(":ip_address", logObject["ip_address"].toString());
            query.bindValue(":device_info", logObject["device_info"].toString());

            if (!query.exec()) {
                throw QString("Ошибка при импорте лога: %1").arg(query.lastError().text());
            }
        }

        if (!db.commit()) {
            throw QString("Ошибка при фиксации транзакции: %1").arg(db.lastError().text());
        }

        // Логируем импорт
        Log log;
        log.setUserId(currentUserId);
        log.setAction("LogsImported");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Импортировано %1 логов").arg(logsArray.size()));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        qDebug() << "Импорт логов успешно завершен";
        return true;

    } catch (const QString& error) {
        qDebug() << "Ошибка при импорте логов:" << error;
        db.rollback();
        emit errorOccurred(error);
        return false;
    }
}

bool AdminController::deleteLogs(const QDate& startDate, const QDate& endDate) {
    if (Log::deleteLogs(startDate, endDate)) {
        // Логируем удаление
        Log deleteLog;
        deleteLog.setUserId(currentUserId);
        deleteLog.setAction("LogsDeleted");
        deleteLog.setTimestamp(QDateTime::currentDateTime());
        deleteLog.setDetails(QString("Удалены логи с %1 по %2").arg(startDate.toString("dd.MM.yyyy")).arg(endDate.toString("dd.MM.yyyy")));
        deleteLog.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        deleteLog.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(deleteLog);

        return true;
    }
    return false;
}

QByteArray AdminController::encryptData(const QByteArray& data, const QByteArray& key) {
    QByteArray iv = QCryptographicHash::hash(
        QDateTime::currentDateTime().toString().toUtf8(),
        QCryptographicHash::Md5);

    QByteArray result;
    QDataStream sizeStream(&result, QIODevice::WriteOnly);
    sizeStream << (qint32)iv.size();
    result.append(iv);

    QByteArray encrypted;
    for (int i = 0; i < data.size(); ++i) {
        encrypted.append(data[i] ^ key[i % key.size()] ^ iv[i % iv.size()]);
    }
    result.append(encrypted);

    return result;
}

QByteArray AdminController::decryptData(const QByteArray& encryptedData, const QByteArray& key) {
    if (encryptedData.size() < 8) {
        return QByteArray();
    }

    QDataStream sizeStream(encryptedData);
    qint32 ivSize;
    sizeStream >> ivSize;

    if (ivSize <= 0 || ivSize > encryptedData.size() - 4) {
        return QByteArray();
    }

    QByteArray iv = encryptedData.mid(4, ivSize);
    QByteArray encrypted = encryptedData.mid(4 + ivSize);
    QByteArray decrypted;

    for (int i = 0; i < encrypted.size(); ++i) {
        decrypted.append(encrypted[i] ^ key[i % key.size()] ^ iv[i % iv.size()]);
    }

    return decrypted;
}

bool AdminController::createBackup(const QString& backupFilePath) {
    QSqlDatabase db = QSqlDatabase::database();
    QJsonObject backupData;

    QStringList tables = {"users", "games", "genres", "gamegenres", "usergames",
                          "borrowings", "ratings", "chatmessages", "logs", "databasebackups"};

    for (const QString& table : tables) {
        QJsonArray tableData;
        QString queryStr = QString("SELECT * FROM %1").arg(table);
        QSqlQuery query;

        if (!query.exec(queryStr)) {
            emit errorOccurred(QString("Ошибка при чтении таблицы %1").arg(table));
            return false;
        }

        QSqlRecord record = query.record();
        while (query.next()) {
            QJsonObject rowData;
            for (int i = 0; i < record.count(); i++) {
                QString fieldName = record.fieldName(i);
                QVariant value = query.value(i);

                switch (value.type()) {
                case QVariant::DateTime:
                    rowData[fieldName] = value.toDateTime().toString(Qt::ISODate);
                    break;
                case QVariant::Date:
                    rowData[fieldName] = value.toDate().toString(Qt::ISODate);
                    break;
                case QVariant::Bool:
                    rowData[fieldName] = value.toBool();
                    break;
                case QVariant::Int:
                case QVariant::LongLong:
                    rowData[fieldName] = value.toLongLong();
                    break;
                case QVariant::Double:
                    rowData[fieldName] = value.toDouble();
                    break;
                default:
                    rowData[fieldName] = value.toString();
                }
            }
            tableData.append(rowData);
        }
        backupData[table] = tableData;
    }

    QJsonDocument jsonDoc(backupData);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Compact);
    QByteArray key = QCryptographicHash::hash("DatabaseBackupKey", QCryptographicHash::Sha256);
    QByteArray encryptedData = encryptData(jsonData, key);

    QFile file(backupFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Не удалось открыть файл для записи");
        return false;
    }

    if (file.write(encryptedData) == -1) {
        emit errorOccurred("Ошибка при записи данных в файл");
        file.close();
        return false;
    }
    file.close();

    QSqlQuery backupQuery;
    backupQuery.prepare("INSERT INTO databasebackups (backup_date, backup_file_path, created_by) "
                        "VALUES (NOW(), :file_path, :created_by)");
    backupQuery.bindValue(":file_path", backupFilePath);
    backupQuery.bindValue(":created_by", currentUserId);

    if (!backupQuery.exec()) {
        emit errorOccurred("Ошибка при сохранении информации о бэкапе");
        return false;
    }

    return true;
}

bool AdminController::restoreFromBackup(const QString& backupFilePath) {
    QFile file(backupFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Не удалось открыть файл: " + file.errorString());
        return false;
    }

    QByteArray encryptedData = file.readAll();
    file.close();

    QByteArray key = QCryptographicHash::hash("DatabaseBackupKey", QCryptographicHash::Sha256);
    QByteArray decryptedData = decryptData(encryptedData, key);

    QJsonDocument jsonDoc = QJsonDocument::fromJson(decryptedData);
    if (jsonDoc.isNull()) {
        emit errorOccurred("Некорректный формат данных");
        return false;
    }

    if (!jsonDoc.isObject()) {
        emit errorOccurred("Некорректный формат резервной копии");
        return false;
    }

    QJsonObject backupData = jsonDoc.object();
    QSqlDatabase db = QSqlDatabase::database();

    if (!db.transaction()) {
        emit errorOccurred("Не удалось начать транзакцию");
        return false;
    }

    try {
        // Отключаем внешние ключи
        QSqlQuery constraintsQuery(db);
        if (!constraintsQuery.exec("SET CONSTRAINTS ALL DEFERRED")) {
            throw QString("Ошибка при отключении ограничений");
        }

        // Очищаем все таблицы
        QStringList tables = {"users", "games", "genres", "gamegenres", "usergames",
                              "borrowings", "ratings", "chatmessages", "logs", "databasebackups"};

        for (const QString& table : tables) {
            QString truncateQuery = QString("TRUNCATE TABLE \"%1\" CASCADE").arg(table);
            QSqlQuery clearQuery(db);
            if (!clearQuery.exec(truncateQuery)) {
                throw QString("Ошибка при очистке таблицы %1").arg(table);
            }
        }

        // Восстанавливаем данные
        for (const QString& table : tables) {
            if (!backupData.contains(table)) {
                continue;
            }

            QJsonArray tableData = backupData[table].toArray();
            for (int i = 0; i < tableData.size(); i++) {
                QJsonObject rowData = tableData[i].toObject();
                QStringList fields;
                QStringList placeholders;
                QVariantList values;

                for (auto it = rowData.constBegin(); it != rowData.constEnd(); ++it) {
                    fields << QString("%1").arg(it.key());
                    placeholders << "?";
                    values << it.value().toVariant();
                }

                QString insertQuery = QString("INSERT INTO %1 (%2) VALUES (%3)")
                                          .arg(table)
                                          .arg(fields.join(", "))
                                          .arg(placeholders.join(", "));

                QSqlQuery query(db);
                query.prepare(insertQuery);

                for (const QVariant& value : values) {
                    query.addBindValue(value);
                }

                if (!query.exec()) {
                    throw QString("Ошибка при восстановлении данных в таблицу %1").arg(table);
                }
            }
        }

        if (!db.commit()) {
            throw QString("Ошибка при фиксации изменений");
        }

        // Логируем восстановление
        Log log;
        log.setUserId(currentUserId);
        log.setAction("DatabaseRestore");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails("Восстановление базы данных из резервной копии: " + backupFilePath);
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        return true;

    } catch (const QString& error) {
        db.rollback();
        emit errorOccurred(error);
        return false;
    }
}

QList<AdminController::BackupInfo> AdminController::getBackupsList() {
    QList<BackupInfo> backups;
    QSqlQuery query("SELECT b.backup_id, b.backup_date, b.backup_file_path, u.username "
                    "FROM databasebackups b "
                    "JOIN users u ON b.created_by = u.user_id "
                    "ORDER BY b.backup_date DESC");

    while (query.next()) {
        BackupInfo info;
        info.backupId = query.value("backup_id").toInt();
        info.backupDate = query.value("backup_date").toDateTime();
        info.filePath = query.value("backup_file_path").toString();
        info.createdBy = query.value("username").toString();
        backups.append(info);
    }

    return backups;
}

AdminController::Statistics AdminController::getStatistics() {
    Statistics stats;
    QSqlDatabase db = DatabaseManager::instance().getDB();

    // Игры по жанрам
    QSqlQuery genreQuery(db);
    genreQuery.exec("SELECT g.name, COUNT(gg.game_id) FROM Genres g "
                    "LEFT JOIN GameGenres gg ON g.genre_id = gg.genre_id "
                    "GROUP BY g.name");
    while (genreQuery.next()) {
        stats.gamesPerGenre[genreQuery.value(0).toString()] = genreQuery.value(1).toInt();
    }

    // Бронирования по дням
    QSqlQuery borrowQuery(db);
    borrowQuery.exec("SELECT DATE(start_date), COUNT(*) FROM Borrowings "
                     "WHERE start_date >= CURRENT_DATE - INTERVAL '30 days' "
                     "GROUP BY DATE(start_date)");
    while (borrowQuery.next()) {
        stats.borrowingsPerDay[borrowQuery.value(0).toDate()] = borrowQuery.value(1).toInt();
    }

    // Распределение рейтингов
    QSqlQuery ratingQuery(db);
    ratingQuery.exec("SELECT rating_value, COUNT(*) FROM Ratings GROUP BY rating_value");
    while (ratingQuery.next()) {
        stats.ratingsDistribution[ratingQuery.value(0).toInt()] = borrowQuery.value(1).toInt();
    }

    // Активность пользователей
    QSqlQuery userQuery(db);
    userQuery.exec("SELECT u.username, COUNT(l.log_id) FROM Users u "
                   "LEFT JOIN Logs l ON u.user_id = l.user_id "
                   "GROUP BY u.username");
    while (userQuery.next()) {
        stats.userActivity[userQuery.value(0).toString()] = userQuery.value(1).toInt();
    }

    return stats;
}

AdminController::GenreStatistics AdminController::getGenreStatistics() {
    GenreStatistics stats;
    QSqlDatabase db = DatabaseManager::instance().getDB();

    // Количество игр по жанрам
    QSqlQuery genreQuery(db);
    genreQuery.exec(
        "SELECT g.name, COUNT(DISTINCT gg.game_id) as game_count, "
        "COUNT(DISTINCT b.borrowing_id) as borrowing_count, "
        "AVG(CAST(r.rating_value AS FLOAT)) as avg_rating "
        "FROM Genres g "
        "LEFT JOIN GameGenres gg ON g.genre_id = gg.genre_id "
        "LEFT JOIN Games gm ON gg.game_id = gm.game_id "
        "LEFT JOIN UserGames ug ON gm.game_id = ug.game_id "
        "LEFT JOIN Borrowings b ON ug.user_game_id = b.lender_user_game_id "
        "LEFT JOIN Ratings r ON gm.game_id = r.game_id "
        "GROUP BY g.name"
        );

    while (genreQuery.next()) {
        QString genreName = genreQuery.value(0).toString();
        stats.gamesPerGenre[genreName] = genreQuery.value(1).toInt();
        stats.borrowingsPerGenre[genreName] = genreQuery.value(2).toInt();
        stats.averageRatingPerGenre[genreName] = genreQuery.value(3).toDouble();
    }

    // Общая статистика
    QSqlQuery totalQuery(db);
    totalQuery.exec("SELECT COUNT(DISTINCT g.game_id), COUNT(DISTINCT gr.genre_id) "
                    "FROM Games g, Genres gr");
    if (totalQuery.next()) {
        stats.totalGames = totalQuery.value(0).toInt();
        stats.totalGenres = totalQuery.value(1).toInt();
    }

    return stats;
}

AdminController::BorrowingStatistics AdminController::getBorrowingStatistics() {
    BorrowingStatistics stats;
    QSqlDatabase db = DatabaseManager::instance().getDB();

    // Очищаем все коллекции
    stats.borrowingsPerDay.clear();
    stats.borrowingsPerUser.clear();
    stats.borrowingsPerGame.clear();
    stats.totalBorrowings = 0;
    stats.activeBorrowings = 0;
    stats.averageBorrowingDuration = 0.0;

    // 1. Получаем общую статистику бронирований
    QSqlQuery totalQuery(db);
    totalQuery.prepare(
        "SELECT "
        "COUNT(*) as total_borrowings, "
        "COUNT(CASE WHEN status = 'confirmed' AND end_date >= CURRENT_DATE THEN 1 END) as active_borrowings, "
        "COALESCE(AVG(CASE WHEN status != 'declined' "
        "    THEN DATE_PART('day', end_date::timestamp - start_date::timestamp) "
        "    END), 0) as avg_duration "
        "FROM Borrowings"
        );

    if (totalQuery.exec() && totalQuery.next()) {
        stats.totalBorrowings = totalQuery.value("total_borrowings").toInt();
        stats.activeBorrowings = totalQuery.value("active_borrowings").toInt();
        stats.averageBorrowingDuration = totalQuery.value("avg_duration").toDouble();
    } else {
        qDebug() << "Error getting total borrowing statistics:" << totalQuery.lastError().text();
        return stats;
    }

    // 2. Получаем статистику бронирований по дням
    QSqlQuery dailyQuery(db);
    dailyQuery.prepare(
        "WITH RECURSIVE dates AS ( "
        "  SELECT CURRENT_DATE - INTERVAL '30 days' AS date "
        "  UNION ALL "
        "  SELECT date + INTERVAL '1 day' "
        "  FROM dates "
        "  WHERE date < CURRENT_DATE "
        ") "
        "SELECT dates.date::date as borrow_date, "
        "COUNT(b.borrowing_id) as borrow_count "
        "FROM dates "
        "LEFT JOIN Borrowings b ON DATE(b.start_date) = dates.date "
        "AND b.status != 'declined' "
        "GROUP BY dates.date "
        "ORDER BY dates.date"
        );

    if (dailyQuery.exec()) {
        while (dailyQuery.next()) {
            QDate date = dailyQuery.value("borrow_date").toDate();
            int count = dailyQuery.value("borrow_count").toInt();
            stats.borrowingsPerDay[date] = count;
        }
    } else {
        qDebug() << "Error getting daily borrowing statistics:" << dailyQuery.lastError().text();
    }

    // 3. Получаем статистику бронирований по пользователям
    QSqlQuery userQuery(db);
    userQuery.prepare(
        "SELECT u.username, "
        "COUNT(*) as borrow_count "
        "FROM Users u "
        "JOIN Borrowings b ON u.user_id = b.borrower_id "
        "WHERE b.status != 'declined' "
        "GROUP BY u.username "
        "ORDER BY borrow_count DESC"
        );

    if (userQuery.exec()) {
        while (userQuery.next()) {
            QString username = userQuery.value("username").toString();
            int count = userQuery.value("borrow_count").toInt();
            stats.borrowingsPerUser[username] = count;
        }
    } else {
        qDebug() << "Error getting user borrowing statistics:" << userQuery.lastError().text();
    }

    // 4. Получаем статистику бронирований по играм
    QSqlQuery gameQuery(db);
    gameQuery.prepare(
        "SELECT g.title, "
        "COUNT(*) as borrow_count "
        "FROM Games g "
        "JOIN UserGames ug ON g.game_id = ug.game_id "
        "JOIN Borrowings b ON ug.user_game_id = b.lender_user_game_id "
        "WHERE b.status != 'declined' "
        "GROUP BY g.title "
        "ORDER BY borrow_count DESC"
        );

    if (gameQuery.exec()) {
        while (gameQuery.next()) {
            QString gameTitle = gameQuery.value("title").toString();
            int count = gameQuery.value("borrow_count").toInt();
            stats.borrowingsPerGame[gameTitle] = count;
        }
    } else {
        qDebug() << "Error getting game borrowing statistics:" << gameQuery.lastError().text();
    }

    // 5. Проверка данных
    if (stats.totalBorrowings <= 0) {
        stats.totalBorrowings = 0;
        stats.activeBorrowings = 0;
        stats.averageBorrowingDuration = 0.0;
        stats.borrowingsPerDay.clear();
        stats.borrowingsPerUser.clear();
        stats.borrowingsPerGame.clear();

        QDate currentDate = QDate::currentDate();
        QDate startDate = currentDate.addDays(-30);

        // Заполняем нулевыми значениями для отображения пустого графика
        for (QDate date = startDate; date <= currentDate; date = date.addDays(1)) {
            stats.borrowingsPerDay[date] = 0;
        }
    }

    return stats;
}

AdminController::RatingStatistics AdminController::getRatingStatistics() {
    RatingStatistics stats;
    QSqlDatabase db = DatabaseManager::instance().getDB();

    // Распределение рейтингов
    QSqlQuery ratingDistQuery(db);
    ratingDistQuery.exec(
        "SELECT rating_value, COUNT(*) "
        "FROM Ratings "
        "GROUP BY rating_value "
        "ORDER BY rating_value"
        );

    while (ratingDistQuery.next()) {
        stats.ratingsDistribution[ratingDistQuery.value(0).toInt()] =
            ratingDistQuery.value(1).toInt();
    }

    // Средний рейтинг по играм
    QSqlQuery gameRatingQuery(db);
    gameRatingQuery.exec(
        "SELECT g.title, AVG(CAST(r.rating_value AS FLOAT)) "
        "FROM Games g "
        "JOIN Ratings r ON g.game_id = r.game_id "
        "GROUP BY g.title"
        );

    while (gameRatingQuery.next()) {
        stats.averageRatingPerGame[gameRatingQuery.value(0).toString()] =
            gameRatingQuery.value(1).toDouble();
    }

    // Рейтинги по пользователям
    QSqlQuery userRatingQuery(db);
    userRatingQuery.exec(
        "SELECT u.username, COUNT(*) "
        "FROM Users u "
        "JOIN Ratings r ON u.user_id = r.user_id "
        "GROUP BY u.username"
        );

    while (userRatingQuery.next()) {
        stats.ratingsPerUser[userRatingQuery.value(0).toString()] =
            userRatingQuery.value(1).toInt();
    }

    // Общая статистика
    QSqlQuery statsQuery(db);
    statsQuery.exec(
        "SELECT COUNT(*), AVG(CAST(rating_value AS FLOAT)) "
        "FROM Ratings"
        );

    if (statsQuery.next()) {
        stats.totalRatings = statsQuery.value(0).toInt();
        stats.overallAverageRating = statsQuery.value(1).toDouble();
    }

    return stats;
}

AdminController::UserActivityStatistics AdminController::getUserActivityStatistics() {
    UserActivityStatistics stats;
    QSqlDatabase db = DatabaseManager::instance().getDB();

    // Логины пользователей
    QSqlQuery loginQuery(db);
    loginQuery.exec(
        "SELECT u.username, COUNT(*) "
        "FROM Users u "
        "JOIN Logs l ON u.user_id = l.user_id "
        "WHERE l.action = 'UserLogin' "
        "GROUP BY u.username"
        );

    while (loginQuery.next()) {
        stats.userLoginCount[loginQuery.value(0).toString()] =
            loginQuery.value(1).toInt();
    }

    // Бронирования пользователей
    QSqlQuery borrowQuery(db);
    borrowQuery.exec(
        "SELECT u.username, COUNT(*) "
        "FROM Users u "
        "JOIN Borrowings b ON u.user_id = b.borrower_id "
        "GROUP BY u.username"
        );

    while (borrowQuery.next()) {
        stats.userBorrowingCount[borrowQuery.value(0).toString()] =
            borrowQuery.value(1).toInt();
    }

    // Рейтинги пользователей
    QSqlQuery ratingQuery(db);
    ratingQuery.exec(
        "SELECT u.username, COUNT(*) "
        "FROM Users u "
        "JOIN Ratings r ON u.user_id = r.user_id "
        "GROUP BY u.username"
        );

    while (ratingQuery.next()) {
        stats.userRatingCount[ratingQuery.value(0).toString()] =
            ratingQuery.value(1).toInt();
    }

    // Сообщения чата
    QSqlQuery chatQuery(db);
    chatQuery.exec(
        "SELECT u.username, COUNT(*) "
        "FROM Users u "
        "JOIN ChatMessages c ON u.user_id = c.sender_id "
        "GROUP BY u.username"
        );

    while (chatQuery.next()) {
        stats.userChatMessageCount[chatQuery.value(0).toString()] =
            chatQuery.value(1).toInt();
    }

    // Временная шкала активности
    QSqlQuery timelineQuery(db);
    timelineQuery.exec(
        "SELECT DATE_TRUNC('hour', l.timestamp) as activity_hour, COUNT(*) "
        "FROM Logs l "
        "WHERE l.timestamp >= CURRENT_TIMESTAMP - INTERVAL '24 hours' "
        "GROUP BY activity_hour "
        "ORDER BY activity_hour"
        );

    while (timelineQuery.next()) {
        stats.activityTimeline[timelineQuery.value(0).toDateTime()] =
            timelineQuery.value(1).toInt();
    }

    return stats;
}

QList<AdminController::ActiveBorrowing> AdminController::getActiveBorrowings() {
    QList<ActiveBorrowing> result;
    QSqlQuery query;

    query.exec("SELECT * FROM active_borrowings");

    while (query.next()) {
        ActiveBorrowing borrowing;
        borrowing.borrowingId = query.value("borrowing_id").toInt();
        borrowing.borrowerName = query.value("borrower_name").toString();
        borrowing.lenderName = query.value("lender_name").toString();
        borrowing.gameTitle = query.value("game_title").toString();
        borrowing.startDate = query.value("start_date").toDate();
        borrowing.endDate = query.value("end_date").toDate();
        result.append(borrowing);
    }

    return result;
}

QList<AdminController::UserStatistic> AdminController::getUserStatistics() {
    QList<UserStatistic> result;
    QSqlQuery query;

    query.exec("SELECT * FROM user_statistics");

    while (query.next()) {
        UserStatistic stat;
        stat.username = query.value("username").toString();
        stat.ownedGames = query.value("owned_games").toInt();
        stat.gamesLent = query.value("games_lent").toInt();
        stat.gamesBorrowed = query.value("games_borrowed").toInt();
        stat.ratingsGiven = query.value("ratings_given").toInt();
        stat.averageRating = query.value("average_rating_given").toDouble();
        stat.messagesSent = query.value("messages_sent").toInt();
        stat.messagesReceived = query.value("messages_received").toInt();
        stat.lastActivity = query.value("last_activity").toDateTime();
        result.append(stat);
    }

    return result;
}

bool AdminController::runDatabaseMaintenance() {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    if (!db.transaction()) {
        emit errorOccurred("Не удалось начать транзакцию");
        return false;
    }

    QSqlQuery query(db);
    try {
        // Логируем начало обслуживания
        Log maintenanceLog;
        maintenanceLog.setUserId(currentUserId);
        maintenanceLog.setAction("MaintenanceStarted");
        maintenanceLog.setTimestamp(QDateTime::currentDateTime());
        maintenanceLog.setDetails("Начало обслуживания базы данных");
        Log::addLog(maintenanceLog);

        // Удаление в правильном порядке (от зависимых к независимым)
        QStringList cleanupQueries = {
            // Сначала удаляем записи из зависимых таблиц
            "DELETE FROM Borrowings WHERE end_date < CURRENT_DATE - INTERVAL '30 days'",
            "DELETE FROM ChatMessages WHERE sent_at < CURRENT_DATE - INTERVAL '30 days'",
            "DELETE FROM Ratings WHERE rating_id NOT IN (SELECT DISTINCT rating_id FROM Ratings ORDER BY rating_id DESC LIMIT 1000)",
            // Затем удаляем неиспользуемые записи из связующих таблиц
            "DELETE FROM GameGenres WHERE NOT EXISTS (SELECT 1 FROM Games WHERE Games.game_id = GameGenres.game_id)",
            "DELETE FROM UserGames WHERE NOT EXISTS (SELECT 1 FROM Games WHERE Games.game_id = UserGames.game_id)",
            // В последнюю очередь удаляем из основных таблиц
            "DELETE FROM Games WHERE game_id NOT IN (SELECT DISTINCT game_id FROM UserGames)"
        };

        // Выполняем запросы по очереди
        for (const QString& sql : cleanupQueries) {
            if (!query.exec(sql)) {
                throw QString("Ошибка при выполнении очистки: %1").arg(query.lastError().text());
            }
        }

        // Обновляем статусы бронирований
        if (!query.exec("UPDATE Borrowings SET status = 'returned' "
                        "WHERE status = 'confirmed' AND end_date < CURRENT_DATE")) {
            throw QString("Ошибка при обновлении статусов бронирований");
        }

        // Обновляем доступные копии
        if (!query.exec("UPDATE UserGames ug SET available_copies = copies - "
                        "(SELECT COUNT(*) FROM Borrowings b "
                        "WHERE b.lender_user_game_id = ug.user_game_id "
                        "AND b.status = 'confirmed')")) {
            throw QString("Ошибка при обновлении доступных копий");
        }

        // Архивируем старые логи
        QString archivePath = QString("logs/archive_%1.log")
                                  .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

        // Получаем старые логи
        if (!query.exec("SELECT * FROM Logs WHERE timestamp < CURRENT_DATE - INTERVAL '30 days'")) {
            throw QString("Ошибка при получении старых логов");
        }

        QFile archiveFile(archivePath);
        if (archiveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&archiveFile);
            while (query.next()) {
                out << QString("%1\t%2\t%3\t%4\n")
                .arg(query.value("timestamp").toDateTime().toString(Qt::ISODate))
                    .arg(query.value("user_id").toString())
                    .arg(query.value("action").toString())
                    .arg(query.value("details").toString());
            }
            archiveFile.close();

            // Удаляем заархивированные логи
            if (!query.exec("DELETE FROM Logs WHERE timestamp < CURRENT_DATE - INTERVAL '30 days'")) {
                throw QString("Ошибка при удалении старых логов");
            }
        }

        // Фиксируем транзакцию
        if (!db.commit()) {
            throw QString("Ошибка при фиксации транзакции");
        }

        // Логируем успешное завершение
        maintenanceLog.setAction("MaintenanceCompleted");
        maintenanceLog.setTimestamp(QDateTime::currentDateTime());
        maintenanceLog.setDetails("Обслуживание базы данных успешно завершено");
        Log::addLog(maintenanceLog);

        return true;

    } catch (const QString& error) {
        db.rollback();
        emit errorOccurred(error);

        // Логируем ошибку
        Log errorLog;
        errorLog.setUserId(currentUserId);
        errorLog.setAction("MaintenanceError");
        errorLog.setTimestamp(QDateTime::currentDateTime());
        errorLog.setDetails(error);
        Log::addLog(errorLog);

        return false;
    }
}

bool AdminController::exportStatisticsToCSV(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Не удалось открыть файл для записи");
        return false;
    }

    QTextStream out(&file);

    // Экспорт статистики пользователей
    QSqlQuery query;

    // Статистика пользователей
    out << "=== User Statistics ===\n";
    out << createCSVRow({"Username", "Owned Games", "Games Lent", "Games Borrowed",
                         "Ratings Given", "Avg Rating", "Messages Sent", "Messages Received",
                         "Last Activity"}) << "\n";

    query.exec("SELECT * FROM user_statistics ORDER BY owned_games DESC");
    while (query.next()) {
        QStringList row;
        row << query.value("username").toString()
            << query.value("owned_games").toString()
            << query.value("games_lent").toString()
            << query.value("games_borrowed").toString()
            << query.value("ratings_given").toString()
            << QString::number(query.value("average_rating_given").toDouble(), 'f', 2)
            << query.value("messages_sent").toString()
            << query.value("messages_received").toString()
            << query.value("last_activity").toDateTime().toString("yyyy-MM-dd HH:mm:ss");
        out << createCSVRow(row) << "\n";
    }

    // Популярные игры
    out << "\n=== Popular Games ===\n";
    out << createCSVRow({"Title", "Owners", "Times Borrowed", "Ratings",
                         "Average Rating", "Genres"}) << "\n";

    query.exec("SELECT * FROM popular_games ORDER BY owners_count DESC");
    while (query.next()) {
        QStringList row;
        row << query.value("title").toString()
            << query.value("owners_count").toString()
            << query.value("times_borrowed").toString()
            << query.value("ratings_count").toString()
            << QString::number(query.value("average_rating").toDouble(), 'f', 2)
            << query.value("genres").toString();
        out << createCSVRow(row) << "\n";
    }

    // Статистика активности
    out << "\n=== Activity Statistics ===\n";
    out << createCSVRow({"Date", "Action", "Count", "Unique Users"}) << "\n";

    query.exec("SELECT * FROM activity_analysis ORDER BY activity_date DESC");
    while (query.next()) {
        QStringList row;
        row << query.value("activity_date").toDate().toString("yyyy-MM-dd")
            << query.value("action").toString()
            << query.value("action_count").toString()
            << query.value("unique_users").toString();
        out << createCSVRow(row) << "\n";
    }

    file.close();
    return true;
}

QString AdminController::createCSVRow(const QStringList& fields) {
    QStringList escapedFields;
    for (const QString& field : fields) {
        QString escapedField = field;
        if (field.contains(',') || field.contains('"') || field.contains('\n')) {
            escapedField.replace("\"", "\"\"");
            escapedField = QString("\"%1\"").arg(escapedField);
        }
        escapedFields << escapedField;
    }
    return escapedFields.join(",");
}

QList<AdminController::PopularGame> AdminController::getPopularGames() {
    QList<PopularGame> games;
    QSqlQuery query;

    query.exec("SELECT * FROM popular_games ORDER BY owners_count DESC, average_rating DESC");

    while (query.next()) {
        PopularGame game;
        game.title = query.value("title").toString();
        game.ownersCount = query.value("owners_count").toInt();
        game.timesBorrowed = query.value("times_borrowed").toInt();
        game.ratingsCount = query.value("ratings_count").toInt();
        game.averageRating = query.value("average_rating").toDouble();
        game.genres = query.value("genres").toString();

        games.append(game);
    }

    return games;
}
