#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "../models/User.h"
#include "../controllers/GameController.h"
#include "../controllers/UserGameController.h"
#include "../controllers/BorrowingController.h"
#include "../controllers/ChatController.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(User* currentUser, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSearchTextChanged(const QString& text);
    void onGenreFilterChanged(int index);
    void onGameDoubleClicked(const QModelIndex& index);
    void onAddGameButtonClicked();
    void onChatButtonClicked();
    void onBorrowingsButtonClicked();
    void onLogoutButtonClicked();
    void onAdminPanelButtonClicked();

    // Новые слоты для обновления данных
    void onGameChanged(int gameId);
    void onBorrowingChanged(int borrowingId);
    void onChatUpdated(int userId);

private:
    Ui::MainWindow *ui;
    User *currentUser;
    GameController *gameController;
    UserGameController *userGameController;
    BorrowingController *borrowingController;
    ChatController *chatController;
    QStandardItemModel *gamesModel;

    void loadGames();
    void setupUI();
    void filterGames();
    void updateBorrowingsList();
    void updateChatNotifications();

    void updateAllWindows();

    QList<Game> gamesList;
    QList<Genre> genresList;

    QTimer *updateTimer;
    void updateGenresList();

};

#endif // MAINWINDOW_H
