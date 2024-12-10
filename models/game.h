#ifndef GAME_H
#define GAME_H

#include <QString>
#include <QList>

class Game {
public:
    // Конструкторы
    Game();
    Game(int gameId, const QString& title, const QString& description, const QString& publisher, int releaseYear);

    // Геттеры и сеттеры
    int getGameId() const;
    void setGameId(int value);

    QString getTitle() const;
    void setTitle(const QString& value);

    QString getDescription() const;
    void setDescription(const QString& value);

    QString getPublisher() const;
    void setPublisher(const QString& value);

    int getReleaseYear() const;
    void setReleaseYear(int value);

    // Методы
    static QList<Game> getAllGames();
    static QList<Game> searchGames(const QString& searchTerm);
    static Game getGameById(int gameId);
    static bool addGame(const Game& game);
    static bool updateGame(const Game& game);
    static bool deleteGame(int gameId);
    static int getLastInsertedId();

private:
    int gameId;
    QString title;
    QString description;
    QString publisher;
    int releaseYear;
};

#endif // GAME_H
