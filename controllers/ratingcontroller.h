#ifndef RATINGCONTROLLER_H
#define RATINGCONTROLLER_H

#include <QObject>
#include "../models/Rating.h"

class RatingController : public QObject {
    Q_OBJECT
public:
    explicit RatingController(QObject *parent = nullptr);

    bool addOrUpdateRating(int userId, int gameId, int ratingValue);
    double getAverageRating(int gameId);

    void setCurrentUserId(int userId) { currentUserId = userId; }

private:
    int currentUserId;

signals:
    void errorOccurred(const QString& error);
};

#endif // RATINGCONTROLLER_H
