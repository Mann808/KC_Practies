#include "ChatController.h"
#include "../models/Log.h"
#include <QDebug>
#include <QSysInfo>

ChatController::ChatController(QObject *parent) : QObject(parent) {}

bool ChatController::sendMessage(const ChatMessage& message) {
    if (message.getContent().isEmpty()) {
        emit errorOccurred("Сообщение не может быть пустым.");
        return false;
    }

    if (ChatMessage::sendMessage(message)) {
        // Логируем отправку сообщения
        Log log;
        log.setUserId(message.getSenderId());
        log.setAction("MessageSent");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails("Отправлено сообщение пользователю ID: " +
                       QString::number(message.getReceiverId()));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        // logMessageAction(message, "MessageSent");
        emit newMessage(message);
        return true;
    } else {
        emit errorOccurred("Ошибка при отправке сообщения.");
        return false;
    }
}

QList<ChatMessage> ChatController::getMessagesBetweenUsers(int userId1, int userId2) {
    QList<ChatMessage> messages = ChatMessage::getMessagesBetweenUsers(userId1, userId2);

    // Помечаем сообщения как прочитанные
    for (const ChatMessage& message : messages) {
        if (message.getReceiverId() == userId1 && !message.getIsRead()) {
            markMessageAsRead(message.getMessageId());
            logMessageAction(message, "MessageRead");
        }
    }

    return messages;
}

QList<User*> ChatController::searchUsers(const QString& searchTerm) {
    return User::searchUsers(searchTerm);
}

QList<ChatMessage> ChatController::getUnreadMessages(int userId) {
    return ChatMessage::getUnreadMessagesForUser(userId);
}

int ChatController::getUnreadMessagesCount(int userId, int fromUserId) {
    return ChatMessage::getUnreadMessagesCount(userId, fromUserId);
}

bool ChatController::markMessageAsRead(int messageId) {
    if (ChatMessage::markAsRead(messageId)) {
        ChatMessage message = ChatMessage::getMessageById(messageId);

        Log log;
        log.setUserId(message.getReceiverId());
        log.setAction("MessageRead");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails("Прочитано сообщение от пользователя ID: " +
                       QString::number(message.getSenderId()));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        // logMessageAction(message, "MessageSent");
        return true;
    }
    return false;
}

bool ChatController::markAllMessagesAsRead(int userId, int fromUserId) {
    if (ChatMessage::markAllAsRead(userId, fromUserId)) {
        Log log;
        log.setUserId(userId);
        log.setAction("MarkAllMessagesAsRead");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Все сообщения от пользователя %1 помечены как прочитанные").arg(fromUserId));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);
        return true;
    }
    return false;
}

void ChatController::logMessageAction(const ChatMessage& message, const QString& action) {
    Log log;
    log.setUserId(message.getReceiverId());
    log.setAction(action);
    log.setTimestamp(QDateTime::currentDateTime());

    QString details;
    if (action == "MessageSent") {
        details = QString("Отправлено сообщение пользователю %1").arg(message.getReceiverId());
    } else if (action == "MessageRead") {
        details = QString("Прочитано сообщение от пользователя %1").arg(message.getSenderId());
    }

    log.setDetails(details);
    log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
    log.setDeviceInfo(QSysInfo::prettyProductName());
    Log::addLog(log);
}

QDateTime ChatController::getLastMessageTime(int userId1, int userId2) {
    QDateTime lastTime;
    QList<ChatMessage> messages = getMessagesBetweenUsers(userId1, userId2);
    if (!messages.isEmpty()) {
        lastTime = messages.last().getSentAt();
    }
    return lastTime;
}
