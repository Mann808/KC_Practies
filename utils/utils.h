#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace Utils {
QString hashPassword(const QString& password);
bool isValidEmail(const QString& email);
QString generateRandomString(int length);
}

#endif // UTILS_H
