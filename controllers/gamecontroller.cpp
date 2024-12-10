#include "GameController.h"
#include <QDebug>
#include <QFile>
#include "usergamecontroller.h"

GameController::GameController(QObject *parent) : QObject(parent), currentUserId(-1) {}

QList<Game> GameController::getAllGames() {
    return Game::getAllGames();
}

QList<Game> GameController::searchGames(const QString& searchTerm) {
    return Game::searchGames(searchTerm);
}

QList<Game> GameController::filterGamesByGenre(int genreId) {
    QList<Game> games;
    QList<int> gameIds = GameGenre::getGamesForGenre(genreId);
    for (int gameId : gameIds) {
        games.append(Game::getGameById(gameId));
    }
    return games;
}

Game GameController::getGameById(int gameId) {
    return Game::getGameById(gameId);
}

bool GameController::addGame(const Game& game, const QList<int>& genreIds) {
    QSqlDatabase db = DatabaseManager::instance().getDB();

    // Начинаем транзакцию
    if (!db.transaction()) {
        emit errorOccurred("Не удалось начать транзакцию");
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO Games (title, description, publisher, release_year) "
                  "VALUES (:title, :description, :publisher, :release_year) RETURNING game_id");

    query.bindValue(":title", game.getTitle());
    query.bindValue(":description", game.getDescription());
    query.bindValue(":publisher", game.getPublisher());
    query.bindValue(":release_year", game.getReleaseYear());

    if (!query.exec() || !query.next()) {
        db.rollback();
        emit errorOccurred("Ошибка при добавлении игры: " + query.lastError().text());
        return false;
    }

    int gameId = query.value(0).toInt();

    // Добавляем жанры
    for (int genreId : genreIds) {
        QSqlQuery genreQuery(db);
        genreQuery.prepare("INSERT INTO GameGenres (game_id, genre_id) VALUES (:game_id, :genre_id)");
        genreQuery.bindValue(":game_id", gameId);
        genreQuery.bindValue(":genre_id", genreId);

        if (!genreQuery.exec()) {
            db.rollback();
            emit errorOccurred("Ошибка при добавлении жанра к игре");
            return false;
        }
    }

    if (!db.commit()) {
        db.rollback();
        emit errorOccurred("Ошибка при сохранении изменений");
        return false;
    }

    // Логируем действие
    Log log;
    log.setUserId(currentUserId);
    log.setAction("GameAdded");
    log.setTimestamp(QDateTime::currentDateTime());
    log.setDetails("Добавлена новая игра: " + game.getTitle());
    log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
    log.setDeviceInfo(QSysInfo::prettyProductName());
    Log::addLog(log);

    emit gameAdded(gameId);
    return true;
}

bool GameController::updateGame(const Game& game, const QList<int>& genreIds) {
    if (Game::updateGame(game)) {
        if (!GameGenre::deleteGenresForGame(game.getGameId())) {
            emit errorOccurred("Ошибка при обновлении жанров игры.");
            return false;
        }
        for (int genreId : genreIds) {
            if (!GameGenre::addGameGenre(game.getGameId(), genreId)) {
                emit errorOccurred("Ошибка при добавлении жанра к игре.");
                return false;
            }
        }
        Log log;
        log.setUserId(currentUserId);
        log.setAction("GameUpdated");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails("Обновлена игра: " + game.getTitle());
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        emit gameUpdated(game.getGameId());
        return true;
    } else {
        emit errorOccurred("Ошибка при обновлении игры.");
        return false;
    }
}

bool GameController::deleteGame(int gameId) {
    Game game = Game::getGameById(gameId);
    if (Game::deleteGame(gameId)) {
        if (!GameGenre::deleteGenresForGame(gameId)) {
            emit errorOccurred("Ошибка при удалении жанров игры.");
            return false;
        }
        Log log;
        log.setUserId(currentUserId);
        log.setAction("GameDeleted");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails("Удалена игра: " + game.getTitle());
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        return true;
    } else {
        emit errorOccurred("Ошибка при удалении игры.");
        return false;
    }
}

QList<Genre> GameController::getAllGenres() {
    return Genre::getAllGenres();
}

bool GameController::addGenre(const QString& genreName) {
    Genre genre;
    genre.setName(genreName);
    if (Genre::addGenre(genre)) {
        return true;
    } else {
        emit errorOccurred("Ошибка при добавлении жанра.");
        return false;
    }
}

bool GameController::updateGenre(const Genre& genre) {
    if (Genre::updateGenre(genre)) {
        return true;
    } else {
        emit errorOccurred("Ошибка при обновлении жанра.");
        return false;
    }
}

bool GameController::deleteGenre(int genreId) {
    if (Genre::deleteGenre(genreId)) {
        return true;
    } else {
        emit errorOccurred("Ошибка при удалении жанра.");
        return false;
    }
}

QString GameController::getGameGenresAsString(int gameId) {
    QList<int> genreIds = GameGenre::getGenresForGame(gameId);
    QStringList genreNames;
    for (int genreId : genreIds) {
        Genre genre = Genre::getGenreById(genreId);
        genreNames.append(genre.getName());
    }
    return genreNames.join(", ");
}

double GameController::getGameAverageRating(int gameId) {
    return Rating::getAverageRatingForGame(gameId);
}

QList<int> GameController::getGameGenreIds(int gameId) {
    return GameGenre::getGenresForGame(gameId);
}

int GameController::getNumberOfPlayers(int gameId) {
    QList<UserGame> userGames = UserGame::getUserGamesByGameId(gameId);
    QSet<int> userIds;
    for (const UserGame& userGame : userGames) {
        userIds.insert(userGame.getUserId());
    }
    return userIds.size();
}

int GameController::getTotalPlayers(int gameId) {
    if (gameId <= 0) {
        return 0;
    }

    // Получаем список пользователей, у которых есть эта игра
    QList<UserGame> userGames = UserGame::getUserGamesByGameId(gameId);
    QSet<int> userIds;
    for (const UserGame& userGame : userGames) {
        userIds.insert(userGame.getUserId());
    }

    // Получаем список бронирований с одобренными заявками (status == "confirmed")
    QList<Borrowing> borrowings = Borrowing::getBorrowingsByGameId(gameId);
    for (const Borrowing& borrowing : borrowings) {
        if (borrowing.getStatus() == "confirmed") {
            userIds.insert(borrowing.getBorrowerId());
        }
    }

    return userIds.size();
}

int GameController::getTotalCopies(int gameId) {
    int totalCopies = 0;

    // Получаем все UserGame для данной игры
    QList<UserGame> userGames = UserGame::getUserGamesByGameId(gameId);
    for (const UserGame& userGame : userGames) {
        // Количество доступных копий плюс количество занятых копий
        int borrowedCopies = Borrowing::getActiveBorrowingsCountForUserGame(userGame.getUserGameId());
        totalCopies += userGame.getAvailableCopies() + borrowedCopies;
    }

    return totalCopies;
}

bool GameController::exportToCSV(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Не удалось открыть файл для записи");
        return false;
    }

    QTextStream out(&file);

    // Записываем заголовки
    out << createCSVRow({"game_id", "title", "description", "publisher", "release_year", "genres", "average_rating"}) << "\n";

    // Получаем все игры
    QList<Game> games = getAllGames();

    for (const Game& game : games) {
        QStringList row;
        row << QString::number(game.getGameId())
            << game.getTitle()
            << game.getDescription()
            << game.getPublisher()
            << QString::number(game.getReleaseYear())
            << getGameGenresAsString(game.getGameId())
            << QString::number(getGameAverageRating(game.getGameId()), 'f', 2);

        out << createCSVRow(row) << "\n";
    }

    file.close();
    return true;
}

bool GameController::importFromCSV(const QString& filename, bool replaceExisting) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred("Не удалось открыть файл для чтения");
        return false;
    }

    QTextStream in(&file);

    // Пропускаем заголовки
    if (!in.atEnd()) in.readLine();

    // Начинаем транзакцию
    QSqlDatabase db = DatabaseManager::instance().getDB();
    if (!db.transaction()) {
        emit errorOccurred("Не удалось начать транзакцию");
        return false;
    }

    try {
        QSqlQuery query(db);

        // Получаем существующие жанры
        QMap<QString, int> existingGenres;
        query.exec("SELECT genre_id, name FROM Genres");
        while (query.next()) {
            existingGenres[query.value("name").toString()] = query.value("genre_id").toInt();
        }

        // Получаем существующие игры
        QMap<QString, int> existingGames;
        query.exec("SELECT game_id, title FROM Games");
        while (query.next()) {
            existingGames[query.value("title").toString()] = query.value("game_id").toInt();
        }

        while (!in.atEnd()) {
            QStringList fields = parseCSVRow(in.readLine());
            if (fields.size() < 6) continue;

            QString title = fields[1];
            int gameId = -1;

            if (existingGames.contains(title)) {
                if (!replaceExisting) {
                    qDebug() << "Пропуск существующей игры:" << title;
                    continue;
                }

                // Обновляем существующую игру
                gameId = existingGames[title];
                query.prepare("UPDATE Games SET description = :description, "
                              "publisher = :publisher, release_year = :release_year "
                              "WHERE game_id = :game_id");
                query.bindValue(":game_id", gameId);
            } else {
                // Добавляем новую игру
                query.prepare("INSERT INTO Games (title, description, publisher, release_year) "
                              "VALUES (:title, :description, :publisher, :release_year) "
                              "RETURNING game_id");
                query.bindValue(":title", title);
            }

            query.bindValue(":description", fields[2]);
            query.bindValue(":publisher", fields[3]);
            query.bindValue(":release_year", fields[4]);

            if (!query.exec() || (!gameId && !query.next())) {
                throw QString("Ошибка при %1 игры: %2")
                    .arg(gameId > 0 ? "обновлении" : "добавлении")
                    .arg(query.lastError().text());
            }

            if (gameId == -1) {
                gameId = query.value(0).toInt();
            }

            // Обновляем жанры
            if (!fields[5].isEmpty()) {
                // Удаляем старые связи с жанрами
                query.prepare("DELETE FROM GameGenres WHERE game_id = :game_id");
                query.bindValue(":game_id", gameId);
                query.exec();

                // Добавляем новые связи
                QStringList genres = fields[5].split(",", Qt::SkipEmptyParts);
                for (QString& genre : genres) {
                    genre = genre.trimmed();
                    if (!existingGenres.contains(genre)) continue;

                    query.prepare("INSERT INTO GameGenres (game_id, genre_id) "
                                  "VALUES (:game_id, :genre_id)");
                    query.bindValue(":game_id", gameId);
                    query.bindValue(":genre_id", existingGenres[genre]);

                    if (!query.exec()) {
                        qDebug() << "Ошибка при добавлении связи с жанром:" << genre;
                    }
                }
            }
        }

        if (!db.commit()) {
            throw QString("Ошибка при фиксации транзакции");
        }

    } catch (const QString& error) {
        db.rollback();
        emit errorOccurred(error);
        return false;
    }

    file.close();
    return true;
}

QString GameController::createCSVRow(const QStringList& fields) {
    QStringList escapedFields;
    for (const QString& field : fields) {
        QString escapedField = field;
        if (field.contains(',') || field.contains('"') || field.contains('\n')) {
            escapedField.replace("\"", "\"\"");
            escapedField = "\"" + escapedField + "\"";
        }
        escapedFields << escapedField;
    }
    return escapedFields.join(",");
}

QStringList GameController::parseCSVRow(const QString& row) {
    QStringList fields;
    QString field;
    bool inQuotes = false;

    for (int i = 0; i < row.length(); ++i) {
        if (row[i] == '"') {
            if (i + 1 < row.length() && row[i + 1] == '"') {
                field += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (row[i] == ',' && !inQuotes) {
            fields << field;
            field.clear();
        } else {
            field += row[i];
        }
    }
    fields << field;

    return fields;
}

bool GameController::exportBorrowingsToCSV(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred("Не удалось открыть файл для записи");
        return false;
    }

    QTextStream out(&file);

    // Заголовки
    out << createCSVRow({"borrowing_id", "borrower", "lender", "game", "start_date", "end_date", "status"}) << "\n";

    QSqlQuery query;
    query.exec("SELECT b.borrowing_id, u1.username as borrower, u2.username as lender, "
               "g.title as game, b.start_date, b.end_date, b.status "
               "FROM Borrowings b "
               "JOIN UserGames ug ON b.lender_user_game_id = ug.user_game_id "
               "JOIN Users u1 ON b.borrower_id = u1.user_id "
               "JOIN Users u2 ON ug.user_id = u2.user_id "
               "JOIN Games g ON ug.game_id = g.game_id");

    while (query.next()) {
        QStringList row;
        row << query.value("borrowing_id").toString()
            << query.value("borrower").toString()
            << query.value("lender").toString()
            << query.value("game").toString()
            << query.value("start_date").toDate().toString(Qt::ISODate)
            << query.value("end_date").toDate().toString(Qt::ISODate)
            << query.value("status").toString();

        out << createCSVRow(row) << "\n";
    }

    file.close();
    return true;
}

bool GameController::importBorrowingsFromCSV(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred("Не удалось открыть файл для чтения");
        return false;
    }

    QTextStream in(&file);
    in.readLine(); // Пропускаем заголовки

    QSqlDatabase db = DatabaseManager::instance().getDB();
    if (!db.transaction()) {
        emit errorOccurred("Не удалось начать транзакцию");
        return false;
    }

    try {
        while (!in.atEnd()) {
            QStringList fields = parseCSVRow(in.readLine());
            if (fields.size() < 7) continue;

            QSqlQuery query(db);
            query.prepare("INSERT INTO Borrowings (borrower_id, lender_user_game_id, "
                          "start_date, end_date, status) "
                          "SELECT u1.user_id, ug.user_game_id, :start_date, :end_date, :status "
                          "FROM Users u1, Users u2, Games g, UserGames ug "
                          "WHERE u1.username = :borrower AND u2.username = :lender "
                          "AND g.title = :game AND ug.user_id = u2.user_id "
                          "AND ug.game_id = g.game_id");

            query.bindValue(":borrower", fields[1]);
            query.bindValue(":lender", fields[2]);
            query.bindValue(":game", fields[3]);
            query.bindValue(":start_date", QDate::fromString(fields[4], Qt::ISODate));
            query.bindValue(":end_date", QDate::fromString(fields[5], Qt::ISODate));
            query.bindValue(":status", fields[6]);

            if (!query.exec()) {
                throw QString("Ошибка при импорте бронирования: %1").arg(query.lastError().text());
            }
        }

        if (!db.commit()) {
            throw QString("Ошибка при фиксации транзакции");
        }

    } catch (const QString& error) {
        db.rollback();
        emit errorOccurred(error);
        return false;
    }

    file.close();
    return true;
}
