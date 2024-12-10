#include "ChatWindow.h"
#include "ui_ChatWindow.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QFont>
#include <QFile>

ChatWindow::ChatWindow(User* currentUser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatWindow),
    currentUser(currentUser),
    chatController(new ChatController(this)),
    selectedUser(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Чат");
    setupConnections();
    setupMessageTimer();
    loadContacts();

    chatController->setCurrentUserId(currentUser->getUserId());

    QFile styleFile(":/styles/chat.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::setupMessageTimer()
{
    messageCheckTimer = new QTimer(this);
    connect(messageCheckTimer, &QTimer::timeout, this, &ChatWindow::checkNewMessages);
    messageCheckTimer->start(5000); // Проверка каждые 5 секунд
}

void ChatWindow::setupConnections()
{
    connect(ui->contactsListView, &QListView::clicked, this, &ChatWindow::onUserSelected);
    connect(ui->sendMessageButton, &QPushButton::clicked, this, &ChatWindow::onSendMessageButtonClicked);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &ChatWindow::onSearchTextChanged);
    connect(ui->messageLineEdit, &QLineEdit::returnPressed, this, &ChatWindow::onSendMessageButtonClicked);
    connect(chatController, &ChatController::newMessage, this, &ChatWindow::onNewMessage);
}

QStandardItem* ChatWindow::createContactItem(User* user, int unreadCount)
{
    QStandardItem* item = new QStandardItem();
    item->setData(user->getUserId(), Qt::UserRole);

    QString displayText = user->getUsername();
    if (unreadCount > 0) {
        displayText += QString(" (%1)").arg(unreadCount);
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
    }

    item->setText(displayText);
    return item;
}

void ChatWindow::updateContactsList()
{
    QStandardItemModel* model = new QStandardItemModel(this);
    QList<QPair<User*, QPair<int, QDateTime>>> sortedContacts; // Добавляем время последнего сообщения

    for (User* user : contactsList) {
        if (user->getUserId() == currentUser->getUserId())
            continue;

        int unreadCount = chatController->getUnreadMessagesCount(currentUser->getUserId(), user->getUserId());

        // Получаем время последнего сообщения
        QDateTime lastMessageTime = chatController->getLastMessageTime(currentUser->getUserId(), user->getUserId());

        sortedContacts.append(qMakePair(user, qMakePair(unreadCount, lastMessageTime)));
    }

    // Сортируем контакты: сначала по наличию непрочитанных, потом по времени последнего сообщения
    std::sort(sortedContacts.begin(), sortedContacts.end(),
              [](const auto& a, const auto& b) {
                  if (a.second.first > 0 && b.second.first == 0) return true;
                  if (a.second.first == 0 && b.second.first > 0) return false;
                  if (a.second.second.isValid() && b.second.second.isValid()) {
                      return a.second.second > b.second.second;
                  }
                  if (a.second.second.isValid()) return true;
                  if (b.second.second.isValid()) return false;
                  return a.first->getUsername().toLower() < b.first->getUsername().toLower();
              });

    for (const auto& pair : sortedContacts) {
        User* user = pair.first;
        int unreadCount = pair.second.first;

        QStandardItem* item = new QStandardItem();
        item->setData(user->getUserId(), Qt::UserRole);

        QString displayText = user->getUsername();
        if (unreadCount > 0) {
            displayText += QString(" (%1)").arg(unreadCount);
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setForeground(QColor("#2196F3"));
        }

        item->setText(displayText);
        model->appendRow(item);
    }

    ui->contactsListView->setModel(model);
}

void ChatWindow::loadContacts()
{
    qDeleteAll(contactsList);
    contactsList = chatController->searchUsers("");
    updateContactsList();
}

void ChatWindow::onUserSelected(const QModelIndex& index)
{
    if (!index.isValid()) {
        return; // Добавляем проверку валидности индекса
    }

    int userId = index.data(Qt::UserRole).toInt();
    if (userId <= 0) {
        return; // Проверяем валидность ID пользователя
    }

    selectedUser = nullptr;
    for (User* user : contactsList) {
        if (user->getUserId() == userId) {
            selectedUser = user;
            break;
        }
    }

    if (selectedUser) {
        loadMessages();
    } else {
        ui->chatTextEdit->clear(); // Очищаем чат если пользователь не найден
    }
}

void ChatWindow::loadMessages()
{
    if (!selectedUser) return;

    User* user = selectedUser;
    ui->chatTextEdit->clear();
    QList<ChatMessage> messages = chatController->getMessagesBetweenUsers(
        currentUser->getUserId(), user->getUserId());

    bool isUnread = false;
    for (const ChatMessage& msg : messages) {
        if (!msg.getIsRead() && msg.getReceiverId() == currentUser->getUserId()) {
            isUnread = true;
            chatController->markMessageAsRead(msg.getMessageId());
            // Явно испускаем сигнал о прочтении
            emit chatController->messageRead(currentUser->getUserId());
        }
        QString messageId = QString("message_%1").arg(msg.getMessageId());

        QString messageStyle = isUnread ?
                                   "background-color: #ffebee; border-left: 3px solid #f44336; padding: 5px; margin: 2px 0;" : // Красный цвет для непрочитанных
                                   "padding: 5px; margin: 2px 0;";

        QString messageHtml = QString(
                                  "<div id='%1' style='%2'>"
                                  "<span style='color: #757575;'>[%3]</span> "
                                  "<span style='font-weight: bold; color: %4;'>%5:</span> "
                                  "<span>%6</span></div>")
                                  .arg(messageId)
                                  .arg(messageStyle)
                                  .arg(msg.getSentAt().toString("dd.MM.yyyy hh:mm"))
                                  .arg(msg.getSenderId() == currentUser->getUserId() ? "#1976D2" : "#f44336") // Красный цвет имени для непрочитанных
                                  .arg(msg.getSenderId() == currentUser->getUserId() ? "Вы" : user->getUsername())
                                  .arg(msg.getContent());

        ui->chatTextEdit->append(messageHtml);

        if (isUnread) {
            chatController->markMessageAsRead(msg.getMessageId());

            QTimer *timer = new QTimer(this);
            timer->setSingleShot(true);

            connect(timer, &QTimer::timeout,
                    [this, messageId, timer, msg, user]() {
                        QTextCursor cursor(ui->chatTextEdit->document());
                        cursor.movePosition(QTextCursor::Start);

                        QRegularExpression re(QString("<div id='%1'.*?</div>").arg(messageId));
                        QString html = ui->chatTextEdit->toHtml();
                        QString newHtml = html.replace(re, QString(
                                                               "<div id='%1' style='padding: 5px; margin: 2px 0;'>"
                                                               "<span style='color: #757575;'>[%2]</span> "
                                                               "<span style='font-weight: bold; color: %3;'>%4:</span> "
                                                               "<span>%5</span></div>")
                                                               .arg(messageId)
                                                               .arg(msg.getSentAt().toString("dd.MM.yyyy hh:mm"))
                                                               .arg(msg.getSenderId() == currentUser->getUserId() ? "#1976D2" : "#2196F3")
                                                               .arg(msg.getSenderId() == currentUser->getUserId() ? "Вы" : user->getUsername())
                                                               .arg(msg.getContent()));

                        ui->chatTextEdit->setHtml(newHtml);
                        timer->deleteLater();
                    });
            timer->start(2000);
        }
    }

    updateContactsList();

    if (isUnread) {
        emit chatController->messageRead(currentUser->getUserId());
    }
}

void ChatWindow::onSendMessageButtonClicked()
{
    if (!selectedUser) {
        QMessageBox::warning(this, "Ошибка", "Выберите пользователя для отправки сообщения.");
        return;
    }

    QString messageContent = ui->messageLineEdit->text().trimmed();
    if (messageContent.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сообщение не может быть пустым.");
        return;
    }

    ChatMessage message;
    message.setSenderId(currentUser->getUserId());
    message.setReceiverId(selectedUser->getUserId());
    message.setContent(messageContent);
    message.setSentAt(QDateTime::currentDateTime());
    message.setIsRead(false);

    if (chatController->sendMessage(message)) {
        ui->messageLineEdit->clear();
        loadMessages();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось отправить сообщение.");
    }
}

void ChatWindow::onSearchTextChanged(const QString& text)
{
    contactsList = chatController->searchUsers(text);
    updateContactsList();
}

void ChatWindow::checkNewMessages()
{
    // Проверяем новые сообщения для текущего пользователя
    QList<ChatMessage> newMessages = chatController->getUnreadMessages(currentUser->getUserId());

    if (!newMessages.isEmpty()) {
        updateContactsList();

        // Если открыт чат с отправителем, обновляем его
        if (selectedUser) {
            for (const ChatMessage& msg : newMessages) {
                if (msg.getSenderId() == selectedUser->getUserId()) {
                    loadMessages();
                    break;
                }
            }
        }
    }
}

void ChatWindow::onNewMessage(const ChatMessage& message)
{
    if (selectedUser && message.getSenderId() == selectedUser->getUserId()) {
        User* user = selectedUser;
        QString messageId = QString("message_%1").arg(message.getMessageId());

        // Стиль для нового сообщения - красный цвет
        QString messageStyle =
            "background-color: #ffebee; border-left: 3px solid #f44336; "
            "padding: 5px; margin: 2px 0;";

        QString messageHtml = QString(
                                  "<div id='%1' style='%2'>"
                                  "<span style='color: #757575;'>[%3]</span> "
                                  "<span style='font-weight: bold; color: #f44336;'>%4:</span> " // Красный цвет имени
                                  "<span>%5</span></div>")
                                  .arg(messageId)
                                  .arg(messageStyle)
                                  .arg(message.getSentAt().toString("dd.MM.yyyy hh:mm"))
                                  .arg(user->getUsername())
                                  .arg(message.getContent());

        ui->chatTextEdit->append(messageHtml);

        QTimer *timer = new QTimer(this);
        timer->setSingleShot(true);

        connect(timer, &QTimer::timeout,
                [this, messageId, timer, message, user]() {
                    QTextCursor cursor(ui->chatTextEdit->document());
                    cursor.movePosition(QTextCursor::Start);

                    QRegularExpression re(QString("<div id='%1'.*?</div>").arg(messageId));
                    QString html = ui->chatTextEdit->toHtml();
                    QString newHtml = html.replace(re, QString(
                                                           "<div id='%1' style='padding: 5px; margin: 2px 0;'>"
                                                           "<span style='color: #757575;'>[%2]</span> "
                                                           "<span style='font-weight: bold; color: #2196F3;'>%3:</span> "
                                                           "<span>%4</span></div>")
                                                           .arg(messageId)
                                                           .arg(message.getSentAt().toString("dd.MM.yyyy hh:mm"))
                                                           .arg(user->getUsername())
                                                           .arg(message.getContent()));

                    ui->chatTextEdit->setHtml(newHtml);
                    timer->deleteLater();
                });
        timer->start(2000);

        chatController->markMessageAsRead(message.getMessageId());
    }
}
