#include "Borrowing.h"
#include "../utils/DatabaseManager.h"
#include "UserGame.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

Borrowing::Borrowing() : borrowingId(-1), lenderUserGameId(-1), borrowerId(-1) {}

Borrowing::Borrowing(int borrowingId, int lenderUserGameId, int borrowerId,
                     const QDate& startDate, const QDate& endDate, const QString& status)
    : borrowingId(borrowingId), lenderUserGameId(lenderUserGameId), borrowerId(borrowerId),
    startDate(startDate), endDate(endDate), status(status) {}

// Геттеры и сеттеры
int Borrowing::getBorrowingId() const { return borrowingId; }
void Borrowing::setBorrowingId(int value) { borrowingId = value; }

int Borrowing::getLenderUserGameId() const { return lenderUserGameId; }
void Borrowing::setLenderUserGameId(int value) { lenderUserGameId = value; }

int Borrowing::getBorrowerId() const { return borrowerId; }
void Borrowing::setBorrowerId(int value) { borrowerId = value; }

QDate Borrowing::getStartDate() const { return startDate; }
void Borrowing::setStartDate(const QDate& value) { startDate = value; }

QDate Borrowing::getEndDate() const { return endDate; }
void Borrowing::setEndDate(const QDate& value) { endDate = value; }

QString Borrowing::getStatus() const { return status; }
void Borrowing::setStatus(const QString& value) { status = value; }

// Методы работы с БД
QList<Borrowing> Borrowing::getBorrowingsByUser(int userId) {
    QList<Borrowing> borrowings;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Borrowings WHERE borrower_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения заимствований пользователя:" << query.lastError().text();
        return borrowings;
    }

    while (query.next()) {
        Borrowing borrowing;
        borrowing.setBorrowingId(query.value("borrowing_id").toInt());
        borrowing.setLenderUserGameId(query.value("lender_user_game_id").toInt());
        borrowing.setBorrowerId(query.value("borrower_id").toInt());
        borrowing.setStartDate(query.value("start_date").toDate());
        borrowing.setEndDate(query.value("end_date").toDate());
        borrowing.setStatus(query.value("status").toString());
        borrowings.append(borrowing);
    }

    return borrowings;
}

QList<Borrowing> Borrowing::getBorrowingsForUser(int userId) {
    QList<Borrowing> borrowings;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT b.* FROM Borrowings b "
                  "JOIN UserGames ug ON b.lender_user_game_id = ug.user_game_id "
                  "WHERE ug.user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения заимствований для пользователя:" << query.lastError().text();
        return borrowings;
    }

    while (query.next()) {
        Borrowing borrowing;
        borrowing.setBorrowingId(query.value("borrowing_id").toInt());
        borrowing.setLenderUserGameId(query.value("lender_user_game_id").toInt());
        borrowing.setBorrowerId(query.value("borrower_id").toInt());
        borrowing.setStartDate(query.value("start_date").toDate());
        borrowing.setEndDate(query.value("end_date").toDate());
        borrowing.setStatus(query.value("status").toString());
        borrowings.append(borrowing);
    }

    return borrowings;
}

bool Borrowing::addBorrowing(const Borrowing& borrowing) {
    if (!UserGame::decrementAvailableCopies(borrowing.getLenderUserGameId())) {
        qDebug() << "Не удалось уменьшить количество доступных копий.";
        return false;
    }

    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("INSERT INTO Borrowings (lender_user_game_id, borrower_id, start_date, "
                  "end_date, status) VALUES (:lender_user_game_id, :borrower_id, "
                  ":start_date, :end_date, :status)");
    query.bindValue(":lender_user_game_id", borrowing.getLenderUserGameId());
    query.bindValue(":borrower_id", borrowing.getBorrowerId());
    query.bindValue(":start_date", borrowing.getStartDate());
    query.bindValue(":end_date", borrowing.getEndDate());
    query.bindValue(":status", borrowing.getStatus());

    if (!query.exec()) {
        qDebug() << "Ошибка добавления заимствования:" << query.lastError().text();
        UserGame::incrementAvailableCopies(borrowing.getLenderUserGameId());
        return false;
    }

    return true;
}

bool Borrowing::updateBorrowing(const Borrowing& borrowing) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE Borrowings SET status = :status WHERE borrowing_id = :borrowing_id");
    query.bindValue(":status", borrowing.getStatus());
    query.bindValue(":borrowing_id", borrowing.getBorrowingId());

    if (!query.exec()) {
        qDebug() << "Ошибка обновления заимствования:" << query.lastError().text();
        return false;
    }

    return true;
}

Borrowing Borrowing::getBorrowingById(int borrowingId) {
    Borrowing borrowing;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Borrowings WHERE borrowing_id = :borrowing_id");
    query.bindValue(":borrowing_id", borrowingId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения заимствования по ID:" << query.lastError().text();
        return borrowing;
    }

    if (query.next()) {
        borrowing.setBorrowingId(query.value("borrowing_id").toInt());
        borrowing.setLenderUserGameId(query.value("lender_user_game_id").toInt());
        borrowing.setBorrowerId(query.value("borrower_id").toInt());
        borrowing.setStartDate(query.value("start_date").toDate());
        borrowing.setEndDate(query.value("end_date").toDate());
        borrowing.setStatus(query.value("status").toString());
    }

    return borrowing;
}

QList<Borrowing> Borrowing::getBorrowingsByGameId(int gameId) {
    QList<Borrowing> borrowings;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT b.* FROM Borrowings b "
                  "JOIN UserGames ug ON b.lender_user_game_id = ug.user_game_id "
                  "WHERE ug.game_id = :game_id");
    query.bindValue(":game_id", gameId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения заимствований по gameId:" << query.lastError().text();
        return borrowings;
    }

    while (query.next()) {
        Borrowing borrowing;
        borrowing.setBorrowingId(query.value("borrowing_id").toInt());
        borrowing.setLenderUserGameId(query.value("lender_user_game_id").toInt());
        borrowing.setBorrowerId(query.value("borrower_id").toInt());
        borrowing.setStartDate(query.value("start_date").toDate());
        borrowing.setEndDate(query.value("end_date").toDate());
        borrowing.setStatus(query.value("status").toString());
        borrowings.append(borrowing);
    }

    return borrowings;
}

int Borrowing::getActiveBorrowingsCountForUserGame(int userGameId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT COUNT(*) FROM Borrowings WHERE lender_user_game_id = :userGameId "
                  "AND status = 'confirmed'");
    query.bindValue(":userGameId", userGameId);

    if (!query.exec()) {
        qDebug() << "Ошибка при получении количества активных бронирований:"
                 << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
