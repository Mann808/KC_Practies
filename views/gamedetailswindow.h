#ifndef GAMEDETAILSWINDOW_H
#define GAMEDETAILSWINDOW_H

#include <QDialog>
#include "../models/User.h"
#include "../models/Game.h"
#include "../controllers/BorrowingController.h"
#include "../controllers/RatingController.h"
#include "../controllers/GameController.h"
#include "../models/Log.h"

namespace Ui {
class GameDetailsWindow;
}

class GameDetailsWindow : public QDialog {
    Q_OBJECT

public:
    explicit GameDetailsWindow(User* currentUser, const Game& game, QWidget *parent = nullptr);
    ~GameDetailsWindow();

    void onGameUpdated(int gameId);
    void onBorrowingChanged(int borrowingId);

private slots:
    void onBorrowButtonClicked();
    void onRateButtonClicked();
    void onStartDateChanged(const QDate& date);
    void onEndDateChanged(const QDate& date);

private:
    Ui::GameDetailsWindow *ui;
    User *currentUser;
    Game game;
    BorrowingController *borrowingController;
    RatingController *ratingController;
    GameController *gameController;
    QList<UserGame> userGamesList;
    void loadGameDetails();
    void setupConnections();
    void setupDateEdits();

signals:
    void gameUpdated();
};

#endif // GAMEDETAILSWINDOW_H
