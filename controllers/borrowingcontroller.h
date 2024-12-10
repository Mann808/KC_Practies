#ifndef BORROWINGCONTROLLER_H
#define BORROWINGCONTROLLER_H

#include <QObject>
#include "../models/borrowing.h"
#include "../models/usergame.h"

class BorrowingController : public QObject {
    Q_OBJECT
public:
    explicit BorrowingController(QObject *parent = nullptr);

    bool requestBorrowing(int lenderUserGameId, int borrowerId, const QDate& startDate, const QDate& endDate);
    bool confirmBorrowing(int borrowingId);
    bool declineBorrowing(int borrowingId);
    bool returnBorrowing(int borrowingId);
    QList<Borrowing> getBorrowingsByUser(int userId); // Заимствования, где пользователь является заемщиком
    QList<Borrowing> getBorrowingsForUser(int userId); // Заимствования, где пользователь является владельцем игры
    void setCurrentUserId(int userId) { currentUserId = userId; }

private:
    int currentUserId;

signals:
    void borrowingStatusChanged(int borrowingId);
    void borrowingRequested(int borrowingId);
    void borrowingReturned(int borrowingId);
    void errorOccurred(const QString& error);
};

#endif // BORROWINGCONTROLLER_H
