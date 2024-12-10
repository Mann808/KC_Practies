#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <QObject>
#include "../models/User.h"

class UserController : public QObject {
    Q_OBJECT
public:
    explicit UserController(QObject *parent = nullptr);

    User* getUserById(int userId);
    bool updateUserProfile(const User& user);

signals:
    void userUpdated(int userId);
    void userBlocked(int userId);
    void userUnblocked(int userId);
    void errorOccurred(const QString& error);
};

#endif // USERCONTROLLER_H
