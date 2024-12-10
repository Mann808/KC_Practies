#ifndef ADDGAMEWINDOW_H
#define ADDGAMEWINDOW_H

#include <QDialog>
#include "../models/User.h"
#include "../controllers/UserGameController.h"
#include "../controllers/GameController.h"

namespace Ui {
class AddGameWindow;
}

class AddGameWindow : public QDialog {
    Q_OBJECT

public:
    explicit AddGameWindow(User* currentUser, QWidget *parent = nullptr);
    ~AddGameWindow();

private slots:
    void onExistingGameSelected(int index);
    void onAddGameButtonClicked();

private:
    Ui::AddGameWindow *ui;
    User *currentUser;
    UserGameController *userGameController;
    GameController *gameController;
    QList<Game> existingGames;
    QList<Genre> genresList;
    void setupUI();

signals:
    void gameAdded();  // Добавляем сигнал

};

#endif // ADDGAMEWINDOW_H
