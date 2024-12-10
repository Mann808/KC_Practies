#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QString>
#include <QDateTime>
#include <QList>
#include "../models/Log.h"

class ChatMessage {
public:
    ChatMessage();
    ChatMessage(int messageId, int senderId, int receiverId, const QString& content,
                const QDateTime& sentAt, bool isRead = false);

    // Геттеры и сеттеры
    int getMessageId() const;
    void setMessageId(int value);

    int getSenderId() const;
    void setSenderId(int value);

    int getReceiverId() const;
    void setReceiverId(int value);

    QString getContent() const;
    void setContent(const QString& value);

    QDateTime getSentAt() const;
    void setSentAt(const QDateTime& value);

    bool getIsRead() const;
    void setIsRead(bool value);

    // Статические методы для работы с БД
    static QList<ChatMessage> getMessagesBetweenUsers(int userId1, int userId2);
    static bool sendMessage(const ChatMessage& message);
    static bool markAsRead(int messageId);
    static bool markAllAsRead(int userId, int fromUserId);
    static QList<ChatMessage> getUnreadMessagesForUser(int userId);
    static int getUnreadMessagesCount(int userId, int fromUserId = -1);
    static ChatMessage getMessageById(int messageId);

private:
    int messageId;
    int senderId;
    int receiverId;
    QString content;
    QDateTime sentAt;
    bool isRead;
};

#endif // CHATMESSAGE_H
