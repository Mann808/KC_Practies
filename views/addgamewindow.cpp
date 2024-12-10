#include "AddGameWindow.h"
#include "ui_AddGameWindow.h"
#include <QMessageBox>

AddGameWindow::AddGameWindow(User* currentUser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddGameWindow),
    currentUser(currentUser),
    userGameController(new UserGameController(this)),
    gameController(new GameController(this))
{
    ui->setupUi(this);

    gameController->setCurrentUserId(currentUser->getUserId());
    userGameController->setCurrentUserId(currentUser->getUserId());

    setupUI();

    connect(ui->existingGameComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddGameWindow::onExistingGameSelected);
    connect(ui->addGameButton, &QPushButton::clicked, this, &AddGameWindow::onAddGameButtonClicked);
}

AddGameWindow::~AddGameWindow()
{
    delete ui;
}

void AddGameWindow::setupUI() {
    existingGames = gameController->getAllGames();
    ui->existingGameComboBox->addItem("Создать новую игру", -1);
    for (const Game& game : existingGames) {
        ui->existingGameComboBox->addItem(game.getTitle(), game.getGameId());
    }

    // Загрузка жанров
    genresList = gameController->getAllGenres();
    for (const Genre& genre : genresList) {
        QListWidgetItem *item = new QListWidgetItem(genre.getName());
        item->setData(Qt::UserRole, genre.getGenreId());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        ui->genresListWidget->addItem(item);
    }

    // Устанавливаем видимость newGameGroupBox в соответствии с выбранным пунктом
    onExistingGameSelected(ui->existingGameComboBox->currentIndex());
}

void AddGameWindow::onExistingGameSelected(int index) {
    int gameId = ui->existingGameComboBox->currentData().toInt();
    if (gameId == -1) {
        // Выбрано создание новой игры
        ui->newGameGroupBox->setVisible(true);
    } else {
        // Выбрана существующая игра
        ui->newGameGroupBox->setVisible(false);
    }
}

void AddGameWindow::onAddGameButtonClicked() {
    int copies = ui->copiesSpinBox->value();
    if (copies <= 0) {
        QMessageBox::warning(this, "Ошибка", "Количество копий должно быть больше нуля.");
        return;
    }

    int gameId = ui->existingGameComboBox->currentData().toInt();
    if (gameId == -1) {
        // Создаем новую игру
        QString title = ui->titleLineEdit->text().trimmed();
        if (title.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Название игры не может быть пустым.");
            return;
        }

        Game newGame;
        newGame.setTitle(title);
        newGame.setDescription(ui->descriptionTextEdit->toPlainText().trimmed());
        newGame.setPublisher(ui->publisherLineEdit->text().trimmed());
        newGame.setReleaseYear(ui->yearSpinBox->value());

        QList<int> selectedGenreIds;
        for (int i = 0; i < ui->genresListWidget->count(); ++i) {
            QListWidgetItem *item = ui->genresListWidget->item(i);
            if (item->checkState() == Qt::Checked) {
                selectedGenreIds.append(item->data(Qt::UserRole).toInt());
            }
        }

        // Добавляем игру и получаем её ID
        if (!gameController->addGame(newGame, selectedGenreIds)) {
            return;
        }

        // Получаем ID новой игры
        gameId = Game::getLastInsertedId();
        if (gameId <= 0) {
            QMessageBox::warning(this, "Ошибка", "Не удалось получить ID новой игры");
            return;
        }
    }

    // Добавляем игру в коллекцию пользователя
    if (userGameController->addUserGame(currentUser->getUserId(), gameId, copies)) {
        QMessageBox::information(this, "Успех", "Игра успешно добавлена в вашу коллекцию.");
        emit gameAdded();
        this->accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось добавить игру в вашу коллекцию.");
    }
}
