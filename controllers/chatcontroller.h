#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QObject>
#include "../models/ChatMessage.h"
#include "../models/User.h"

class ChatController : public QObject {
    Q_OBJECT

public:
    explicit ChatController(QObject *parent = nullptr);

    bool sendMessage(const ChatMessage& message);
    QList<ChatMessage> getMessagesBetweenUsers(int userId1, int userId2);
    QList<User*> searchUsers(const QString& searchTerm);
    QList<ChatMessage> getUnreadMessages(int userId);
    int getUnreadMessagesCount(int userId, int fromUserId = -1);
    bool markMessageAsRead(int messageId);
    bool markAllMessagesAsRead(int userId, int fromUserId);
    QDateTime getLastMessageTime(int userId1, int userId2);

    void setCurrentUserId(int userId) { currentUserId = userId; }

signals:
    void messageReceived(int userId);
    void errorOccurred(const QString& error);
    void messageRead(int userId);
    void newMessage(const ChatMessage& message);

private:
    void logMessageAction(const ChatMessage& message, const QString& action);

    int currentUserId;
};

#endif // CHATCONTROLLER_H
