#include "Utils.h"
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QRandomGenerator>

namespace Utils {

QString hashPassword(const QString& password) {
    QByteArray hashed = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hashed.toHex());
}

bool isValidEmail(const QString& email) {
    QRegularExpression regex("^[\\w\\.-]+@[\\w\\.-]+\\.\\w+$");
    return regex.match(email).hasMatch();
}

QString generateRandomString(int length) {
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    QString randomString;
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleCharacters.length());
        randomString.append(possibleCharacters.at(index));
    }
    return randomString;
}

} // namespace Utils
