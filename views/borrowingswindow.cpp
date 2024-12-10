#include "BorrowingsWindow.h"
#include "ui_BorrowingsWindow.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include "../models/Game.h" // Добавлено
#include "../controllers/GameController.h" // Добавлено

BorrowingsWindow::BorrowingsWindow(User* currentUser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BorrowingsWindow),
    currentUser(currentUser),
    borrowingController(new BorrowingController(this)),
    gameController(new GameController(this)) // Инициализировано
{
    ui->setupUi(this);
    setWindowTitle("Управление бронированиями");
    loadBorrowings();
    setupConnections();

    borrowingController->setCurrentUserId(currentUser->getUserId());
    gameController->setCurrentUserId(currentUser->getUserId());

    connect(borrowingController, &BorrowingController::borrowingStatusChanged,
            this, &BorrowingsWindow::onBorrowingChanged);
    connect(borrowingController, &BorrowingController::borrowingRequested,
            this, &BorrowingsWindow::onBorrowingChanged);
    connect(borrowingController, &BorrowingController::borrowingReturned,
            this, &BorrowingsWindow::onBorrowingChanged);
    connect(borrowingController, &BorrowingController::borrowingStatusChanged,
            this, &BorrowingsWindow::onBorrowingStatusChanged);
}

BorrowingsWindow::~BorrowingsWindow()
{
    delete ui;
}

void BorrowingsWindow::setupConnections() {
    connect(ui->confirmButton, &QPushButton::clicked, this, &BorrowingsWindow::onConfirmButtonClicked);
    connect(ui->declineButton, &QPushButton::clicked, this, &BorrowingsWindow::onDeclineButtonClicked);
    connect(ui->returnButton, &QPushButton::clicked, this, &BorrowingsWindow::onReturnButtonClicked);
}

// views/borrowingswindow.cpp
void BorrowingsWindow::loadBorrowings() {
    // Загрузка моих бронирований
    myBorrowings = borrowingController->getBorrowingsByUser(currentUser->getUserId());
    QStandardItemModel *myModel = new QStandardItemModel(this);
    myModel->setColumnCount(6); // Добавляем колонки для дат
    myModel->setHeaderData(0, Qt::Horizontal, "Игра");
    myModel->setHeaderData(1, Qt::Horizontal, "Владелец");
    myModel->setHeaderData(2, Qt::Horizontal, "Дата начала");
    myModel->setHeaderData(3, Qt::Horizontal, "Дата окончания");
    myModel->setHeaderData(4, Qt::Horizontal, "Статус");
    myModel->setHeaderData(5, Qt::Horizontal, "ID");

    for (const Borrowing& borrowing : myBorrowings) {
        QList<QStandardItem*> rowItems;
        UserGame userGame = UserGame::getUserGameById(borrowing.getLenderUserGameId());
        Game game = Game::getGameById(userGame.getGameId());
        User* owner = User::getUserById(userGame.getUserId());

        rowItems << new QStandardItem(game.getTitle());
        rowItems << new QStandardItem(owner ? owner->getUsername() : "Неизвестно");
        rowItems << new QStandardItem(borrowing.getStartDate().toString("dd.MM.yyyy"));
        rowItems << new QStandardItem(borrowing.getEndDate().toString("dd.MM.yyyy"));
        rowItems << new QStandardItem(borrowing.getStatus());

        QStandardItem *idItem = new QStandardItem();
        idItem->setData(borrowing.getBorrowingId(), Qt::UserRole);
        rowItems << idItem;

        myModel->appendRow(rowItems);

        delete owner;
    }

    ui->myBorrowingsTableView->setModel(myModel);
    ui->myBorrowingsTableView->hideColumn(5); // Скрываем колонку с ID
    ui->myBorrowingsTableView->horizontalHeader()->setStretchLastSection(true);

    // Аналогично обновляем отображение запросов к пользователю
    requestsToMe = borrowingController->getBorrowingsForUser(currentUser->getUserId());
    QStandardItemModel *requestsModel = new QStandardItemModel(this);
    requestsModel->setColumnCount(6);
    requestsModel->setHeaderData(0, Qt::Horizontal, "Игра");
    requestsModel->setHeaderData(1, Qt::Horizontal, "Запросил");
    requestsModel->setHeaderData(2, Qt::Horizontal, "Дата начала");
    requestsModel->setHeaderData(3, Qt::Horizontal, "Дата окончания");
    requestsModel->setHeaderData(4, Qt::Horizontal, "Статус");
    requestsModel->setHeaderData(5, Qt::Horizontal, "ID");

    for (const Borrowing& borrowing : requestsToMe) {
        QList<QStandardItem*> rowItems;
        UserGame userGame = UserGame::getUserGameById(borrowing.getLenderUserGameId());
        Game game = Game::getGameById(userGame.getGameId());
        User* borrower = User::getUserById(borrowing.getBorrowerId());

        rowItems << new QStandardItem(game.getTitle());
        rowItems << new QStandardItem(borrower ? borrower->getUsername() : "Неизвестно");
        rowItems << new QStandardItem(borrowing.getStartDate().toString("dd.MM.yyyy"));
        rowItems << new QStandardItem(borrowing.getEndDate().toString("dd.MM.yyyy"));
        rowItems << new QStandardItem(borrowing.getStatus());

        QStandardItem *idItem = new QStandardItem();
        idItem->setData(borrowing.getBorrowingId(), Qt::UserRole);
        rowItems << idItem;

        requestsModel->appendRow(rowItems);

        delete borrower;
    }

    ui->requestsTableView->setModel(requestsModel);
    ui->requestsTableView->hideColumn(5);
    ui->requestsTableView->horizontalHeader()->setStretchLastSection(true);
}

void BorrowingsWindow::onConfirmButtonClicked()
{
    QModelIndex index = ui->requestsTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите запрос для подтверждения.");
        return;
    }

    int row = index.row();
    Borrowing borrowing = requestsToMe.at(row);

    if (borrowing.getStatus() != "requested") {
        QMessageBox::warning(this, "Ошибка", "Только запросы в состоянии 'requested' могут быть подтверждены.");
        return;
    }

    if (borrowingController->confirmBorrowing(borrowing.getBorrowingId())) {
        QMessageBox::information(this, "Успех", "Бронирование подтверждено.");
        emit borrowingController->borrowingStatusChanged(borrowing.getBorrowingId());
        loadBorrowings();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось подтвердить бронирование.");
    }
}

void BorrowingsWindow::onDeclineButtonClicked()
{
    QModelIndex index = ui->requestsTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите запрос для отклонения.");
        return;
    }

    int row = index.row();
    Borrowing borrowing = requestsToMe.at(row);

    if (borrowing.getStatus() != "requested") {
        QMessageBox::warning(this, "Ошибка", "Только запросы в состоянии 'requested' могут быть отклонены.");
        return;
    }

    if (borrowingController->declineBorrowing(borrowing.getBorrowingId())) {
        QMessageBox::information(this, "Успех", "Бронирование отклонено.");
        emit borrowingController->borrowingStatusChanged(borrowing.getBorrowingId());
        loadBorrowings();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось отклонить бронирование.");
    }
}

void BorrowingsWindow::onReturnButtonClicked()
{
    QModelIndex index = ui->myBorrowingsTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите бронирование для возврата.");
        return;
    }

    int row = index.row();
    Borrowing borrowing = myBorrowings.at(row);

    if (borrowing.getStatus() != "confirmed") {
        QMessageBox::warning(this, "Ошибка", "Только бронирования в состоянии 'confirmed' могут быть возвращены.");
        return;
    }

    if (borrowingController->returnBorrowing(borrowing.getBorrowingId())) {
        QMessageBox::information(this, "Успех", "Бронирование возвращено.");
        emit borrowingController->borrowingStatusChanged(borrowing.getBorrowingId());
        loadBorrowings();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось вернуть бронирование.");
    }
}

void BorrowingsWindow::onBorrowingChanged(int borrowingId)
{
    loadBorrowings(); // Обновляем список бронирований
}

void BorrowingsWindow::onBorrowingStatusChanged(int borrowingId)
{
    loadBorrowings();
}
