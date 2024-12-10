#include "ChatMessage.h"
#include "../utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

ChatMessage::ChatMessage() : messageId(-1), senderId(-1), receiverId(-1), isRead(false) {}

ChatMessage::ChatMessage(int messageId, int senderId, int receiverId,
                         const QString& content, const QDateTime& sentAt, bool isRead)
    : messageId(messageId), senderId(senderId), receiverId(receiverId),
    content(content), sentAt(sentAt), isRead(isRead) {}

// Геттеры и сеттеры
int ChatMessage::getMessageId() const { return messageId; }
void ChatMessage::setMessageId(int value) { messageId = value; }

int ChatMessage::getSenderId() const { return senderId; }
void ChatMessage::setSenderId(int value) { senderId = value; }

int ChatMessage::getReceiverId() const { return receiverId; }
void ChatMessage::setReceiverId(int value) { receiverId = value; }

QString ChatMessage::getContent() const { return content; }
void ChatMessage::setContent(const QString& value) { content = value; }

QDateTime ChatMessage::getSentAt() const { return sentAt; }
void ChatMessage::setSentAt(const QDateTime& value) { sentAt = value; }

bool ChatMessage::getIsRead() const { return isRead; }
void ChatMessage::setIsRead(bool value) { isRead = value; }

QList<ChatMessage> ChatMessage::getMessagesBetweenUsers(int userId1, int userId2) {
    QList<ChatMessage> messages;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM ChatMessages WHERE "
                  "(sender_id = :user1 AND receiver_id = :user2) OR "
                  "(sender_id = :user2 AND receiver_id = :user1) "
                  "ORDER BY sent_at ASC");
    query.bindValue(":user1", userId1);
    query.bindValue(":user2", userId2);

    if (!query.exec()) {
        qDebug() << "Ошибка получения сообщений:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        ChatMessage message;
        message.setMessageId(query.value("message_id").toInt());
        message.setSenderId(query.value("sender_id").toInt());
        message.setReceiverId(query.value("receiver_id").toInt());
        message.setContent(query.value("content").toString());
        message.setSentAt(query.value("sent_at").toDateTime());
        message.setIsRead(query.value("is_read").toBool());
        messages.append(message);
    }

    return messages;
}

bool ChatMessage::sendMessage(const ChatMessage& message) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("INSERT INTO ChatMessages (sender_id, receiver_id, content, sent_at, is_read) "
                  "VALUES (:sender_id, :receiver_id, :content, :sent_at, FALSE)");
    query.bindValue(":sender_id", message.getSenderId());
    query.bindValue(":receiver_id", message.getReceiverId());
    query.bindValue(":content", message.getContent());
    query.bindValue(":sent_at", message.getSentAt());

    return query.exec();
}

bool ChatMessage::markAsRead(int messageId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE ChatMessages SET is_read = TRUE WHERE message_id = :message_id");
    query.bindValue(":message_id", messageId);

    return query.exec();
}

bool ChatMessage::markAllAsRead(int userId, int fromUserId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    if (fromUserId > 0) {
        query.prepare("UPDATE ChatMessages SET is_read = TRUE "
                      "WHERE receiver_id = :user_id AND sender_id = :from_user_id");
        query.bindValue(":from_user_id", fromUserId);
    } else {
        query.prepare("UPDATE ChatMessages SET is_read = TRUE "
                      "WHERE receiver_id = :user_id");
    }
    query.bindValue(":user_id", userId);

    return query.exec();
}

QList<ChatMessage> ChatMessage::getUnreadMessagesForUser(int userId) {
    QList<ChatMessage> messages;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM ChatMessages "
                  "WHERE receiver_id = :user_id AND is_read = FALSE "
                  "ORDER BY sent_at DESC");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения непрочитанных сообщений:" << query.lastError().text();
        return messages;
    }

    while (query.next()) {
        ChatMessage message;
        message.setMessageId(query.value("message_id").toInt());
        message.setSenderId(query.value("sender_id").toInt());
        message.setReceiverId(query.value("receiver_id").toInt());
        message.setContent(query.value("content").toString());
        message.setSentAt(query.value("sent_at").toDateTime());
        message.setIsRead(false);
        messages.append(message);
    }

    return messages;
}

int ChatMessage::getUnreadMessagesCount(int userId, int fromUserId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    if (fromUserId > 0) {
        query.prepare("SELECT COUNT(*) FROM ChatMessages "
                      "WHERE receiver_id = :user_id AND sender_id = :from_user_id "
                      "AND is_read = FALSE");
        query.bindValue(":from_user_id", fromUserId);
    } else {
        query.prepare("SELECT COUNT(*) FROM ChatMessages "
                      "WHERE receiver_id = :user_id AND is_read = FALSE");
    }
    query.bindValue(":user_id", userId);

    if (!query.exec() || !query.next()) {
        qDebug() << "Ошибка получения количества непрочитанных сообщений:"
                 << query.lastError().text();
        return 0;
    }

    return query.value(0).toInt();
}

ChatMessage ChatMessage::getMessageById(int messageId) {
    ChatMessage message;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM ChatMessages WHERE message_id = :message_id");
    query.bindValue(":message_id", messageId);

    if (query.exec() && query.next()) {
        message.setMessageId(query.value("message_id").toInt());
        message.setSenderId(query.value("sender_id").toInt());
        message.setReceiverId(query.value("receiver_id").toInt());
        message.setContent(query.value("content").toString());
        message.setSentAt(query.value("sent_at").toDateTime());
        message.setIsRead(query.value("is_read").toBool());
    }

    return message;
}
