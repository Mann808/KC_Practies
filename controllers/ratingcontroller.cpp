#include "RatingController.h"
#include <QDebug>

RatingController::RatingController(QObject *parent) : QObject(parent) {}

bool RatingController::addOrUpdateRating(int userId, int gameId, int ratingValue) {
    if (ratingValue < 1 || ratingValue > 5) {
        emit errorOccurred("Рейтинг должен быть от 1 до 5.");
        return false;
    }

    Rating rating;
    rating.setUserId(userId);
    rating.setGameId(gameId);
    rating.setRatingValue(ratingValue);

    if (Rating::addOrUpdateRating(rating)) {
        Log log;
        log.setUserId(userId);
        log.setAction("GameRated");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Оценка %1 для игры ID: %2")
                           .arg(ratingValue)
                           .arg(gameId));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);
        return true;
    } else {
        emit errorOccurred("Ошибка при сохранении рейтинга.");
        return false;
    }
}

double RatingController::getAverageRating(int gameId) {
    return Rating::getAverageRatingForGame(gameId);
}
