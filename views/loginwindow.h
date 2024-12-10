#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include "../controllers/AuthController.h"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginButtonClicked();
    void onRegisterButtonClicked();
    void onLoginSuccess(User* user);
    void onLoginFailed(const QString& error);

private:
    Ui::LoginWindow *ui;
    AuthController *authController;
};

#endif // LOGINWINDOW_H
