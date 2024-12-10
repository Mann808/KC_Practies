#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>
#include "../models/Log.h"

class User {
public:
    // Конструкторы
    User();
    User(int userId, const QString& username, const QString& email, const QString& passwordHash, const QString& role, bool isBlocked, const QDateTime& dateJoined);

    // Геттеры и сеттеры
    int getUserId() const;
    void setUserId(int value);

    QString getUsername() const;
    void setUsername(const QString& value);

    QString getEmail() const;
    void setEmail(const QString& value);

    QString getPasswordHash() const;
    void setPasswordHash(const QString& value);

    QString getRole() const;
    void setRole(const QString& value);

    bool getIsBlocked() const;
    void setIsBlocked(bool value);

    QDateTime getDateJoined() const;
    void setDateJoined(const QDateTime& value);

    enum LoginStatus {
        Success,
        IncorrectCredentials,
        UserBlocked
    };

    // Методы
    static bool registerUser(const QString& username, const QString& email, const QString& password);
    static std::pair<User*, LoginStatus> loginUser(const QString& username, const QString& password);
    static User* getUserById(int userId);
    static QList<User*> getAllUsers();
    static QList<User*> searchUsers(const QString& searchTerm);
    static bool updateUser(const User& user);
    static bool blockUser(int userId);
    static bool unblockUser(int userId);
    static bool isUsernameTaken(const QString& username);
    static bool isEmailRegistered(const QString& email);

private:
    int userId;
    QString username;
    QString email;
    QString passwordHash;
    QString role; // 'user' или 'admin'
    bool isBlocked;
    QDateTime dateJoined;
};

#endif // USER_H
