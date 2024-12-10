#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "GameDetailsWindow.h"
#include "AddGameWindow.h"
#include "chatwindow.h"
#include "BorrowingsWindow.h"
#include "AdminPanelWindow.h"
#include "LoginWindow.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>

MainWindow::MainWindow(User* currentUser, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentUser(currentUser),
    gameController(new GameController(this)),
    userGameController(new UserGameController(this)),
    borrowingController(new BorrowingController(this)),
    chatController(new ChatController(this)),
    gamesModel(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Сначала настраиваем UI
    setupUI();

    // Затем подключаем основные сигналы UI
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(ui->genreComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onGenreFilterChanged);
    connect(ui->gamesTableView, &QTableView::doubleClicked, this, &MainWindow::onGameDoubleClicked);
    connect(ui->addGameButton, &QPushButton::clicked, this, &MainWindow::onAddGameButtonClicked);
    connect(ui->chatButton, &QPushButton::clicked, this, &MainWindow::onChatButtonClicked);
    connect(ui->borrowingsButton, &QPushButton::clicked, this, &MainWindow::onBorrowingsButtonClicked);
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutButtonClicked);

    connect(gameController, &GameController::gameAdded,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);

    if (currentUser->getRole() == "admin") {
        ui->adminPanelButton->setVisible(true);
        connect(ui->adminPanelButton, &QPushButton::clicked, this, &MainWindow::onAdminPanelButtonClicked);
    } else {
        ui->adminPanelButton->setVisible(false);
    }

    // Теперь подключаем сигналы контроллеров
    connect(gameController, &GameController::gameAdded,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);
    connect(gameController, &GameController::gameUpdated,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);
    connect(gameController, &GameController::gameDeleted,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);
    connect(gameController, &GameController::ratingUpdated,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);

    connect(borrowingController, &BorrowingController::borrowingStatusChanged,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);
    connect(borrowingController, &BorrowingController::borrowingRequested,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);
    connect(borrowingController, &BorrowingController::borrowingReturned,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);

    connect(chatController, &ChatController::messageReceived,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);
    connect(chatController, &ChatController::messageRead,
            this, &MainWindow::updateAllWindows, Qt::QueuedConnection);

    // В конце загружаем данные
    loadGames();
    updateBorrowingsList();
    updateChatNotifications();

    gameController->setCurrentUserId(currentUser->getUserId());
    userGameController->setCurrentUserId(currentUser->getUserId());
    borrowingController->setCurrentUserId(currentUser->getUserId());
    chatController->setCurrentUserId(currentUser->getUserId());

    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        updateAllWindows();
    });
    updateTimer->start(5000);

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUI() {
    ui->gamesTableView->setModel(gamesModel);
    gamesModel->setColumnCount(6);
    gamesModel->setHeaderData(0, Qt::Horizontal, "Название");
    gamesModel->setHeaderData(1, Qt::Horizontal, "Издатель");
    gamesModel->setHeaderData(2, Qt::Horizontal, "Год выпуска");
    gamesModel->setHeaderData(3, Qt::Horizontal, "Жанры");
    gamesModel->setHeaderData(4, Qt::Horizontal, "Средний рейтинг");
    gamesModel->setHeaderData(5, Qt::Horizontal, "Количество игроков"); // Новый столбец

    ui->gamesTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->gamesTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->gamesTableView->horizontalHeader()->setStretchLastSection(true);

    // Загрузка жанров в комбобокс
    genresList = gameController->getAllGenres();
    ui->genreComboBox->addItem("Все жанры", -1);
    for (const Genre& genre : genresList) {
        ui->genreComboBox->addItem(genre.getName(), genre.getGenreId());
    }
}

void MainWindow::loadGames() {
    // Очищаем текущий список
    gamesList = gameController->getAllGames();
    gamesModel->removeRows(0, gamesModel->rowCount());

    for (const Game& game : gamesList) {
        if (game.getGameId() <= 0) {
            continue; // Пропускаем игры с недопустимым gameId
        }

        QList<QStandardItem*> rowItems;

        // Создаем элементы для каждой колонки
        QStandardItem* titleItem = new QStandardItem(game.getTitle());
        QStandardItem* publisherItem = new QStandardItem(game.getPublisher());
        QStandardItem* yearItem = new QStandardItem(QString::number(game.getReleaseYear()));
        QStandardItem* genresItem = new QStandardItem(gameController->getGameGenresAsString(game.getGameId()));
        QStandardItem* ratingItem = new QStandardItem(
            QString::number(gameController->getGameAverageRating(game.getGameId()), 'f', 2));
        QStandardItem* playersItem = new QStandardItem(
            QString::number(gameController->getTotalPlayers(game.getGameId())));

        // Добавляем элементы в строку
        rowItems << titleItem << publisherItem << yearItem << genresItem << ratingItem << playersItem;

        // Устанавливаем данные игры
        for (QStandardItem* item : rowItems) {
            item->setData(game.getGameId(), Qt::UserRole);
            item->setEditable(false);
        }

        gamesModel->appendRow(rowItems);
    }

    // Подгоняем размеры колонок под содержимое
    ui->gamesTableView->resizeColumnsToContents();

    // Обновляем представление
    ui->gamesTableView->viewport()->update();
}

void MainWindow::onSearchTextChanged(const QString& text) {
    filterGames();
}

void MainWindow::onGenreFilterChanged(int index) {
    filterGames();
}

void MainWindow::filterGames() {
    QString searchText = ui->searchLineEdit->text().toLower();
    int selectedGenreId = ui->genreComboBox->currentData().toInt();

    gamesModel->removeRows(0, gamesModel->rowCount());

    for (const Game& game : gamesList) {
        if (game.getGameId() <= 0) {
            continue; // Пропускаем игры с недопустимым gameId
        }

        bool matchesSearch = game.getTitle().toLower().contains(searchText);

        bool matchesGenre = (selectedGenreId == -1) || GameGenre::getGenresForGame(game.getGameId()).contains(selectedGenreId);

        if (matchesSearch && matchesGenre) {
            QList<QStandardItem*> rowItems;
            rowItems << new QStandardItem(game.getTitle());
            rowItems << new QStandardItem(game.getPublisher());
            rowItems << new QStandardItem(QString::number(game.getReleaseYear()));
            rowItems << new QStandardItem(gameController->getGameGenresAsString(game.getGameId()));
            rowItems << new QStandardItem(QString::number(gameController->getGameAverageRating(game.getGameId()), 'f', 2));

            int totalPlayers = gameController->getTotalPlayers(game.getGameId());
            rowItems << new QStandardItem(QString::number(totalPlayers));

            gamesModel->appendRow(rowItems);
        }
    }
}

void MainWindow::onGameDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) return;

    int row = index.row();
    QString gameTitle = gamesModel->item(row, 0)->text();

    Game selectedGame;
    for (const Game& game : gamesList) {
        if (game.getTitle() == gameTitle) {
            selectedGame = game;
            break;
        }
    }

    if (selectedGame.getGameId() != -1) {
        GameDetailsWindow *detailsWindow = new GameDetailsWindow(currentUser, selectedGame, this);
        // Подключаем сигнал обновления
        connect(detailsWindow, &GameDetailsWindow::gameUpdated, this, [this]() {
            loadGames(); // Обновляем список игр
        });
        detailsWindow->exec();
        delete detailsWindow;
    }
}

void MainWindow::onAddGameButtonClicked() {
    AddGameWindow *addGameWindow = new AddGameWindow(currentUser, this);
    connect(addGameWindow, &AddGameWindow::gameAdded, this, &MainWindow::updateAllWindows);
    addGameWindow->exec();
    delete addGameWindow;
}

void MainWindow::onChatButtonClicked()
{
    ChatWindow *chatWindow = new ChatWindow(currentUser, this);
    connect(chatWindow, &ChatWindow::finished, this, &MainWindow::updateAllWindows);
    chatWindow->show();
}

void MainWindow::onBorrowingsButtonClicked()
{
    BorrowingsWindow *borrowingsWindow = new BorrowingsWindow(currentUser, this);
    connect(borrowingsWindow, &BorrowingsWindow::finished, this, &MainWindow::updateAllWindows);
    borrowingsWindow->exec();
}

void MainWindow::onLogoutButtonClicked() {
    this->close();
    LoginWindow *loginWindow = new LoginWindow();
    loginWindow->show();
}

void MainWindow::onAdminPanelButtonClicked() {
    AdminPanelWindow *adminPanel = new AdminPanelWindow(currentUser);
    adminPanel->setAttribute(Qt::WA_DeleteOnClose);
    adminPanel->show();
}

void MainWindow::onGameChanged(int gameId)
{
    loadGames(); // Обновляем список игр
}

void MainWindow::onBorrowingChanged(int borrowingId)
{
    loadGames(); // Обновляем список игр, так как могло измениться количество доступных копий
    updateBorrowingsList(); // Обновляем список заимствований
}

void MainWindow::onChatUpdated(int userId)
{
    updateChatNotifications(); // Обновляем индикаторы непрочитанных сообщений
}

void MainWindow::updateBorrowingsList()
{
    // Получаем заимствования где пользователь - заемщик
    QList<Borrowing> borrowings = borrowingController->getBorrowingsByUser(currentUser->getUserId());
    // Получаем заимствования где пользователь - владелец игры
    QList<Borrowing> lentGames = borrowingController->getBorrowingsForUser(currentUser->getUserId());

    int activeCount = 0;
    int requestCount = 0;

    // Подсчитываем активные заимствования и запросы
    for (const Borrowing& b : borrowings) {
        if (b.getStatus() == "confirmed") {
            activeCount++;
        }
    }

    for (const Borrowing& b : lentGames) {
        if (b.getStatus() == "requested") {
            requestCount++;
        }
    }

    // Обновляем текст кнопки заимствований
    QString buttonText = "Бронирования";
    if (activeCount > 0 || requestCount > 0) {
        buttonText += QString(" (%1").arg(activeCount);
        if (requestCount > 0) {
            buttonText += QString("/%1").arg(requestCount);
        }
        buttonText += ")";
    }
    ui->borrowingsButton->setText(buttonText);
}

void MainWindow::updateChatNotifications()
{
    QList<ChatMessage> unreadMessages = chatController->getUnreadMessages(currentUser->getUserId());

    // Обновляем индикатор на кнопке чата
    if (!unreadMessages.isEmpty()) {
        ui->chatButton->setText(QString("Чат (%1)").arg(unreadMessages.size()));
    } else {
        ui->chatButton->setText("Чат");
    }
}

void MainWindow::updateAllWindows() {
    // Обновляем список игр
    loadGames();

    // Обновляем счетчик бронирований
    updateBorrowingsList();

    // Обновляем счетчик чата
    updateChatNotifications();

    // Принудительно обновляем отображение
    ui->gamesTableView->viewport()->update();

    updateGenresList();

    // Вызываем обработку фильтров для обновления отображения
    filterGames();
}

void MainWindow::updateGenresList() {
    // Сохраняем текущий выбранный жанр
    int currentGenreId = ui->genreComboBox->currentData().toInt();

    // Очищаем и перезаполняем список
    ui->genreComboBox->clear();
    ui->genreComboBox->addItem("Все жанры", -1);
    genresList = gameController->getAllGenres();

    for (const Genre& genre : genresList) {
        ui->genreComboBox->addItem(genre.getName(), genre.getGenreId());

        // Восстанавливаем предыдущий выбор
        if(genre.getGenreId() == currentGenreId) {
            ui->genreComboBox->setCurrentIndex(ui->genreComboBox->count() - 1);
        }
    }
}
