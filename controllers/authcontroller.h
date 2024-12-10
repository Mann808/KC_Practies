#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <QObject>
#include "../models/User.h"

class AuthController : public QObject {
    Q_OBJECT
public:
    explicit AuthController(QObject *parent = nullptr);

    void registerUser(const QString& username, const QString& email, const QString& password);
    void loginUser(const QString& username, const QString& password);

    void initializeAdmin();

signals:
    void registrationSuccess();
    void registrationFailed(const QString& error);
    void loginSuccess(User* user);
    void loginFailed(const QString& error);

};

#endif // AUTHCONTROLLER_H
