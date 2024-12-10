#include "GameGenre.h"
#include "../utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

GameGenre::GameGenre() : gameId(-1), genreId(-1) {}

GameGenre::GameGenre(int gameId, int genreId)
    : gameId(gameId), genreId(genreId) {}

// Геттеры и сеттеры

int GameGenre::getGameId() const {
    return gameId;
}

void GameGenre::setGameId(int value) {
    gameId = value;
}

int GameGenre::getGenreId() const {
    return genreId;
}

void GameGenre::setGenreId(int value) {
    genreId = value;
}

// Методы

QList<int> GameGenre::getGenresForGame(int gameId) {
    QList<int> genreIds;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT genre_id FROM GameGenres WHERE game_id = :game_id");
    query.bindValue(":game_id", gameId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения жанров для игры:" << query.lastError().text();
        return genreIds;
    }

    while (query.next()) {
        genreIds.append(query.value("genre_id").toInt());
    }

    return genreIds;
}

QList<int> GameGenre::getGamesForGenre(int genreId) {
    QList<int> gameIds;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT game_id FROM GameGenres WHERE genre_id = :genre_id");
    query.bindValue(":genre_id", genreId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения игр для жанра:" << query.lastError().text();
        return gameIds;
    }

    while (query.next()) {
        gameIds.append(query.value("game_id").toInt());
    }

    return gameIds;
}

bool GameGenre::addGameGenre(int gameId, int genreId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("INSERT INTO GameGenres (game_id, genre_id) VALUES (:game_id, :genre_id)");
    query.bindValue(":game_id", gameId);
    query.bindValue(":genre_id", genreId);

    if (!query.exec()) {
        qDebug() << "Ошибка добавления связи игра-жанр:" << query.lastError().text();
        return false;
    }

    return true;
}

bool GameGenre::removeGameGenre(int gameId, int genreId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("DELETE FROM GameGenres WHERE game_id = :game_id AND genre_id = :genre_id");
    query.bindValue(":game_id", gameId);
    query.bindValue(":genre_id", genreId);

    if (!query.exec()) {
        qDebug() << "Ошибка удаления связи игра-жанр:" << query.lastError().text();
        return false;
    }

    return true;
}

bool GameGenre::deleteGenresForGame(int gameId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("DELETE FROM GameGenres WHERE game_id = :game_id");
    query.bindValue(":game_id", gameId);

    if (!query.exec()) {
        qDebug() << "Ошибка удаления жанров для игры:" << query.lastError().text();
        return false;
    }

    return true;
}
