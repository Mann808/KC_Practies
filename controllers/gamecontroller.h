#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include "../models/Game.h"
#include "../models/Genre.h"
#include "../models/GameGenre.h"
#include "../models/Rating.h"

class GameController : public QObject {
    Q_OBJECT
public:
    explicit GameController(QObject *parent = nullptr);

    QList<Game> getAllGames();
    QList<Game> searchGames(const QString& searchTerm);
    QList<Game> filterGamesByGenre(int genreId);
    Game getGameById(int gameId);
    bool addGame(const Game& game, const QList<int>& genreIds);
    bool updateGame(const Game& game, const QList<int>& genreIds);
    bool deleteGame(int gameId);

    QList<Genre> getAllGenres();
    bool addGenre(const QString& genreName);
    bool updateGenre(const Genre& genre);
    bool deleteGenre(int genreId);

    QString getGameGenresAsString(int gameId);
    double getGameAverageRating(int gameId);

    int getNumberOfPlayers(int gameId);
    QList<int> getGameGenreIds(int gameId);
    int getTotalPlayers(int gameId);
    int getTotalCopies(int gameId);

    void setCurrentUserId(int userId) { currentUserId = userId; }

    bool exportToCSV(const QString& filename);
    bool importFromCSV(const QString& filename, bool replaceExisting = false);  // Значение по умолчанию здесь
    bool exportBorrowingsToCSV(const QString& filename);
    bool importBorrowingsFromCSV(const QString& filename);

signals:
    void errorOccurred(const QString& error);
    void gameUpdated(int gameId);
    void gameAdded(int gameId);
    void gameDeleted(int gameId);
    void ratingUpdated(int gameId);

private:
    int currentUserId;

    QString createCSVRow(const QStringList& fields);
    QStringList parseCSVRow(const QString& row);
};

#endif // GAMECONTROLLER_H
