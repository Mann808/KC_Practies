#include "GameDetailsWindow.h"
#include "ui_GameDetailsWindow.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QNetworkInterface>
#include <QSysInfo>

GameDetailsWindow::GameDetailsWindow(User* currentUser, const Game& game, QWidget *parent)
    : QDialog(parent),
    ui(new Ui::GameDetailsWindow),
    currentUser(currentUser),
    game(game),
    borrowingController(new BorrowingController(this)),
    ratingController(new RatingController(this)),
    gameController(new GameController(this))
{
    ui->setupUi(this);
    setupDateEdits();
    setupConnections();
    loadGameDetails();

    borrowingController->setCurrentUserId(currentUser->getUserId());
    ratingController->setCurrentUserId(currentUser->getUserId());
    gameController->setCurrentUserId(currentUser->getUserId());

    connect(gameController, &GameController::gameUpdated,
            this, &GameDetailsWindow::onGameUpdated);
    connect(gameController, &GameController::ratingUpdated,
            this, &GameDetailsWindow::onGameUpdated);
    connect(borrowingController, &BorrowingController::borrowingStatusChanged,
            this, &GameDetailsWindow::onBorrowingChanged);
}

GameDetailsWindow::~GameDetailsWindow()
{
    delete ui;
}

void GameDetailsWindow::setupDateEdits()
{
    // Устанавливаем минимальную дату как текущую
    QDate currentDate = QDate::currentDate();
    ui->startDateEdit->setMinimumDate(currentDate);
    ui->endDateEdit->setMinimumDate(currentDate);

    // Устанавливаем начальные значения
    ui->startDateEdit->setDate(currentDate);
    ui->endDateEdit->setDate(currentDate.addDays(7)); // По умолчанию на неделю
}

void GameDetailsWindow::setupConnections()
{
    connect(ui->borrowButton, &QPushButton::clicked, this, &GameDetailsWindow::onBorrowButtonClicked);
    connect(ui->rateButton, &QPushButton::clicked, this, &GameDetailsWindow::onRateButtonClicked);
    connect(ui->startDateEdit, &QDateEdit::dateChanged, this, &GameDetailsWindow::onStartDateChanged);
    connect(ui->endDateEdit, &QDateEdit::dateChanged, this, &GameDetailsWindow::onEndDateChanged);
}

void GameDetailsWindow::loadGameDetails()
{
    ui->titleLabel->setText(game.getTitle());
    ui->publisherLabel->setText("Издатель: " + game.getPublisher());
    ui->yearLabel->setText("Год выпуска: " + QString::number(game.getReleaseYear()));
    ui->genresLabel->setText("Жанры: " + gameController->getGameGenresAsString(game.getGameId()));
    ui->descriptionTextEdit->setPlainText(game.getDescription());
    ui->averageRatingLabel->setText(QString("Средний рейтинг: %1")
                                        .arg(QString::number(gameController->getGameAverageRating(game.getGameId()), 'f', 2)));

    int totalCopies = gameController->getTotalCopies(game.getGameId());
    ui->totalCopiesLabel->setText(QString("Общее количество копий: %1").arg(totalCopies));

    // Загрузка пользователей с игрой
    userGamesList = UserGame::getUserGamesByGameId(game.getGameId());
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal, "Пользователь");
    model->setHeaderData(1, Qt::Horizontal, "Доступные копии");
    model->setHeaderData(2, Qt::Horizontal, "ID");

    for (const UserGame& userGame : userGamesList) {
        if (userGame.getUserId() == currentUser->getUserId()) continue;

        User* user = User::getUserById(userGame.getUserId());
        if (!user) continue;

        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(user->getUsername());
        rowItems << new QStandardItem(QString::number(userGame.getAvailableCopies()));

        QStandardItem *idItem = new QStandardItem();
        idItem->setData(userGame.getUserGameId(), Qt::UserRole);
        rowItems << idItem;

        model->appendRow(rowItems);
        delete user;
    }

    ui->usersTableView->setModel(model);
    ui->usersTableView->hideColumn(2); // Скрываем колонку с ID
    ui->usersTableView->horizontalHeader()->setStretchLastSection(true);
}

void GameDetailsWindow::onStartDateChanged(const QDate& date)
{
    if (date > ui->endDateEdit->date()) {
        ui->endDateEdit->setDate(date);
    }
}

void GameDetailsWindow::onEndDateChanged(const QDate& date)
{
    if (date < ui->startDateEdit->date()) {
        ui->startDateEdit->setDate(date);
    }
}

void GameDetailsWindow::onBorrowButtonClicked()
{
    QModelIndex index = ui->usersTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите пользователя для бронирования.");
        return;
    }

    int row = index.row();
    int userGameId = ui->usersTableView->model()->data(
                                                    ui->usersTableView->model()->index(row, 2), Qt::UserRole).toInt();

    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();

    if (startDate >= endDate) {
        QMessageBox::warning(this, "Ошибка",
                             "Дата окончания должна быть позже даты начала.");
        return;
    }

    if (borrowingController->requestBorrowing(userGameId, currentUser->getUserId(),
                                              startDate, endDate))
    {
        // Логируем действие
        Log log;
        log.setUserId(currentUser->getUserId());
        log.setAction("BorrowingRequest");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Запрос на бронирование игры '%1' с %2 по %3")
                           .arg(game.getTitle())
                           .arg(startDate.toString("dd.MM.yyyy"))
                           .arg(endDate.toString("dd.MM.yyyy")));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        QMessageBox::information(this, "Запрос отправлен", "Запрос на бронирование отправлен.");
        emit gameUpdated();
        this->accept();
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось отправить запрос на бронирование.");
    }
}

void GameDetailsWindow::onRateButtonClicked()
{
    int ratingValue = ui->ratingSpinBox->value();
    if (ratingController->addOrUpdateRating(currentUser->getUserId(), game.getGameId(),
                                            ratingValue))
    {
        // Логируем действие
        Log log;
        log.setUserId(currentUser->getUserId());
        log.setAction("GameRating");
        log.setTimestamp(QDateTime::currentDateTime());
        log.setDetails(QString("Оценка игры '%1': %2")
                           .arg(game.getTitle())
                           .arg(ratingValue));
        log.setIpAddress(QNetworkInterface::allAddresses().first().toString());
        log.setDeviceInfo(QSysInfo::prettyProductName());
        Log::addLog(log);

        QMessageBox::information(this, "Успех", "Ваша оценка сохранена.");
        // Обновляем отображение среднего рейтинга
        ui->averageRatingLabel->setText(QString("Средний рейтинг: %1")
                                            .arg(QString::number(gameController->getGameAverageRating(game.getGameId()),
                                                                 'f', 2)));
        emit gameUpdated();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить вашу оценку.");
    }
}

void GameDetailsWindow::onGameUpdated(int gameId)
{
    if (gameId == game.getGameId()) {
        loadGameDetails(); // Обновляем детали игры
    }
}

void GameDetailsWindow::onBorrowingChanged(int borrowingId)
{
    loadGameDetails(); // Обновляем информацию о доступных копиях
}
