#include "UserGameController.h"
#include <QDebug>

UserGameController::UserGameController(QObject *parent) : QObject(parent) {}

QList<UserGame> UserGameController::getUserGames(int userId) {
    return UserGame::getUserGames(userId);
}

bool UserGameController::addUserGame(int userId, int gameId, int copies) {
    if (copies <= 0) {
        emit errorOccurred("Количество копий должно быть больше нуля.");
        return false;
    }

    // Проверяем существование игры
    Game game = Game::getGameById(gameId);
    if (game.getGameId() <= 0) {
        emit errorOccurred("Указанная игра не существует.");
        return false;
    }

    UserGame userGame;
    userGame.setUserId(userId);
    userGame.setGameId(gameId);
    userGame.setCopies(copies);
    userGame.setAvailableCopies(copies);

    if (UserGame::addUserGame(userGame)) {
        // Логируем добавление
        Log log;
        log.setUserId(userId);
        log.setAction("AddUserGame");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Добавлена игра ID: %1 в количестве: %2").arg(gameId).arg(copies));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        return true;
    } else {
        emit errorOccurred("Ошибка при добавлении игры в вашу коллекцию.");
        return false;
    }
}

bool UserGameController::updateUserGame(int userGameId, int copies) {
    if (copies <= 0) {
        emit errorOccurred("Количество копий должно быть больше нуля.");
        return false;
    }

    UserGame userGame = UserGame::getUserGameById(userGameId);
    if (userGame.getUserGameId() == -1) {
        emit errorOccurred("Игра не найдена в вашей коллекции.");
        return false;
    }

    int borrowedCopies = userGame.getCopies() - userGame.getAvailableCopies();
    if (copies < borrowedCopies) {
        emit errorOccurred("Вы не можете установить количество копий меньше, чем количество занятых копий.");
        return false;
    }

    userGame.setCopies(copies);
    userGame.setAvailableCopies(copies - borrowedCopies);

    if (UserGame::updateUserGame(userGame)) {
        Log log;
        log.setUserId(userGame.getUserId());
        log.setAction("UpdateUserGame");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Обновлено количество копий игры ID: %1 на: %2").arg(userGame.getGameId()).arg(copies));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);
        return true;
    } else {
        emit errorOccurred("Ошибка при обновлении вашей игры.");
        return false;
    }
}

bool UserGameController::deleteUserGame(int userGameId) {
    UserGame userGame = UserGame::getUserGameById(userGameId);
    if (userGame.getUserGameId() == -1) {
        emit errorOccurred("Игра не найдена в вашей коллекции.");
        return false;
    }

    // Проверяем, есть ли активные бронирования
    QList<Borrowing> borrowings = Borrowing::getBorrowingsByUser(userGame.getUserId());
    for (const Borrowing& borrowing : borrowings) {
        if (borrowing.getLenderUserGameId() == userGameId && borrowing.getStatus() != "returned") {
            emit errorOccurred("Вы не можете удалить игру, пока есть активные бронирования.");
            return false;
        }
    }

    if (UserGame::deleteUserGame(userGameId)) {
        Log log;
        log.setUserId(userGame.getUserId());
        log.setAction("DeleteUserGame");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Удалена игра ID: %1 из коллекции").arg(userGame.getGameId()));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);
        return true;
    } else {
        emit errorOccurred("Ошибка при удалении вашей игры.");
        return false;
    }
}
