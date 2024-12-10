#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QDialog>
#include <QTimer>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include "../models/User.h"
#include "../controllers/ChatController.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QDialog {
    Q_OBJECT

public:
    explicit ChatWindow(User* currentUser, QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void onUserSelected(const QModelIndex& index);
    void onSendMessageButtonClicked();
    void onSearchTextChanged(const QString& text);
    void onNewMessage(const ChatMessage& message);
    void checkNewMessages();

private:
    Ui::ChatWindow *ui;
    User *currentUser;
    ChatController *chatController;
    QList<User*> contactsList;
    User *selectedUser;
    QTimer *messageCheckTimer;

    void loadContacts();
    void updateContactsList();
    void loadMessages();
    void setupConnections();
    QStandardItem* createContactItem(User* user, int unreadCount = 0);
    void setupMessageTimer();
    void updateUnreadMessagesDisplay();
};

#endif // CHATWINDOW_H
