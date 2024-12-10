#ifndef USERGAMECONTROLLER_H
#define USERGAMECONTROLLER_H

#include <QObject>
#include "../models/UserGame.h"
#include "../models/Game.h"
#include "../models/borrowing.h"

class UserGameController : public QObject {
    Q_OBJECT
public:
    explicit UserGameController(QObject *parent = nullptr);

    QList<UserGame> getUserGames(int userId);
    bool addUserGame(int userId, int gameId, int copies);
    bool updateUserGame(int userGameId, int copies);
    bool deleteUserGame(int userGameId);

    void setCurrentUserId(int userId) { currentUserId = userId; }

private:
    int currentUserId;

signals:
    void errorOccurred(const QString& error);
};

#endif // USERGAMECONTROLLER_H
