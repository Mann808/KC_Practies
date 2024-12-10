#include "Game.h"
#include "../utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

Game::Game() : gameId(-1), releaseYear(0) {}

Game::Game(int gameId, const QString& title, const QString& description, const QString& publisher, int releaseYear)
    : gameId(gameId), title(title), description(description), publisher(publisher), releaseYear(releaseYear) {}

// Геттеры и сеттеры

int Game::getGameId() const {
    return gameId;
}

void Game::setGameId(int value) {
    gameId = value;
}

QString Game::getTitle() const {
    return title;
}

void Game::setTitle(const QString& value) {
    title = value;
}

QString Game::getDescription() const {
    return description;
}

void Game::setDescription(const QString& value) {
    description = value;
}

QString Game::getPublisher() const {
    return publisher;
}

void Game::setPublisher(const QString& value) {
    publisher = value;
}

int Game::getReleaseYear() const {
    return releaseYear;
}

void Game::setReleaseYear(int value) {
    releaseYear = value;
}

// Методы

QList<Game> Game::getAllGames() {
    QList<Game> games;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    if (!query.exec("SELECT * FROM Games")) {
        qDebug() << "Ошибка получения списка игр:" << query.lastError().text();
        return games;
    }

    while (query.next()) {
        Game game;
        game.setGameId(query.value("game_id").toInt());
        game.setTitle(query.value("title").toString());
        game.setDescription(query.value("description").toString());
        game.setPublisher(query.value("publisher").toString());
        game.setReleaseYear(query.value("release_year").toInt());
        games.append(game);
    }

    return games;
}

QList<Game> Game::searchGames(const QString& searchTerm) {
    QList<Game> games;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Games WHERE title ILIKE :searchTerm OR description ILIKE :searchTerm");
    query.bindValue(":searchTerm", "%" + searchTerm + "%");

    if (!query.exec()) {
        qDebug() << "Ошибка поиска игр:" << query.lastError().text();
        return games;
    }

    while (query.next()) {
        Game game;
        game.setGameId(query.value("game_id").toInt());
        game.setTitle(query.value("title").toString());
        game.setDescription(query.value("description").toString());
        game.setPublisher(query.value("publisher").toString());
        game.setReleaseYear(query.value("release_year").toInt());
        games.append(game);
    }

    return games;
}

Game Game::getGameById(int gameId) {
    Game game;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Games WHERE game_id = :game_id");
    query.bindValue(":game_id", gameId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения игры по ID:" << query.lastError().text();
        return game;
    }

    if (query.next()) {
        game.setGameId(query.value("game_id").toInt());
        game.setTitle(query.value("title").toString());
        game.setDescription(query.value("description").toString());
        game.setPublisher(query.value("publisher").toString());
        game.setReleaseYear(query.value("release_year").toInt());
    } else {
        qDebug() << "Игра с указанным ID не найдена.";
    }

    return game;
}

bool Game::addGame(const Game& game) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("INSERT INTO Games (title, description, publisher, release_year) "
                  "VALUES (:title, :description, :publisher, :release_year)");
    query.bindValue(":title", game.getTitle());
    query.bindValue(":description", game.getDescription());
    query.bindValue(":publisher", game.getPublisher());
    query.bindValue(":release_year", game.getReleaseYear());

    if (!query.exec()) {
        qDebug() << "Ошибка добавления игры:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Game::updateGame(const Game& game) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE Games SET title = :title, description = :description, "
                  "publisher = :publisher, release_year = :release_year "
                  "WHERE game_id = :game_id");
    query.bindValue(":title", game.getTitle());
    query.bindValue(":description", game.getDescription());
    query.bindValue(":publisher", game.getPublisher());
    query.bindValue(":release_year", game.getReleaseYear());
    query.bindValue(":game_id", game.getGameId());

    if (!query.exec()) {
        qDebug() << "Ошибка обновления игры:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Game::deleteGame(int gameId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("DELETE FROM Games WHERE game_id = :game_id");
    query.bindValue(":game_id", gameId);

    if (!query.exec()) {
        qDebug() << "Ошибка удаления игры:" << query.lastError().text();
        return false;
    }

    return true;
}

int Game::getLastInsertedId() {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    // Для PostgreSQL используем RETURNING
    query.prepare("SELECT currval(pg_get_serial_sequence('games', 'game_id'))");

    if (!query.exec()) {
        qDebug() << "Ошибка получения последнего ID:" << query.lastError().text();
        return -1;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return -1;
}
