#ifndef BORROWING_H
#define BORROWING_H

#include <QDate>
#include <QList>
#include "../models/Log.h"

class Borrowing {
public:
    // Конструкторы
    Borrowing();
    Borrowing(int borrowingId, int lenderUserGameId, int borrowerId, const QDate& startDate, const QDate& endDate, const QString& status);

    // Геттеры и сеттеры
    int getBorrowingId() const;
    void setBorrowingId(int value);

    int getLenderUserGameId() const;
    void setLenderUserGameId(int value);

    int getBorrowerId() const;
    void setBorrowerId(int value);

    QDate getStartDate() const;
    void setStartDate(const QDate& value);

    QDate getEndDate() const;
    void setEndDate(const QDate& value);

    QString getStatus() const;
    void setStatus(const QString& value);

    // Методы
    static QList<Borrowing> getBorrowingsByUser(int userId);
    static QList<Borrowing> getBorrowingsForUser(int userId);
    static Borrowing getBorrowingById(int borrowingId);
    static bool addBorrowing(const Borrowing& borrowing);
    static bool updateBorrowing(const Borrowing& borrowing);
    static bool deleteBorrowing(int borrowingId);
    static QList<Borrowing> getBorrowingsByGameId(int gameId);
    static int getActiveBorrowingsCountForUserGame(int userGameId);

private:
    int borrowingId;
    int lenderUserGameId;
    int borrowerId;
    QDate startDate;
    QDate endDate;
    QString status; // 'requested', 'confirmed', 'returned', 'declined'
};

#endif // BORROWING_H
