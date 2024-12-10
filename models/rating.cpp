#include "Rating.h"
#include "../utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

Rating::Rating() : ratingId(-1), userId(-1), gameId(-1), ratingValue(0) {}

Rating::Rating(int ratingId, int userId, int gameId, int ratingValue)
    : ratingId(ratingId), userId(userId), gameId(gameId), ratingValue(ratingValue) {}

// Геттеры и сеттеры

int Rating::getRatingId() const {
    return ratingId;
}

void Rating::setRatingId(int value) {
    ratingId = value;
}

int Rating::getUserId() const {
    return userId;
}

void Rating::setUserId(int value) {
    userId = value;
}

int Rating::getGameId() const {
    return gameId;
}

void Rating::setGameId(int value) {
    gameId = value;
}

int Rating::getRatingValue() const {
    return ratingValue;
}

void Rating::setRatingValue(int value) {
    ratingValue = value;
}

// Методы

QList<Rating> Rating::getRatingsForGame(int gameId) {
    QList<Rating> ratings;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Ratings WHERE game_id = :game_id");
    query.bindValue(":game_id", gameId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения рейтингов для игры:" << query.lastError().text();
        return ratings;
    }

    while (query.next()) {
        Rating rating;
        rating.setRatingId(query.value("rating_id").toInt());
        rating.setUserId(query.value("user_id").toInt());
        rating.setGameId(query.value("game_id").toInt());
        rating.setRatingValue(query.value("rating_value").toInt());
        ratings.append(rating);
    }

    return ratings;
}

double Rating::getAverageRatingForGame(int gameId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT AVG(rating_value) FROM Ratings WHERE game_id = :game_id");
    query.bindValue(":game_id", gameId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения среднего рейтинга для игры:" << query.lastError().text();
        return 0.0;
    }

    if (query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

bool Rating::addOrUpdateRating(const Rating& rating) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    // Проверяем, есть ли уже рейтинг от этого пользователя для этой игры
    query.prepare("SELECT rating_id FROM Ratings WHERE user_id = :user_id AND game_id = :game_id");
    query.bindValue(":user_id", rating.getUserId());
    query.bindValue(":game_id", rating.getGameId());

    if (!query.exec()) {
        qDebug() << "Ошибка проверки существующего рейтинга:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        // Обновляем существующий рейтинг
        int existingRatingId = query.value("rating_id").toInt();
        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE Ratings SET rating_value = :rating_value WHERE rating_id = :rating_id");
        updateQuery.bindValue(":rating_value", rating.getRatingValue());
        updateQuery.bindValue(":rating_id", existingRatingId);

        if (!updateQuery.exec()) {
            qDebug() << "Ошибка обновления рейтинга:" << updateQuery.lastError().text();
            return false;
        }
    } else {
        // Добавляем новый рейтинг
        QSqlQuery insertQuery(db);
        insertQuery.prepare("INSERT INTO Ratings (user_id, game_id, rating_value) "
                            "VALUES (:user_id, :game_id, :rating_value)");
        insertQuery.bindValue(":user_id", rating.getUserId());
        insertQuery.bindValue(":game_id", rating.getGameId());
        insertQuery.bindValue(":rating_value", rating.getRatingValue());

        if (!insertQuery.exec()) {
            qDebug() << "Ошибка добавления рейтинга:" << insertQuery.lastError().text();
            return false;
        }
    }

    return true;
}
