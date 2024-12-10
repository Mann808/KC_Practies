#ifndef GAMEEDITWINDOW_H
#define GAMEEDITWINDOW_H

#include <QDialog>
#include "../models/Game.h"
#include "../controllers/GameController.h"
#include "../models/User.h"

namespace Ui {
class GameEditWindow;
}

class GameEditWindow : public QDialog {
    Q_OBJECT

public:
    explicit GameEditWindow(User* currentUser, QWidget *parent = nullptr, const Game& game = Game());
    ~GameEditWindow();

signals:
    void gameSaved();

private:
    Ui::GameEditWindow *ui;
    User* currentUser;
    GameController* gameController;
    Game currentGame;
    bool isEditMode;

    void setupConnections();
    void populateFields();
    void loadGenres();

private slots:
    void onSaveClicked();
    void onCancelClicked();
};

#endif // GAMEEDITWINDOW_H
