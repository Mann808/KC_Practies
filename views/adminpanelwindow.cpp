#include "AdminPanelWindow.h"
#include "ui_AdminPanelWindow.h"
#include "../controllers/AdminController.h"
#include "../controllers/GameController.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QDateTime>
#include <QSqlQuery>
#include <QDebug>

AdminPanelWindow::AdminPanelWindow(User* currentUser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminPanelWindow),
    currentUser(currentUser),
    adminController(new AdminController(this)),
    gameController(new GameController(this))
{
    ui->setupUi(this);
    setWindowTitle("Панель администратора");

    adminController->setCurrentUserId(currentUser->getUserId());
    gameController->setCurrentUserId(currentUser->getUserId());

    // Инициализация моделей для таблиц
    activeBorrowingsModel = new QStandardItemModel(this);
    userStatsModel = new QStandardItemModel(this);
    popularGamesModel = new QStandardItemModel(this);

    // Настройка таймера для обновления
    maintenanceTimer = new QTimer(this);
    maintenanceTimer->setInterval(5000); // Обновление каждые 5 секунд

    // Подключение сигналов основного интерфейса
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &AdminPanelWindow::onTabChanged);
    connect(maintenanceTimer, &QTimer::timeout, this, &AdminPanelWindow::updateAll);

    // Подключение сигналов для вкладки обслуживания БД
    connect(ui->maintenanceButton, &QPushButton::clicked, this, &AdminPanelWindow::onMaintenanceButtonClicked);

    // Подключение сигналов для управления данными
    connect(ui->exportButton, &QPushButton::clicked, this, &AdminPanelWindow::onExportButtonClicked);
    connect(ui->importButton, &QPushButton::clicked, this, &AdminPanelWindow::onImportButtonClicked);
    connect(ui->exportTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminPanelWindow::onExportTypeChanged);
    connect(ui->importTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminPanelWindow::onImportTypeChanged);

    // Подключение сигналов для вкладки со статистикой
    connect(ui->dataViewsTab, &QTabWidget::currentChanged, this, &AdminPanelWindow::onDataViewTabChanged);

    // Подключение сигналов для таблиц данных
    connect(ui->activeBorrowingsTable, &QTableView::doubleClicked,
            this, &AdminPanelWindow::onActiveBorrowingDoubleClicked);
    connect(ui->userStatsTable, &QTableView::doubleClicked,
            this, &AdminPanelWindow::onUserStatsDoubleClicked);
    connect(ui->popularGamesTable, &QTableView::doubleClicked,
            this, &AdminPanelWindow::onPopularGameDoubleClicked);

    // Подключение сигналов контроллеров
    connect(adminController, &AdminController::errorOccurred,
            this, &AdminPanelWindow::showError);
    connect(gameController, &GameController::errorOccurred,
            this, &AdminPanelWindow::showError);

    setupMaintenanceTab();
    setupDataManagementTab();
    updateAll();

    maintenanceTimer->start();

    setupConnections();
    setupStatsUpdateTimer();

    onUsersTabLoaded();

    ui->startDateEdit->setDate(QDate::currentDate().addDays(-30));
    ui->endDateEdit->setDate(QDate::currentDate());
}

AdminPanelWindow::~AdminPanelWindow() {
    if (statsUpdateTimer) {
        statsUpdateTimer->stop();
        delete statsUpdateTimer;
    }
    delete ui;
}

void AdminPanelWindow::setupStatsUpdateTimer() {
    statsUpdateTimer = new QTimer(this);
    connect(statsUpdateTimer, &QTimer::timeout, this, &AdminPanelWindow::updateStatistics);
    statsUpdateTimer->start(5000); // Обновление каждые 5 секунд
}

void AdminPanelWindow::setupConnections() {
    connect(ui->tabWidget, &QTabWidget::currentChanged, [this](int index){
        switch(index) {
        case 0: onUsersTabLoaded(); break;
        case 1: onGenresTabLoaded(); break;
        case 2: onLogsTabLoaded(); break;
        case 3: onBackupsTabLoaded(); break;
        }
    });
    connect(ui->statisticsTabWidget, &QTabWidget::currentChanged,
            [this](int index) {
                switch(index) {
                case 0: // Вкладка статистики жанров
                    updateGenreStatistics();
                    break;
                case 1: // Вкладка статистики бронирований
                    updateBorrowingStatistics();
                    break;
                case 2: // Вкладка статистики рейтингов
                    updateRatingStatistics();
                    break;
                case 3: // Вкладка активности пользователей
                    updateUserActivityStatistics();
                    break;
                }
            });
    connect(ui->blockUserButton, &QPushButton::clicked, this, &AdminPanelWindow::onBlockUserButtonClicked);
    connect(ui->unblockUserButton, &QPushButton::clicked, this, &AdminPanelWindow::onUnblockUserButtonClicked);
    connect(ui->addGenreButton, &QPushButton::clicked, this, &AdminPanelWindow::onAddGenreButtonClicked);
    connect(ui->editGenreButton, &QPushButton::clicked, this, &AdminPanelWindow::onEditGenreButtonClicked);
    connect(ui->deleteGenreButton, &QPushButton::clicked, this, &AdminPanelWindow::onDeleteGenreButtonClicked);

    connect(ui->archiveLogsButton, &QPushButton::clicked, this, &AdminPanelWindow::onArchiveLogsButtonClicked);
    connect(ui->deleteLogsButton, &QPushButton::clicked, this, &AdminPanelWindow::onDeleteLogsButtonClicked);
    connect(ui->importLogsButton, &QPushButton::clicked, this, &AdminPanelWindow::onImportLogsButtonClicked);
    connect(ui->refreshLogsButton, &QPushButton::clicked, this, &AdminPanelWindow::onRefreshLogsButtonClicked);

    connect(ui->createBackupButton, &QPushButton::clicked,
            this, &AdminPanelWindow::onCreateBackupButtonClicked);
    connect(ui->restoreBackupButton, &QPushButton::clicked,
            this, &AdminPanelWindow::onRestoreBackupButtonClicked);

    connect(ui->tabWidget, &QTabWidget::currentChanged, [this](int index) {
        // Если открыта вкладка статистики
        if (index == ui->tabWidget->indexOf(ui->statisticsTab)) {
            updateStatistics();
        }
    });
}

void AdminPanelWindow::onUsersTabLoaded() {
    loadUsers();
}

void AdminPanelWindow::loadUsers() {
    usersList = adminController->getAllUsers();
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(5);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Имя пользователя");
    model->setHeaderData(2, Qt::Horizontal, "Email");
    model->setHeaderData(3, Qt::Horizontal, "Роль");
    model->setHeaderData(4, Qt::Horizontal, "Заблокирован");

    for (User* user : usersList) {
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(QString::number(user->getUserId()));
        rowItems << new QStandardItem(user->getUsername());
        rowItems << new QStandardItem(user->getEmail());
        rowItems << new QStandardItem(user->getRole());
        rowItems << new QStandardItem(user->getIsBlocked() ? "Да" : "Нет");

        model->appendRow(rowItems);
    }

    ui->usersTableView->setModel(model);
    ui->usersTableView->horizontalHeader()->setStretchLastSection(true);
}

void AdminPanelWindow::onBlockUserButtonClicked() {
    QModelIndex index = ui->usersTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите пользователя для блокировки.");
        return;
    }

    int row = index.row();
    int userId = usersList.at(row)->getUserId();

    if (adminController->blockUser(userId)) {
        QMessageBox::information(this, "Успех", "Пользователь заблокирован.");
        loadUsers();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось заблокировать пользователя.");
    }
}

void AdminPanelWindow::onUnblockUserButtonClicked() {
    QModelIndex index = ui->usersTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите пользователя для разблокировки.");
        return;
    }

    int row = index.row();
    int userId = usersList.at(row)->getUserId();

    if (adminController->unblockUser(userId)) {
        QMessageBox::information(this, "Успех", "Пользователь разблокирован.");
        loadUsers();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось разблокировать пользователя.");
    }
}

void AdminPanelWindow::onGenresTabLoaded() {
    loadGenres();
}

void AdminPanelWindow::loadGenres() {
    genresList = adminController->getAllGenres();
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(2);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Название");

    for (const Genre& genre : genresList) {
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(QString::number(genre.getGenreId()));
        rowItems << new QStandardItem(genre.getName());

        model->appendRow(rowItems);
    }

    ui->genresTableView->setModel(model);
    ui->genresTableView->horizontalHeader()->setStretchLastSection(true);
}

void AdminPanelWindow::onAddGenreButtonClicked() {
    bool ok;
    QString genreName = QInputDialog::getText(this, "Добавить жанр", "Название жанра:", QLineEdit::Normal, "", &ok);
    if (ok && !genreName.isEmpty()) {
        if (adminController->addGenre(genreName)) {
            QMessageBox::information(this, "Успех", "Жанр добавлен.");
            loadGenres();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить жанр.");
        }
    }
}

void AdminPanelWindow::onEditGenreButtonClicked() {
    QModelIndex index = ui->genresTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите жанр для редактирования.");
        return;
    }

    int row = index.row();
    Genre genre = genresList.at(row);

    bool ok;
    QString genreName = QInputDialog::getText(this, "Редактировать жанр", "Название жанра:", QLineEdit::Normal, genre.getName(), &ok);
    if (ok && !genreName.isEmpty()) {
        genre.setName(genreName);
        if (adminController->updateGenre(genre)) {
            QMessageBox::information(this, "Успех", "Жанр обновлен.");
            loadGenres();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось обновить жанр.");
        }
    }
}

void AdminPanelWindow::onDeleteGenreButtonClicked() {
    QModelIndex index = ui->genresTableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите жанр для удаления.");
        return;
    }

    int row = index.row();
    int genreId = genresList.at(row).getGenreId();

    if (adminController->deleteGenre(genreId)) {
        QMessageBox::information(this, "Успех", "Жанр удален.");
        loadGenres();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить жанр.");
    }
}

void AdminPanelWindow::onLogsTabLoaded() {
    loadLogs();
}

void AdminPanelWindow::loadLogs() {
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();

    logsList = adminController->getLogsByDateRange(startDate, endDate);

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, "ID пользователя");
    model->setHeaderData(1, Qt::Horizontal, "Действие");
    model->setHeaderData(2, Qt::Horizontal, "Время");
    model->setHeaderData(3, Qt::Horizontal, "Детали");

    for (const Log& log : logsList) {
        QList<QStandardItem*> rowItems;
        rowItems << new QStandardItem(QString::number(log.getUserId()));
        rowItems << new QStandardItem(log.getAction());
        rowItems << new QStandardItem(log.getTimestamp().toString("dd.MM.yyyy hh:mm:ss"));
        rowItems << new QStandardItem(log.getDetails());

        model->appendRow(rowItems);
    }

    ui->logsTableView->setModel(model);
    ui->logsTableView->horizontalHeader()->setStretchLastSection(true);
}

void AdminPanelWindow::onArchiveLogsButtonClicked() {
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Сохранить архив логов", "", "Файлы логов (*.log)");

    if (fileName.isEmpty())
        return;

    if (adminController->archiveLogs(startDate, endDate, fileName)) {
        QMessageBox::information(this, "Успех", "Логи успешно архивированы");
        loadLogs();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось архивировать логи");
    }
}

void AdminPanelWindow::onDeleteLogsButtonClicked() {
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();

    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "Подтверждение", "Вы уверены, что хотите удалить логи за выбранный период?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (adminController->deleteLogs(startDate, endDate)) {
            QMessageBox::information(this, "Успех", "Логи успешно удалены");
            loadLogs();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить логи");
        }
    }
}

void AdminPanelWindow::onImportLogsButtonClicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Выбрать файл логов", "", "Файлы логов (*.log)");

    if (fileName.isEmpty())
        return;

    if (adminController->importLogs(fileName)) {
        QMessageBox::information(this, "Успех", "Логи успешно импортированы");
        loadLogs();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось импортировать логи");
    }
}

void AdminPanelWindow::onRefreshLogsButtonClicked() {
    loadLogs();
}

void AdminPanelWindow::onBackupsTabLoaded() {
    loadBackups();
}

void AdminPanelWindow::loadBackups() {
    auto backups = adminController->getBackupsList();

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Дата создания");
    model->setHeaderData(2, Qt::Horizontal, "Путь к файлу");
    model->setHeaderData(3, Qt::Horizontal, "Создал");

    for (const auto& backup : backups) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(backup.backupId));
        row << new QStandardItem(backup.backupDate.toString("dd.MM.yyyy hh:mm:ss"));
        row << new QStandardItem(backup.filePath);
        row << new QStandardItem(backup.createdBy);
        model->appendRow(row);
    }

    ui->backupsTableView->setModel(model);
    ui->backupsTableView->horizontalHeader()->setStretchLastSection(true);
}

void AdminPanelWindow::onCreateBackupButtonClicked() {
    // Создаем директорию для бэкапов, если её нет
    QString backupDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/GameManagerBackups";
    QDir dir;
    if (!dir.exists(backupDir)) {
        if (!dir.mkpath(backupDir)) {
            QMessageBox::warning(this, "Ошибка",
                                 "Не удалось создать директорию для резервных копий");
            return;
        }
    }

    // Формируем имя файла с текущей датой и временем
    QString defaultFileName = backupDir + "/backup_" +
                              QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".bak";

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Сохранить резервную копию",
                                                    defaultFileName,
                                                    "Резервные копии (*.bak)");

    if (fileName.isEmpty())
        return;

    // Проверяем права доступа
    QFileInfo fileInfo(fileName);
    QFileInfo dirInfo(fileInfo.dir().path());
    if (!dirInfo.isWritable()) {
        QMessageBox::warning(this, "Ошибка",
                             "Нет прав на запись в выбранную директорию:\n" + dirInfo.filePath());
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Пробуем создать тестовый файл
    QFile testFile(fileName + ".test");
    if (!testFile.open(QIODevice::WriteOnly)) {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось создать файл в выбранной директории:\n" + testFile.errorString());
        return;
    }
    testFile.close();
    testFile.remove();

    // Теперь пытаемся создать бэкап
    if (adminController->createBackup(fileName)) {
        QApplication::restoreOverrideCursor();
        QMessageBox::information(this, "Успех",
                                 "Резервная копия успешно создана:\n" + fileName);
        loadBackups();
    } else {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, "Ошибка",
                              "Не удалось создать резервную копию.\nПроверьте права доступа и свободное место на диске.");
    }
}

void AdminPanelWindow::onRestoreBackupButtonClicked() {
    QMessageBox::StandardButton reply = QMessageBox::warning(this,
                                                             "Подтверждение",
                                                             "Восстановление из резервной копии заменит все текущие данные! Продолжить?",
                                                             QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No)
        return;

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Выбрать резервную копию", "", "Резервные копии (*.bak)");

    if (fileName.isEmpty())
        return;

    if (adminController->restoreFromBackup(fileName)) {
        QMessageBox::information(this, "Успех",
                                 "База данных успешно восстановлена из резервной копии");
        loadBackups();
        loadUsers();  // Обновляем все данные
        loadGenres();
        loadLogs();
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось восстановить базу данных из резервной копии");
    }
}

void AdminPanelWindow::updateStatistics() {
    updateGenreStatistics();
    updateBorrowingStatistics();
    updateRatingStatistics();
    updateUserActivityStatistics();
}

void AdminPanelWindow::updateGenreStatistics() {
    AdminController::GenreStatistics stats = adminController->getGenreStatistics();

    if (stats.totalGames <= 0) {
        ui->genreStatsDescription->setText("Статистика жанров:\nНа данный момент игр нет.");
        return;
    }

    QChart *chart = new QChart();
    QPieSeries *series = new QPieSeries();

    for (auto it = stats.gamesPerGenre.begin(); it != stats.gamesPerGenre.end(); ++it) {
        series->append(it.key(), it.value());
    }

    chart->addSeries(series);
    chart->setTitle("Распределение игр по жанрам");
    chart->legend()->setAlignment(Qt::AlignRight);
    chart->setMargins(QMargins(5, 5, 5, 5));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->genreChartView->parentWidget()->layout());
    if (layout) {
        layout->removeWidget(ui->genreChartView);
        delete ui->genreChartView;
        layout->insertWidget(0, chartView);
        ui->genreChartView = chartView;

        layout->setStretch(0, 3);
        layout->setStretch(1, 1);
        layout->setStretch(2, 1);
    }

    // Обновляем описательную статистику
    QString description = QString(
        "Общая статистика по жанрам:\n"
        "Всего игр: %1\n"
        "Всего жанров: %2\n\n"
        "Топ жанров по количеству игр:\n"
    ).arg(stats.totalGames)
     .arg(stats.totalGenres);

    // Сортируем жанры по количеству игр
    QList<QPair<QString, int>> sortedGenres;
    for (auto it = stats.gamesPerGenre.begin(); it != stats.gamesPerGenre.end(); ++it) {
        sortedGenres.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedGenres.begin(), sortedGenres.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second;
              });

    for (int i = 0; i < qMin(5, sortedGenres.size()); ++i) {
        description += QString("%1. %2: %3 игр\n")
                          .arg(i + 1)
                          .arg(sortedGenres[i].first)
                          .arg(sortedGenres[i].second);
    }

    ui->genreStatsDescription->setText(description);

    // Обновляем таблицу статистики
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Жанр", "Кол-во игр", "Кол-во бронирований", "Средний рейтинг"});

    for (auto it = stats.gamesPerGenre.begin(); it != stats.gamesPerGenre.end(); ++it) {
        QList<QStandardItem*> row;
        row << new QStandardItem(it.key());
        row << new QStandardItem(QString::number(it.value()));
        row << new QStandardItem(QString::number(stats.borrowingsPerGenre[it.key()]));
        row << new QStandardItem(QString::number(stats.averageRatingPerGenre[it.key()], 'f', 2));
        model->appendRow(row);
    }

    ui->genreStatsTable->setModel(model);
    ui->genreStatsTable->resizeColumnsToContents();
}

void AdminPanelWindow::updateBorrowingStatistics() {
    AdminController::BorrowingStatistics stats = adminController->getBorrowingStatistics();

    if (stats.totalBorrowings <= 0) {
        ui->borrowingStatsDescription->setText("Статистика бронирований:\n"
                                               "На данный момент бронирований нет.");
        return;
    }

    // График динамики бронирований
    QChart *chart = new QChart();
    QLineSeries *borrowingSeries = new QLineSeries();

    // Заполняем данные для графика
    for (auto it = stats.borrowingsPerDay.begin(); it != stats.borrowingsPerDay.end(); ++it) {
        borrowingSeries->append(it.key().startOfDay().toMSecsSinceEpoch(), it.value());
    }

    // Сначала добавляем серию к графику
    chart->addSeries(borrowingSeries);

    // Затем создаем и настраиваем оси
    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("dd.MM.yyyy");
    axisX->setTitleText("Дата");

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Количество бронирований");
    axisY->setMin(0);
    axisY->setMax(qMax(5.0, stats.totalBorrowings * 1.2)); // Увеличиваем максимум для лучшей видимости

    // Добавляем оси к графику
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // Присоединяем оси к серии
    borrowingSeries->attachAxis(axisX);
    borrowingSeries->attachAxis(axisY);

    chart->setTitle("Динамика бронирований за последние 30 дней");
    chart->legend()->hide(); // Скрываем легенду, так как у нас одна серия
    chart->setMargins(QMargins(5, 5, 5, 5));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Устанавливаем политику размера
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Заменяем старый график новым
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->borrowingChartView->parentWidget()->layout());
    if (layout) {
        // Удаляем старый виджет
        layout->removeWidget(ui->borrowingChartView);
        delete ui->borrowingChartView;

        // Добавляем новый виджет с правильными настройками размера
        layout->insertWidget(0, chartView);
        ui->borrowingChartView = chartView;

        // Устанавливаем соотношение размеров для элементов layout
        layout->setStretch(0, 3); // График (больше места)
        layout->setStretch(1, 1); // Описание
        layout->setStretch(2, 1); // Таблица
    }

    // Обновляем описание
    QString description = QString(
        "Статистика бронирований:\n"
        "Всего бронирований: %1\n"
        "Активных бронирований: %2\n"
        "Средняя продолжительность бронирования: %3 дней\n\n"
        "Топ пользователей по количеству бронирований:\n"
    ).arg(stats.totalBorrowings)
     .arg(stats.activeBorrowings)
     .arg(QString::number(stats.averageBorrowingDuration, 'f', 1));

    // Сортируем пользователей по количеству бронирований
    QList<QPair<QString, int>> sortedUsers;
    for (auto it = stats.borrowingsPerUser.begin(); it != stats.borrowingsPerUser.end(); ++it) {
        sortedUsers.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedUsers.begin(), sortedUsers.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second;
              });

    for (int i = 0; i < qMin(5, sortedUsers.size()); ++i) {
        description += QString("%1. %2: %3 бронирований\n")
                          .arg(i + 1)
                          .arg(sortedUsers[i].first)
                          .arg(sortedUsers[i].second);
    }

    ui->borrowingStatsDescription->setText(description);

    // Обновляем таблицу
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Игра", "Количество бронирований"});

    for (auto it = stats.borrowingsPerGame.begin(); it != stats.borrowingsPerGame.end(); ++it) {
        QList<QStandardItem*> row;
        row << new QStandardItem(it.key());
        row << new QStandardItem(QString::number(it.value()));
        model->appendRow(row);
    }

    ui->borrowingStatsTable->setModel(model);
    ui->borrowingStatsTable->resizeColumnsToContents();
}

void AdminPanelWindow::updateRatingStatistics() {
    AdminController::RatingStatistics stats = adminController->getRatingStatistics();

    if (stats.totalRatings <= 0) {
        ui->ratingStatsDescription->setText("Статистика рейтингов:\nНа данный момент оценок нет.");
        return;
    }

    QChart *chart = new QChart();
    QBarSeries *series = new QBarSeries();
    QBarSet *ratingSet = new QBarSet("Количество оценок");

    for (int i = 1; i <= 5; ++i) {
        *ratingSet << stats.ratingsDistribution.value(i, 0);
    }

    series->append(ratingSet);
    chart->addSeries(series);

    QStringList categories;
    for (int i = 1; i <= 5; ++i) {
        categories << QString::number(i);
    }

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("Оценка");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Количество оценок");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->setTitle("Распределение рейтингов");
    chart->setMargins(QMargins(5, 5, 5, 5));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->ratingChartView->parentWidget()->layout());
    if (layout) {
        layout->removeWidget(ui->ratingChartView);
        delete ui->ratingChartView;
        layout->insertWidget(0, chartView);
        ui->ratingChartView = chartView;

        layout->setStretch(0, 3);
        layout->setStretch(1, 1);
        layout->setStretch(2, 1);
    }

    // Обновляем описание
    QString description = QString(
        "Статистика рейтингов:\n"
        "Всего оценок: %1\n"
        "Средний рейтинг: %2\n\n"
        "Топ игр по рейтингу:\n"
    ).arg(stats.totalRatings)
     .arg(QString::number(stats.overallAverageRating, 'f', 2));

    // Сортируем игры по рейтингу
    QList<QPair<QString, double>> sortedGames;
    for (auto it = stats.averageRatingPerGame.begin(); it != stats.averageRatingPerGame.end(); ++it) {
        sortedGames.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedGames.begin(), sortedGames.end(),
              [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
                  return a.second > b.second;
              });

    for (int i = 0; i < qMin(5, sortedGames.size()); ++i) {
        description += QString("%1. %2: %3\n")
                          .arg(i + 1)
                          .arg(sortedGames[i].first)
                          .arg(QString::number(sortedGames[i].second, 'f', 2));
    }

    ui->ratingStatsDescription->setText(description);

    // Обновляем таблицу
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Игра", "Средний рейтинг", "Количество оценок"});

    for (auto it = stats.averageRatingPerGame.begin(); it != stats.averageRatingPerGame.end(); ++it) {
        QList<QStandardItem*> row;
        row << new QStandardItem(it.key());
        row << new QStandardItem(QString::number(it.value(), 'f', 2));
        row << new QStandardItem(QString::number(stats.ratingsPerUser[it.key()]));
        model->appendRow(row);
    }

    ui->ratingStatsTable->setModel(model);
    ui->ratingStatsTable->resizeColumnsToContents();
}

void AdminPanelWindow::updateUserActivityStatistics() {
    AdminController::UserActivityStatistics stats = adminController->getUserActivityStatistics();

    if (stats.activityTimeline.isEmpty()) {
        ui->userActivityDescription->setText("Статистика активности:\nНа данный момент активности нет.");
        return;
    }

    QChart *chart = new QChart();
    QLineSeries *series = new QLineSeries();

    for (auto it = stats.activityTimeline.begin(); it != stats.activityTimeline.end(); ++it) {
        series->append(it.key().toMSecsSinceEpoch(), it.value());
    }

    chart->addSeries(series);

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("HH:mm");
    axisX->setTitleText("Время");

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Количество действий");
    axisY->setMin(0);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chart->setTitle("Активность пользователей за последние 24 часа");
    chart->legend()->hide();
    chart->setMargins(QMargins(5, 5, 5, 5));

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->userActivityChartView->parentWidget()->layout());
    if (layout) {
        layout->removeWidget(ui->userActivityChartView);
        delete ui->userActivityChartView;
        layout->insertWidget(0, chartView);
        ui->userActivityChartView = chartView;

        layout->setStretch(0, 3);
        layout->setStretch(1, 1);
        layout->setStretch(2, 1);
    }

    // Обновляем описание
    QString description = "Статистика активности пользователей:\n\n";
    description += "Топ пользователей по общей активности:\n";

    // Считаем общую активность для каждого пользователя
    QMap<QString, int> totalActivity;
    for (auto it = stats.userLoginCount.begin(); it != stats.userLoginCount.end(); ++it) {
        totalActivity[it.key()] = it.value() +
                                 stats.userBorrowingCount.value(it.key(), 0) +
                                 stats.userRatingCount.value(it.key(), 0) +
                                 stats.userChatMessageCount.value(it.key(), 0);
    }

    // Сортируем пользователей по общей активности
    QList<QPair<QString, int>> sortedUsers;
    for (auto it = totalActivity.begin(); it != totalActivity.end(); ++it) {
        sortedUsers.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedUsers.begin(), sortedUsers.end(),
              [](const QPair<QString, int>& a, const QPair<QString, int>& b) {
                  return a.second > b.second;
              });

    for (int i = 0; i < qMin(5, sortedUsers.size()); ++i) {
        description += QString("%1. %2: %3 действий\n")
                          .arg(i + 1)
                          .arg(sortedUsers[i].first)
                          .arg(sortedUsers[i].second);
    }

    ui->userActivityDescription->setText(description);

    // Обновляем таблицу
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({
        "Пользователь",
        "Входы в систему",
        "Бронирования",
        "Оценки",
        "Сообщения",
        "Всего действий"
    });

    for (auto it = stats.userLoginCount.begin(); it != stats.userLoginCount.end(); ++it) {
        QList<QStandardItem*> row;
        QString username = it.key();
        row << new QStandardItem(username);
        row << new QStandardItem(QString::number(stats.userLoginCount[username]));
        row << new QStandardItem(QString::number(stats.userBorrowingCount[username]));
        row << new QStandardItem(QString::number(stats.userRatingCount[username]));
        row << new QStandardItem(QString::number(stats.userChatMessageCount[username]));
        row << new QStandardItem(QString::number(totalActivity[username]));
        model->appendRow(row);
    }

    ui->userActivityTable->setModel(model);
    ui->userActivityTable->resizeColumnsToContents();
}

void AdminPanelWindow::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    updateStatistics();
}

void AdminPanelWindow::setupMaintenanceTab() {
    maintenanceTimer = new QTimer(this);
    maintenanceTimer->setInterval(60000); // Обновление каждую минуту
    connect(maintenanceTimer, &QTimer::timeout,
            this, &AdminPanelWindow::onMaintenanceTimerTimeout);
    maintenanceTimer->start();

    // Настраиваем таблицу логов обслуживания
    QStandardItemModel* logModel = new QStandardItemModel(this);
    logModel->setHorizontalHeaderLabels({"Время", "Действие", "Результат"});
    ui->maintenanceLogTable->setModel(logModel);

    updateMaintenanceStatus();
}

void AdminPanelWindow::setupDataManagementTab() {
    // Настраиваем модели для таблиц
    activeBorrowingsModel = new QStandardItemModel(this);
    userStatsModel = new QStandardItemModel(this);
    popularGamesModel = new QStandardItemModel(this);

    // Устанавливаем заголовки для активных бронирований
    activeBorrowingsModel->setHorizontalHeaderLabels({
        "ID", "Заемщик", "Владелец", "Игра", "Начало", "Окончание"
    });
    ui->activeBorrowingsTable->setModel(activeBorrowingsModel);

    // Устанавливаем заголовки для статистики пользователей
    userStatsModel->setHorizontalHeaderLabels({
        "Пользователь", "Игр", "Отдано", "Взято", "Оценок", "Средняя оценка",
        "Отправлено", "Получено", "Последняя активность"
    });
    ui->userStatsTable->setModel(userStatsModel);

    // Устанавливаем заголовки для популярных игр
    popularGamesModel->setHorizontalHeaderLabels({
        "Название", "Владельцев", "Бронирований", "Оценок", "Средняя оценка", "Жанры"
    });
    ui->popularGamesTable->setModel(popularGamesModel);

    // Обновляем данные
    updateActiveBorrowings();
    updateUserStatistics();
    updatePopularGames();
}

void AdminPanelWindow::onMaintenanceButtonClicked() {
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Очищаем таблицу результатов
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Время", "Действие", "Результат"});
    ui->maintenanceLogTable->setModel(model);

    if (adminController->runDatabaseMaintenance()) {
        // Получаем логи обслуживания
        QSqlQuery query;
        query.exec("SELECT timestamp, action, details FROM Logs "
                   "WHERE action LIKE 'Maintenance%' "
                   "ORDER BY timestamp DESC LIMIT 10");

        while (query.next()) {
            QList<QStandardItem*> row;
            row << new QStandardItem(query.value("timestamp").toDateTime().toString("dd.MM.yyyy hh:mm:ss"))
                << new QStandardItem(query.value("action").toString())
                << new QStandardItem(query.value("details").toString());
            model->appendRow(row);
        }

        // Обновляем статистику
        query.exec("SELECT "
                   "(SELECT COUNT(*) FROM Borrowings WHERE status = 'confirmed') as active_borrowings, "
                   "pg_size_pretty(pg_database_size(current_database())) as db_size");

        if (query.next()) {
            ui->activeBorrowingsValue->setText(query.value("active_borrowings").toString());
            ui->databaseSizeValue->setText(query.value("db_size").toString());
        }

        // Обновляем информацию о последнем бэкапе
        query.exec("SELECT backup_date FROM DatabaseBackups ORDER BY backup_date DESC LIMIT 1");
        if (query.next()) {
            ui->lastBackupValue->setText(query.value(0).toDateTime().toString("dd.MM.yyyy HH:mm"));
        }

        QMessageBox::information(this, "Успех", "Обслуживание базы данных выполнено успешно");
    } else {
        QMessageBox::warning(this, "Ошибка", "Произошла ошибка при выполнении обслуживания");
    }

    ui->maintenanceLogTable->resizeColumnsToContents();
    QApplication::restoreOverrideCursor();
}

void AdminPanelWindow::onExportButtonClicked() {
    QString filter;
    switch (ui->exportTypeCombo->currentIndex()) {
    case 0:
        filter = "CSV файлы игр (*.csv);;Все файлы (*.*)";
        break;
    case 1:
        filter = "CSV файлы бронирований (*.csv);;Все файлы (*.*)";
        break;
    case 2:
        filter = "CSV файлы статистики (*.csv);;Все файлы (*.*)";
        break;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Сохранить файл", "", filter);

    if (fileName.isEmpty())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool success = false;
    switch (ui->exportTypeCombo->currentIndex()) {
    case 0:
        success = gameController->exportToCSV(fileName);
        break;
    case 1:
        success = gameController->exportBorrowingsToCSV(fileName);
        break;
    case 2:
        success = adminController->exportStatisticsToCSV(fileName);
        break;
    }

    QApplication::restoreOverrideCursor();

    if (success) {
        QMessageBox::information(this, "Успех",
                                 "Данные успешно экспортированы в " + fileName);
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Произошла ошибка при экспорте данных");
    }
}

void AdminPanelWindow::onImportButtonClicked() {
    QString filter;
    switch (ui->importTypeCombo->currentIndex()) {
    case 0:
        filter = "CSV файлы игр (*.csv);;Все файлы (*.*)";
        break;
    case 1:
        filter = "CSV файлы бронирований (*.csv);;Все файлы (*.*)";
        break;
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Открыть файл", "", filter);

    if (fileName.isEmpty())
        return;

    if (QMessageBox::question(this, "Подтверждение",
                              "Импорт данных может привести к дублированию записей. Продолжить?",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool success = false;
    switch (ui->importTypeCombo->currentIndex()) {
    case 0: // Игры
        success = gameController->importFromCSV(fileName, ui->replaceExistingCheckBox->isChecked());
        break;
    case 1: // Бронирования
        success = gameController->importBorrowingsFromCSV(fileName);
        break;
    }

    QApplication::restoreOverrideCursor();

    if (success) {
        QMessageBox::information(this, "Успех",
                                 "Данные успешно импортированы из " + fileName);
        updateAll();
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Произошла ошибка при импорте данных");
    }
}

void AdminPanelWindow::updateMaintenanceStatus() {
    QSqlQuery query;

    // Получаем информацию о последнем бэкапе
    query.exec("SELECT backup_date FROM DatabaseBackups "
               "ORDER BY backup_date DESC LIMIT 1");
    if (query.next()) {
        ui->lastBackupValue->setText(
            query.value(0).toDateTime().toString("dd.MM.yyyy hh:mm"));
    } else {
        ui->lastBackupValue->setText("Нет");
    }

    // Получаем количество активных бронирований
    query.exec("SELECT COUNT(*) FROM active_borrowings");
    if (query.next()) {
        ui->activeBorrowingsValue->setText(
            query.value(0).toString());
    }

    // Получаем размер базы данных
    query.exec("SELECT pg_size_pretty(pg_database_size(current_database()))");
    if (query.next()) {
        ui->databaseSizeValue->setText(query.value(0).toString());
    }
}

// Методы обновления таблиц данных
void AdminPanelWindow::updateActiveBorrowings() {
    activeBorrowingsModel->removeRows(0, activeBorrowingsModel->rowCount());

    QList<AdminController::ActiveBorrowing> borrowings =
        adminController->getActiveBorrowings();

    for (const auto& borrowing : borrowings) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(borrowing.borrowingId))
            << new QStandardItem(borrowing.borrowerName)
            << new QStandardItem(borrowing.lenderName)
            << new QStandardItem(borrowing.gameTitle)
            << new QStandardItem(borrowing.startDate.toString("dd.MM.yyyy"))
            << new QStandardItem(borrowing.endDate.toString("dd.MM.yyyy"));

        activeBorrowingsModel->appendRow(row);
    }
}

void AdminPanelWindow::updateUserStatistics() {
    userStatsModel->removeRows(0, userStatsModel->rowCount());

    QList<AdminController::UserStatistic> stats =
        adminController->getUserStatistics();

    for (const auto& stat : stats) {
        QList<QStandardItem*> row;
        row << new QStandardItem(stat.username)
            << new QStandardItem(QString::number(stat.ownedGames))
            << new QStandardItem(QString::number(stat.gamesLent))
            << new QStandardItem(QString::number(stat.gamesBorrowed))
            << new QStandardItem(QString::number(stat.ratingsGiven))
            << new QStandardItem(QString::number(stat.averageRating, 'f', 2))
            << new QStandardItem(QString::number(stat.messagesSent))
            << new QStandardItem(QString::number(stat.messagesReceived))
            << new QStandardItem(stat.lastActivity.toString("dd.MM.yyyy hh:mm"));

        userStatsModel->appendRow(row);
    }

    ui->userStatsTable->resizeColumnsToContents();
}

void AdminPanelWindow::updatePopularGames() {
    popularGamesModel->removeRows(0, popularGamesModel->rowCount());

    QList<AdminController::PopularGame> games =
        adminController->getPopularGames();

    for (const auto& game : games) {
        QList<QStandardItem*> row;
        row << new QStandardItem(game.title)
            << new QStandardItem(QString::number(game.ownersCount))
            << new QStandardItem(QString::number(game.timesBorrowed))
            << new QStandardItem(QString::number(game.ratingsCount))
            << new QStandardItem(QString::number(game.averageRating, 'f', 2))
            << new QStandardItem(game.genres);

        popularGamesModel->appendRow(row);
    }

    ui->popularGamesTable->resizeColumnsToContents();
}

bool AdminPanelWindow::exportStatisticsToCSV(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);

    // Экспорт статистики пользователей
    out << "User Statistics\n";
    out << "Username,Owned Games,Games Lent,Games Borrowed,Ratings Given,"
        << "Average Rating,Messages Sent,Messages Received,Last Activity\n";

    QList<AdminController::UserStatistic> userStats =
        adminController->getUserStatistics();

    for (const auto& stat : userStats) {
        out << QString("%1,%2,%3,%4,%5,%6,%7,%8,%9\n")
        .arg(stat.username)
            .arg(stat.ownedGames)
            .arg(stat.gamesLent)
            .arg(stat.gamesBorrowed)
            .arg(stat.ratingsGiven)
            .arg(stat.averageRating)
            .arg(stat.messagesSent)
            .arg(stat.messagesReceived)
            .arg(stat.lastActivity.toString("dd.MM.yyyy hh:mm"));
    }

    out << "\nPopular Games\n";
    out << "Title,Owners Count,Times Borrowed,Ratings Count,Average Rating,Genres\n";

    QList<AdminController::PopularGame> games =
        adminController->getPopularGames();

    for (const auto& game : games) {
        out << QString("%1,%2,%3,%4,%5,%6\n")
        .arg(game.title)
            .arg(game.ownersCount)
            .arg(game.timesBorrowed)
            .arg(game.ratingsCount)
            .arg(game.averageRating)
            .arg(game.genres);
    }

    file.close();
    return true;
}

void AdminPanelWindow::onMaintenanceTimerTimeout() {
    updateMaintenanceStatus();
}

void AdminPanelWindow::onDatabaseTabChanged(int index) {
    // Обновляем данные при переключении вкладок
    switch (index) {
    case 5: // Вкладка обслуживания БД
        updateMaintenanceStatus();
        break;
    case 6: // Вкладка управления данными
        updateActiveBorrowings();
        updateUserStatistics();
        updatePopularGames();
        break;
    default:
        break;
    }
}

void AdminPanelWindow::updateAll() {
    updateMaintenanceStatus();
    updateActiveBorrowings();
    updateUserStatistics();
    updatePopularGames();
    loadUsers();
    loadGenres();
    loadLogs();
    loadBackups();
    updateGenreStatistics();
    updateBorrowingStatistics();
    updateRatingStatistics();
    updateUserActivityStatistics();
}

void AdminPanelWindow::onTabChanged(int index) {
    switch(index) {
    case 4: // Статистика
        updateAll();
        break;
    case 5: // Обслуживание БД
        updateMaintenanceStatus();
        break;
    case 6: // Управление данными
        onDataViewTabChanged(ui->dataViewsTab->currentIndex());
        break;
    }
}

void AdminPanelWindow::onDataViewTabChanged(int index) {
    switch(index) {
    case 0: // Активные бронирования
        updateActiveBorrowings();
        break;
    case 1: // Статистика пользователей
        updateUserStatistics();
        break;
    case 2: // Популярные игры
        updatePopularGames();
        break;
    }
}

void AdminPanelWindow::onExportTypeChanged(int index) {
    // Обновляем доступность кнопки экспорта в зависимости от выбранного типа
    ui->exportButton->setEnabled(true);

    // Можно добавить специфические настройки для разных типов экспорта
    switch(index) {
    case 0: // Игры
        break;
    case 1: // Бронирования
        break;
    case 2: // Статистика
        break;
    }
}

void AdminPanelWindow::onImportTypeChanged(int index) {
    // Обновляем доступность кнопки импорта в зависимости от выбранного типа
    ui->importButton->setEnabled(true);

    switch(index) {
    case 0: // Игры
        break;
    case 1: // Бронирования
        break;
    }
}

void AdminPanelWindow::onActiveBorrowingDoubleClicked(const QModelIndex& index) {
    int row = index.row();
    QString borrowingId = activeBorrowingsModel->item(row, 0)->text();

    // Показываем детальную информацию о бронировании
    QSqlQuery query;
    query.prepare("SELECT * FROM active_borrowings WHERE borrowing_id = :id");
    query.bindValue(":id", borrowingId);

    if (query.exec() && query.next()) {
        QString details = QString(
                              "Бронирование №%1\n\n"
                              "Игра: %2\n"
                              "Заемщик: %3\n"
                              "Владелец: %4\n"
                              "Период: %5 - %6")
                              .arg(borrowingId)
                              .arg(query.value("game_title").toString())
                              .arg(query.value("borrower_name").toString())
                              .arg(query.value("lender_name").toString())
                              .arg(query.value("start_date").toDate().toString("dd.MM.yyyy"))
                              .arg(query.value("end_date").toDate().toString("dd.MM.yyyy"));

        QMessageBox::information(this, "Информация о бронировании", details);
    }
}

void AdminPanelWindow::onUserStatsDoubleClicked(const QModelIndex& index) {
    int row = index.row();
    QString username = userStatsModel->item(row, 0)->text();

    // Показываем детальную статистику пользователя
    QSqlQuery query;
    query.prepare("SELECT * FROM user_statistics WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        QString details = QString(
                              "Статистика пользователя %1\n\n"
                              "Игр в коллекции: %2\n"
                              "Одолжено другим: %3\n"
                              "Взято в долг: %4\n"
                              "Оставлено оценок: %5\n"
                              "Средняя оценка: %6\n"
                              "Отправлено сообщений: %7\n"
                              "Получено сообщений: %8\n"
                              "Последняя активность: %9")
                              .arg(username)
                              .arg(query.value("owned_games").toInt())
                              .arg(query.value("games_lent").toInt())
                              .arg(query.value("games_borrowed").toInt())
                              .arg(query.value("ratings_given").toInt())
                              .arg(query.value("average_rating_given").toDouble(), 0, 'f', 2)
                              .arg(query.value("messages_sent").toInt())
                              .arg(query.value("messages_received").toInt())
                              .arg(query.value("last_activity").toDateTime().toString("dd.MM.yyyy hh:mm"));

        QMessageBox::information(this, "Статистика пользователя", details);
    }
}

void AdminPanelWindow::onPopularGameDoubleClicked(const QModelIndex& index) {
    int row = index.row();
    QString gameTitle = popularGamesModel->item(row, 0)->text();

    // Показываем детальную информацию об игре
    QSqlQuery query;
    query.prepare("SELECT * FROM popular_games WHERE title = :title");
    query.bindValue(":title", gameTitle);

    if (query.exec() && query.next()) {
        QString details = QString(
                              "Информация об игре %1\n\n"
                              "Издатель: %2\n"
                              "Год выпуска: %3\n"
                              "Количество владельцев: %4\n"
                              "Количество бронирований: %5\n"
                              "Количество оценок: %6\n"
                              "Средняя оценка: %7\n"
                              "Жанры: %8")
                              .arg(gameTitle)
                              .arg(query.value("publisher").toString())
                              .arg(query.value("release_year").toInt())
                              .arg(query.value("owners_count").toInt())
                              .arg(query.value("times_borrowed").toInt())
                              .arg(query.value("ratings_count").toInt())
                              .arg(query.value("average_rating").toDouble(), 0, 'f', 2)
                              .arg(query.value("genres").toString());

        QMessageBox::information(this, "Информация об игре", details);
    }
}

void AdminPanelWindow::showError(const QString& error) {
    QMessageBox::warning(this, "Ошибка", error);
}
