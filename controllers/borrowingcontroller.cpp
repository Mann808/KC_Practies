#include "BorrowingController.h"
#include <QDebug>

BorrowingController::BorrowingController(QObject *parent) : QObject(parent) {}

bool BorrowingController::requestBorrowing(int lenderUserGameId, int borrowerId,
                                           const QDate& startDate, const QDate& endDate) {
    if (startDate > endDate) {
        emit errorOccurred("Дата начала не может быть позже даты окончания.");
        return false;
    }

    if (startDate < QDate::currentDate()) {
        emit errorOccurred("Дата начала не может быть раньше текущей даты.");
        return false;
    }

    UserGame lenderUserGame = UserGame::getUserGameById(lenderUserGameId);
    if (lenderUserGame.getUserGameId() == -1) {
        emit errorOccurred("Игра не найдена.");
        return false;
    }

    if (lenderUserGame.getAvailableCopies() <= 0) {
        emit errorOccurred("Нет доступных копий для бронирования.");
        return false;
    }

    Borrowing borrowing;
    borrowing.setLenderUserGameId(lenderUserGameId);
    borrowing.setBorrowerId(borrowerId);
    borrowing.setStartDate(startDate);
    borrowing.setEndDate(endDate);
    borrowing.setStatus("requested");

    if (Borrowing::addBorrowing(borrowing)) {
        Log log;
        log.setUserId(borrowerId);
        log.setAction("BorrowingRequest");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Запрос на бронирование игры ID: %1 с %2 по %3")
                           .arg(lenderUserGameId)
                           .arg(startDate.toString("dd.MM.yyyy"))
                           .arg(endDate.toString("dd.MM.yyyy")));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        return true;
    } else {
        emit errorOccurred("Ошибка при создании запроса на бронирование.");
        return false;
    }
}

bool BorrowingController::confirmBorrowing(int borrowingId) {
    Borrowing borrowing = Borrowing::getBorrowingById(borrowingId);
    if (borrowing.getBorrowingId() == -1) {
        emit errorOccurred("Бронирование не найдено.");
        return false;
    }

    if (borrowing.getStatus() != "requested") {
        emit errorOccurred("Бронирование не находится в состоянии ожидания.");
        return false;
    }

    borrowing.setStatus("confirmed");
    if (Borrowing::updateBorrowing(borrowing)) {
        if (borrowing.getStatus() == "confirmed") {
            Log log;
            log.setUserId(borrowing.getLenderUserGameId());
            log.setAction("BorrowingConfirm");
            log.setTimestamp(QDateTime::currentDateTime());
            log.setDetails("Подтверждено бронирование ID: " + QString::number(borrowingId));
            log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
            log.setDeviceInfo(QSysInfo::prettyProductName());
            Log::addLog(log);
        }
        emit borrowingStatusChanged(borrowingId);
        return true;
    } else {
        emit errorOccurred("Ошибка при подтверждении бронирования.");
        return false;
    }
}

bool BorrowingController::declineBorrowing(int borrowingId) {
    Borrowing borrowing = Borrowing::getBorrowingById(borrowingId);
    if (borrowing.getBorrowingId() == -1) {
        emit errorOccurred("Бронирование не найдено.");
        return false;
    }

    if (borrowing.getStatus() != "requested") {
        emit errorOccurred("Бронирование не находится в состоянии ожидания.");
        return false;
    }

    borrowing.setStatus("declined");
    if (Borrowing::updateBorrowing(borrowing)) {
        // Возвращаем доступную копию
        if (!UserGame::incrementAvailableCopies(borrowing.getLenderUserGameId())) {
            emit errorOccurred("Ошибка при возврате доступной копии.");
            return false;
        }
        if (borrowing.getStatus() == "declined") {
            Log log;
            log.setUserId(borrowing.getBorrowerId());
            log.setAction("BorrowingDeclined");
            log.setTimestamp(QDateTime::currentDateTime());
            log.setDetails(QString("Отклонено бронирование ID: %1").arg(borrowingId));
            log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
            log.setDeviceInfo(QSysInfo::prettyProductName());
            Log::addLog(log);

            return true;
        }
    } else {
        emit errorOccurred("Ошибка при отклонении бронирования.");
        return false;
    }
}

bool BorrowingController::returnBorrowing(int borrowingId) {
    Borrowing borrowing = Borrowing::getBorrowingById(borrowingId);
    if (borrowing.getBorrowingId() == -1) {
        emit errorOccurred("Бронирование не найдено.");
        return false;
    }

    if (borrowing.getStatus() != "confirmed") {
        emit errorOccurred("Бронирование не находится в состоянии подтверждено.");
        return false;
    }

    borrowing.setStatus("returned");
    if (Borrowing::updateBorrowing(borrowing)) {
        // Увеличиваем количество доступных копий
        if (!UserGame::incrementAvailableCopies(borrowing.getLenderUserGameId())) {
            emit errorOccurred("Ошибка при возврате доступной копии.");
            return false;
        }
        if (borrowing.getStatus() == "returned") {
            Log log;
            log.setUserId(borrowing.getBorrowerId());
            log.setAction("BorrowingReturned");
            log.setTimestamp(QDateTime::currentDateTime());
            log.setDetails(QString("Возвращено бронирование ID: %1").arg(borrowingId));
            log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
            log.setDeviceInfo(QSysInfo::prettyProductName());
            Log::addLog(log);

            return true;
        }
    } else {
        emit errorOccurred("Ошибка при возврате бронирования.");
        return false;
    }
}

QList<Borrowing> BorrowingController::getBorrowingsByUser(int userId) {
    return Borrowing::getBorrowingsByUser(userId);
}

QList<Borrowing> BorrowingController::getBorrowingsForUser(int userId) {
    return Borrowing::getBorrowingsForUser(userId);
}
