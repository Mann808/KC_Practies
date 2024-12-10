#ifndef USERGAME_H
#define USERGAME_H

#include <QString>
#include <QList>

class UserGame {
public:
    // Конструкторы
    UserGame();
    UserGame(int userGameId, int userId, int gameId, int copies, int availableCopies);

    // Геттеры и сеттеры
    int getUserGameId() const;
    void setUserGameId(int value);

    int getUserId() const;
    void setUserId(int value);

    int getGameId() const;
    void setGameId(int value);

    int getCopies() const;
    void setCopies(int value);

    int getAvailableCopies() const;
    void setAvailableCopies(int value);

    // Методы
    static QList<UserGame> getUserGames(int userId);
    static QList<UserGame> getAllUserGames();
    static UserGame getUserGameById(int userGameId);
    static QList<UserGame> getUserGamesByGameId(int gameId);
    static bool addUserGame(const UserGame& userGame);
    static bool updateUserGame(const UserGame& userGame);
    static bool deleteUserGame(int userGameId);
    static bool decrementAvailableCopies(int userGameId);
    static bool incrementAvailableCopies(int userGameId);

private:
    int userGameId;
    int userId;
    int gameId;
    int copies;
    int availableCopies;
};

#endif // USERGAME_H
