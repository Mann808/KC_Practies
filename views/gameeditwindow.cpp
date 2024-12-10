#include "GameEditWindow.h"
#include "ui_GameEditWindow.h"
#include <QMessageBox>

GameEditWindow::GameEditWindow(User* currentUser, QWidget *parent, const Game& game) :
    QDialog(parent),
    ui(new Ui::GameEditWindow),
    currentUser(currentUser),
    currentGame(game)
{
    ui->setupUi(this);
    gameController = new GameController(this);
    isEditMode = (currentGame.getGameId() != -1);

    setupConnections();
    loadGenres();

    if (isEditMode)
    {
        populateFields();
        this->setWindowTitle("Редактирование игры");
    }
    else
    {
        this->setWindowTitle("Добавление новой игры");
    }
}

GameEditWindow::~GameEditWindow()
{
    delete ui;
}

void GameEditWindow::setupConnections()
{
    connect(ui->saveButton, &QPushButton::clicked, this, &GameEditWindow::onSaveClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &GameEditWindow::onCancelClicked);
}

void GameEditWindow::populateFields()
{
    ui->titleLineEdit->setText(currentGame.getTitle());
    ui->descriptionTextEdit->setPlainText(currentGame.getDescription());
    ui->publisherLineEdit->setText(currentGame.getPublisher());
    ui->releaseYearSpinBox->setValue(currentGame.getReleaseYear());
    int numberOfPlayers = gameController->getNumberOfPlayers(currentGame.getGameId());
    ui->playersSpinBox->setValue(numberOfPlayers);
    ui->playersSpinBox->setReadOnly(true);
    // Установить жанры игры
    QList<int> gameGenreIds = gameController->getGameGenreIds(currentGame.getGameId());
    for (int i = 0; i < ui->genresListWidget->count(); ++i)
    {
        QListWidgetItem* item = ui->genresListWidget->item(i);
        int genreId = item->data(Qt::UserRole).toInt();
        if (gameGenreIds.contains(genreId))
        {
            item->setSelected(true);
            item->setCheckState(Qt::Checked);
        }
    }
}

void GameEditWindow::loadGenres()
{
    QList<Genre> genres = gameController->getAllGenres();
    for (const Genre& genre : genres)
    {
        QListWidgetItem* item = new QListWidgetItem(genre.getName());
        item->setData(Qt::UserRole, genre.getGenreId());
        ui->genresListWidget->addItem(item);
    }
}

void GameEditWindow::onSaveClicked()
{
    Game game;
    game.setTitle(ui->titleLineEdit->text());
    game.setDescription(ui->descriptionTextEdit->toPlainText());
    game.setPublisher(ui->publisherLineEdit->text());
    game.setReleaseYear(ui->releaseYearSpinBox->value());
    //game.setPlayers(ui->playersSpinBox->value());

    QList<int> selectedGenreIds;
    QList<QListWidgetItem*> selectedItems = ui->genresListWidget->selectedItems();
    for (QListWidgetItem* item : selectedItems)
    {
        int genreId = item->data(Qt::UserRole).toInt();
        selectedGenreIds.append(genreId);
    }

    bool success = false;
    if (isEditMode)
    {
        game.setGameId(currentGame.getGameId());
        success = gameController->updateGame(game, selectedGenreIds);
    }
    else
    {
        success = gameController->addGame(game, selectedGenreIds);
    }

    if (success)
    {
        QMessageBox::information(this, "Сохранение", "Игра успешно сохранена.");
        emit gameSaved();
        this->accept();
    }
    else
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить игру.");
    }
}

void GameEditWindow::onCancelClicked()
{
    this->reject();
}
