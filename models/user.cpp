#include "User.h"
#include "../utils/DatabaseManager.h"
#include "../utils/Utils.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

User::User() : userId(-1), isBlocked(false) {}

User::User(int userId, const QString& username, const QString& email, const QString& passwordHash, const QString& role, bool isBlocked, const QDateTime& dateJoined)
    : userId(userId), username(username), email(email), passwordHash(passwordHash), role(role), isBlocked(isBlocked), dateJoined(dateJoined) {}

// Геттеры и сеттеры

int User::getUserId() const {
    return userId;
}

void User::setUserId(int value) {
    userId = value;
}

QString User::getUsername() const {
    return username;
}

void User::setUsername(const QString& value) {
    username = value;
}

QString User::getEmail() const {
    return email;
}

void User::setEmail(const QString& value) {
    email = value;
}

QString User::getPasswordHash() const {
    return passwordHash;
}

void User::setPasswordHash(const QString& value) {
    passwordHash = value;
}

QString User::getRole() const {
    return role;
}

void User::setRole(const QString& value) {
    role = value;
}

bool User::getIsBlocked() const {
    return isBlocked;
}

void User::setIsBlocked(bool value) {
    isBlocked = value;
}

QDateTime User::getDateJoined() const {
    return dateJoined;
}

void User::setDateJoined(const QDateTime& value) {
    dateJoined = value;
}

// Методы

bool User::registerUser(const QString& username, const QString& email, const QString& password) {
    if (isUsernameTaken(username)) {
        qDebug() << "Имя пользователя уже занято.";
        return false;
    }

    if (isEmailRegistered(email)) {
        qDebug() << "Электронная почта уже зарегистрирована.";
        return false;
    }

    QString passwordHash = Utils::hashPassword(password);

    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("INSERT INTO Users (username, email, password_hash, role, is_blocked) "
                  "VALUES (:username, :email, :password_hash, 'user', FALSE)");
    query.bindValue(":username", username);
    query.bindValue(":email", email);
    query.bindValue(":password_hash", passwordHash);

    if (!query.exec()) {
        qDebug() << "Ошибка регистрации пользователя:" << query.lastError().text();
        return false;
    }

    return true;
}

std::pair<User*, User::LoginStatus> User::loginUser(const QString& username, const QString& password) {
    QString passwordHash = Utils::hashPassword(password);

    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Ошибка авторизации пользователя:" << query.lastError().text();
        return std::make_pair(nullptr, IncorrectCredentials);
    }

    if (query.next()) {
        QString storedPasswordHash = query.value("password_hash").toString();
        bool isBlocked = query.value("is_blocked").toBool();

        if (isBlocked) {
            qDebug() << "Пользователь заблокирован.";
            return std::make_pair(nullptr, UserBlocked);
        }

        if (storedPasswordHash != passwordHash) {
            qDebug() << "Неверный пароль.";
            return std::make_pair(nullptr, IncorrectCredentials);
        }

        User* user = new User();
        user->setUserId(query.value("user_id").toInt());
        user->setUsername(query.value("username").toString());
        user->setEmail(query.value("email").toString());
        user->setPasswordHash(storedPasswordHash);
        user->setRole(query.value("role").toString());
        user->setIsBlocked(isBlocked);
        user->setDateJoined(query.value("date_joined").toDateTime());
        return std::make_pair(user, Success);
    } else {
        qDebug() << "Пользователь с указанным именем не найден.";
        return std::make_pair(nullptr, IncorrectCredentials);
    }
}

User* User::getUserById(int userId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Users WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения пользователя по ID:" << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        User* user = new User();
        user->setUserId(query.value("user_id").toInt());
        user->setUsername(query.value("username").toString());
        user->setEmail(query.value("email").toString());
        user->setPasswordHash(query.value("password_hash").toString());
        user->setRole(query.value("role").toString());
        user->setIsBlocked(query.value("is_blocked").toBool());
        user->setDateJoined(query.value("date_joined").toDateTime());
        return user;
    } else {
        qDebug() << "Пользователь с указанным ID не найден.";
        return nullptr;
    }
}

QList<User*> User::getAllUsers() {
    QList<User*> users;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    if (!query.exec("SELECT * FROM Users")) {
        qDebug() << "Ошибка получения списка пользователей:" << query.lastError().text();
        return users;
    }

    while (query.next()) {
        User* user = new User();
        user->setUserId(query.value("user_id").toInt());
        user->setUsername(query.value("username").toString());
        user->setEmail(query.value("email").toString());
        user->setPasswordHash(query.value("password_hash").toString());
        user->setRole(query.value("role").toString());
        user->setIsBlocked(query.value("is_blocked").toBool());
        user->setDateJoined(query.value("date_joined").toDateTime());
        users.append(user);
    }

    return users;
}

QList<User*> User::searchUsers(const QString& searchTerm) {
    QList<User*> users;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Users WHERE username ILIKE :searchTerm OR email ILIKE :searchTerm");
    query.bindValue(":searchTerm", "%" + searchTerm + "%");

    if (!query.exec()) {
        qDebug() << "Ошибка поиска пользователей:" << query.lastError().text();
        return users;
    }

    while (query.next()) {
        User* user = new User();
        user->setUserId(query.value("user_id").toInt());
        user->setUsername(query.value("username").toString());
        user->setEmail(query.value("email").toString());
        user->setPasswordHash(query.value("password_hash").toString());
        user->setRole(query.value("role").toString());
        user->setIsBlocked(query.value("is_blocked").toBool());
        user->setDateJoined(query.value("date_joined").toDateTime());
        users.append(user);
    }

    return users;
}

bool User::updateUser(const User& user) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE Users SET username = :username, email = :email, password_hash = :password_hash, role = :role, is_blocked = :is_blocked WHERE user_id = :user_id");
    query.bindValue(":username", user.getUsername());
    query.bindValue(":email", user.getEmail());
    query.bindValue(":password_hash", user.getPasswordHash());
    query.bindValue(":role", user.getRole());
    query.bindValue(":is_blocked", user.getIsBlocked());
    query.bindValue(":user_id", user.getUserId());

    if (!query.exec()) {
        qDebug() << "Ошибка обновления пользователя:" << query.lastError().text();
        return false;
    }

    return true;
}

bool User::blockUser(int userId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE Users SET is_blocked = TRUE WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка блокировки пользователя:" << query.lastError().text();
        return false;
    }

    return true;
}

bool User::unblockUser(int userId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE Users SET is_blocked = FALSE WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка разблокировки пользователя:" << query.lastError().text();
        return false;
    }

    return true;
}

bool User::isUsernameTaken(const QString& username) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT COUNT(*) FROM Users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Ошибка проверки имени пользователя:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return count > 0;
    }

    return false;
}

bool User::isEmailRegistered(const QString& email) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT COUNT(*) FROM Users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qDebug() << "Ошибка проверки электронной почты:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        int count = query.value(0).toInt();
        return count > 0;
    }

    return false;
}
