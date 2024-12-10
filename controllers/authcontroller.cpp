#include "AuthController.h"
#include "../utils/Utils.h"
#include <QDebug>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <QCryptographicHash>

AuthController::AuthController(QObject *parent) : QObject(parent) {}

void AuthController::registerUser(const QString& username, const QString& email, const QString& password) {
    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        emit registrationFailed("Все поля обязательны для заполнения.");
        return;
    }

    if (!Utils::isValidEmail(email)) {
        emit registrationFailed("Некорректный формат электронной почты.");
        return;
    }

    if (User::isUsernameTaken(username)) {
        emit registrationFailed("Имя пользователя уже занято.");
        return;
    }

    if (User::isEmailRegistered(email)) {
        emit registrationFailed("Электронная почта уже зарегистрирована.");
        return;
    }

    if (User::registerUser(username, email, password)) {
        // Логируем успешную регистрацию
        QSqlQuery query;
        query.prepare("SELECT user_id FROM Users WHERE username = :username");
        query.bindValue(":username", username);
        if (query.exec() && query.next()) {
            int userId = query.value("user_id").toInt();

            Log log;
            log.setUserId(userId);
            log.setAction("UserRegistration");
            log.setTimestamp(QDateTime::currentDateTime());
            log.setDetails("Успешная регистрация пользователя: " + username);
            log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
            log.setDeviceInfo(QSysInfo::prettyProductName());
            Log::addLog(log);
        }

        emit registrationSuccess();
    } else {
        emit registrationFailed("Ошибка при регистрации пользователя.");
    }
}

void AuthController::loginUser(const QString& username, const QString& password) {
    if (username.isEmpty() || password.isEmpty()) {
        emit loginFailed("Введите имя пользователя и пароль.");
        return;
    }

    auto loginResult = User::loginUser(username, password);
    User* user = loginResult.first;
    User::LoginStatus status = loginResult.second;

    switch (status) {
    case User::Success:
        {
            Log log;
            log.setUserId(user->getUserId());
            log.setAction("UserLogin");
            log.setTimestamp(QDateTime::currentDateTime());
            log.setDetails("Успешный вход в систему");
            log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
            log.setDeviceInfo(QSysInfo::prettyProductName());
            Log::addLog(log);

            emit loginSuccess(user);
        }
        break;
    case User::IncorrectCredentials:
        emit loginFailed("Неверное имя пользователя или пароль.");
        break;
    case User::UserBlocked:
        emit loginFailed("Ваш аккаунт заблокирован.");
        break;
    }
}

void AuthController::initializeAdmin() {
    // Проверяем, есть ли администратор в базе данных
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE role = 'admin'");

    if (query.exec()) {
        if (query.next()) {
            int adminCount = query.value(0).toInt();
            if (adminCount == 0) {
                // Администратор отсутствует, создаём его
                QString defaultAdminUsername = "admin";
                QString defaultAdminPassword = "admin123"; // Рекомендуется изменить на более безопасный пароль
                QString defaultAdminEmail = "admin@example.com";

                // Хешируем пароль
                QString passwordHash = Utils::hashPassword(defaultAdminPassword);

                QSqlQuery insertQuery;
                insertQuery.prepare("INSERT INTO users (username, email, password_hash, role, is_blocked) "
                                    "VALUES (:username, :email, :password_hash, 'admin', false)");
                insertQuery.bindValue(":username", defaultAdminUsername);
                insertQuery.bindValue(":email", defaultAdminEmail);
                insertQuery.bindValue(":password_hash", passwordHash);

                if (!insertQuery.exec()) {
                    qDebug() << "Ошибка при создании администратора:" << insertQuery.lastError().text();
                } else {
                    qDebug() << "Администратор успешно создан с именем пользователя 'admin' и паролем 'admin123'.";
                }
            }
        }
    } else {
        qDebug() << "Ошибка при проверке наличия администратора:" << query.lastError().text();
    }
}
