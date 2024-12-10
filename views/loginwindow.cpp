#include "LoginWindow.h"
#include "ui_LoginWindow.h"
#include "RegistrationWindow.h"
#include "MainWindow.h"
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow),
    authController(new AuthController(this))
{
    ui->setupUi(this);
    setWindowTitle("Вход в систему");

    connect(ui->loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterButtonClicked);
    connect(authController, &AuthController::loginSuccess, this, &LoginWindow::onLoginSuccess);
    connect(authController, &AuthController::loginFailed, this, &LoginWindow::onLoginFailed);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::onLoginButtonClicked() {
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    authController->loginUser(username, password);
}

void LoginWindow::onRegisterButtonClicked() {
    RegistrationWindow *regWindow = new RegistrationWindow();
    regWindow->show();
}

void LoginWindow::onLoginSuccess(User* user) {
    MainWindow *mainWindow = new MainWindow(user);
    mainWindow->show();
    this->close();
}

void LoginWindow::onLoginFailed(const QString& error) {
    QMessageBox::warning(this, "Ошибка входа", error);
}
