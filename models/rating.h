#ifndef RATING_H
#define RATING_H

#include <qlist.h>
#include "../models/Log.h"

class Rating {
public:
    // Конструкторы
    Rating();
    Rating(int ratingId, int userId, int gameId, int ratingValue);

    // Геттеры и сеттеры
    int getRatingId() const;
    void setRatingId(int value);

    int getUserId() const;
    void setUserId(int value);

    int getGameId() const;
    void setGameId(int value);

    int getRatingValue() const;
    void setRatingValue(int value);

    // Методы
    static QList<Rating> getRatingsForGame(int gameId);
    static double getAverageRatingForGame(int gameId);
    static bool addOrUpdateRating(const Rating& rating);

private:
    int ratingId;
    int userId;
    int gameId;
    int ratingValue;
};

#endif // RATING_H
