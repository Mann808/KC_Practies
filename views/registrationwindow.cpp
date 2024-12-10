#include "RegistrationWindow.h"
#include "ui_RegistrationWindow.h"
#include <QMessageBox>

RegistrationWindow::RegistrationWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegistrationWindow),
    authController(new AuthController(this))
{
    ui->setupUi(this);
    setWindowTitle("Регистрация");

    connect(ui->registerButton, &QPushButton::clicked, this, &RegistrationWindow::onRegisterButtonClicked);
    connect(authController, &AuthController::registrationSuccess, this, &RegistrationWindow::onRegistrationSuccess);
    connect(authController, &AuthController::registrationFailed, this, &RegistrationWindow::onRegistrationFailed);
}

RegistrationWindow::~RegistrationWindow()
{
    delete ui;
}

void RegistrationWindow::onRegisterButtonClicked() {
    QString username = ui->usernameLineEdit->text();
    QString email = ui->emailLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString confirmPassword = ui->confirmPasswordLineEdit->text();

    if (password != confirmPassword) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают.");
        return;
    }

    authController->registerUser(username, email, password);
}

void RegistrationWindow::onRegistrationSuccess() {
    QMessageBox::information(this, "Регистрация", "Регистрация прошла успешно.");
    this->close();
}

void RegistrationWindow::onRegistrationFailed(const QString& error) {
    QMessageBox::warning(this, "Ошибка регистрации", error);
}
