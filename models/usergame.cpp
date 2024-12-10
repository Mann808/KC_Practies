#include "UserGame.h"
#include "../utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

UserGame::UserGame() : userGameId(-1), userId(-1), gameId(-1), copies(0), availableCopies(0) {}

UserGame::UserGame(int userGameId, int userId, int gameId, int copies, int availableCopies)
    : userGameId(userGameId), userId(userId), gameId(gameId), copies(copies), availableCopies(availableCopies) {}

// Геттеры и сеттеры

int UserGame::getUserGameId() const {
    return userGameId;
}

void UserGame::setUserGameId(int value) {
    userGameId = value;
}

int UserGame::getUserId() const {
    return userId;
}

void UserGame::setUserId(int value) {
    userId = value;
}

int UserGame::getGameId() const {
    return gameId;
}

void UserGame::setGameId(int value) {
    gameId = value;
}

int UserGame::getCopies() const {
    return copies;
}

void UserGame::setCopies(int value) {
    copies = value;
}

int UserGame::getAvailableCopies() const {
    return availableCopies;
}

void UserGame::setAvailableCopies(int value) {
    availableCopies = value;
}

// Методы

QList<UserGame> UserGame::getUserGames(int userId) {
    QList<UserGame> userGames;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM UserGames WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения игр пользователя:" << query.lastError().text();
        return userGames;
    }

    while (query.next()) {
        UserGame userGame;
        userGame.setUserGameId(query.value("user_game_id").toInt());
        userGame.setUserId(query.value("user_id").toInt());
        userGame.setGameId(query.value("game_id").toInt());
        userGame.setCopies(query.value("copies").toInt());
        userGame.setAvailableCopies(query.value("available_copies").toInt());
        userGames.append(userGame);
    }

    return userGames;
}

QList<UserGame> UserGame::getAllUserGames() {
    QList<UserGame> userGames;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    if (!query.exec("SELECT * FROM UserGames")) {
        qDebug() << "Ошибка получения всех игр пользователей:" << query.lastError().text();
        return userGames;
    }

    while (query.next()) {
        UserGame userGame;
        userGame.setUserGameId(query.value("user_game_id").toInt());
        userGame.setUserId(query.value("user_id").toInt());
        userGame.setGameId(query.value("game_id").toInt());
        userGame.setCopies(query.value("copies").toInt());
        userGame.setAvailableCopies(query.value("available_copies").toInt());
        userGames.append(userGame);
    }

    return userGames;
}

UserGame UserGame::getUserGameById(int userGameId) {
    UserGame userGame;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM UserGames WHERE user_game_id = :user_game_id");
    query.bindValue(":user_game_id", userGameId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения игры пользователя по ID:" << query.lastError().text();
        return userGame;
    }

    if (query.next()) {
        userGame.setUserGameId(query.value("user_game_id").toInt());
        userGame.setUserId(query.value("user_id").toInt());
        userGame.setGameId(query.value("game_id").toInt());
        userGame.setCopies(query.value("copies").toInt());
        userGame.setAvailableCopies(query.value("available_copies").toInt());
    } else {
        qDebug() << "Игра пользователя с указанным ID не найдена.";
    }

    return userGame;
}

QList<UserGame> UserGame::getUserGamesByGameId(int gameId) {
    QList<UserGame> userGames;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM UserGames WHERE game_id = :game_id");
    query.bindValue(":game_id", gameId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения игр пользователей по ID игры:" << query.lastError().text();
        return userGames;
    }

    while (query.next()) {
        UserGame userGame;
        userGame.setUserGameId(query.value("user_game_id").toInt());
        userGame.setUserId(query.value("user_id").toInt());
        userGame.setGameId(query.value("game_id").toInt());
        userGame.setCopies(query.value("copies").toInt());
        userGame.setAvailableCopies(query.value("available_copies").toInt());
        userGames.append(userGame);
    }

    return userGames;
}

bool UserGame::addUserGame(const UserGame& userGame) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("INSERT INTO UserGames (user_id, game_id, copies, available_copies) "
                  "VALUES (:user_id, :game_id, :copies, :available_copies)");
    query.bindValue(":user_id", userGame.getUserId());
    query.bindValue(":game_id", userGame.getGameId());
    query.bindValue(":copies", userGame.getCopies());
    query.bindValue(":available_copies", userGame.getAvailableCopies());

    if (!query.exec()) {
        qDebug() << "Ошибка добавления игры пользователя:" << query.lastError().text();
        return false;
    }

    return true;
}

bool UserGame::updateUserGame(const UserGame& userGame) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE UserGames SET copies = :copies, available_copies = :available_copies "
                  "WHERE user_game_id = :user_game_id");
    query.bindValue(":copies", userGame.getCopies());
    query.bindValue(":available_copies", userGame.getAvailableCopies());
    query.bindValue(":user_game_id", userGame.getUserGameId());

    if (!query.exec()) {
        qDebug() << "Ошибка обновления игры пользователя:" << query.lastError().text();
        return false;
    }

    return true;
}

bool UserGame::deleteUserGame(int userGameId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("DELETE FROM UserGames WHERE user_game_id = :user_game_id");
    query.bindValue(":user_game_id", userGameId);

    if (!query.exec()) {
        qDebug() << "Ошибка удаления игры пользователя:" << query.lastError().text();
        return false;
    }

    return true;
}

bool UserGame::decrementAvailableCopies(int userGameId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE UserGames SET available_copies = available_copies - 1 "
                  "WHERE user_game_id = :user_game_id AND available_copies > 0");
    query.bindValue(":user_game_id", userGameId);

    if (!query.exec()) {
        qDebug() << "Ошибка уменьшения доступных копий игры пользователя:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "Нет доступных копий для уменьшения.";
        return false;
    }

    return true;
}

bool UserGame::incrementAvailableCopies(int userGameId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE UserGames SET available_copies = available_copies + 1 "
                  "WHERE user_game_id = :user_game_id AND available_copies < copies");
    query.bindValue(":user_game_id", userGameId);

    if (!query.exec()) {
        qDebug() << "Ошибка увеличения доступных копий игры пользователя:" << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "Доступные копии уже максимальны.";
        return false;
    }

    return true;
}
