#ifndef GAMEGENRE_H
#define GAMEGENRE_H

#include <QList>

class GameGenre {
public:
    // Конструкторы
    GameGenre();
    GameGenre(int gameId, int genreId);

    // Геттеры и сеттеры
    int getGameId() const;
    void setGameId(int value);

    int getGenreId() const;
    void setGenreId(int value);

    // Методы
    static QList<int> getGenresForGame(int gameId);
    static QList<int> getGamesForGenre(int genreId);
    static bool addGameGenre(int gameId, int genreId);
    static bool removeGameGenre(int gameId, int genreId);
    static bool deleteGenresForGame(int gameId);

private:
    int gameId;
    int genreId;
};

#endif // GAMEGENRE_H
