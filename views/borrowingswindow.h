#ifndef BORROWINGSWINDOW_H
#define BORROWINGSWINDOW_H

#include <QDialog>
#include "../models/User.h"
#include "../controllers/BorrowingController.h"
#include "../controllers/GameController.h" // Добавлено

namespace Ui {
class BorrowingsWindow;
}

class BorrowingsWindow : public QDialog {
    Q_OBJECT

public:
    explicit BorrowingsWindow(User* currentUser, QWidget *parent = nullptr);
    ~BorrowingsWindow();

    void onBorrowingChanged(int borrowingId);
    void onBorrowingStatusChanged(int borrowingId);

private slots:
    void onConfirmButtonClicked();
    void onDeclineButtonClicked();
    void onReturnButtonClicked();

private:
    Ui::BorrowingsWindow *ui;
    User *currentUser;
    BorrowingController *borrowingController;
    GameController *gameController; // Добавлено
    QList<Borrowing> myBorrowings;
    QList<Borrowing> requestsToMe;
    void loadBorrowings();
    void setupConnections();
};

#endif // BORROWINGSWINDOW_H
