#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H

#include <QWidget>
#include "../controllers/AuthController.h"

namespace Ui {
class RegistrationWindow;
}

class RegistrationWindow : public QWidget {
    Q_OBJECT

public:
    explicit RegistrationWindow(QWidget *parent = nullptr);
    ~RegistrationWindow();

private slots:
    void onRegisterButtonClicked();
    void onRegistrationSuccess();
    void onRegistrationFailed(const QString& error);

private:
    Ui::RegistrationWindow *ui;
    AuthController *authController;
};

#endif // REGISTRATIONWINDOW_H
