#include "UserController.h"
#include <QDebug>

UserController::UserController(QObject *parent) : QObject(parent) {}

User* UserController::getUserById(int userId) {
    return User::getUserById(userId);
}

bool UserController::updateUserProfile(const User& user) {
    if (User::updateUser(user)) {
        return true;
    } else {
        emit errorOccurred("Ошибка при обновлении профиля пользователя.");
        return false;
    }
}
