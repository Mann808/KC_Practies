#ifndef ADMINPANELWINDOW_H
#define ADMINPANELWINDOW_H

#include <QDialog>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include "../models/User.h"
#include "../controllers/AdminController.h"
#include <QtCharts>
#include "../controllers/GameController.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>

namespace Ui {
class AdminPanelWindow;
}

class AdminPanelWindow : public QDialog {
    Q_OBJECT

public:
    explicit AdminPanelWindow(User* currentUser, QWidget *parent = nullptr);
    ~AdminPanelWindow();

private slots:
    void onUsersTabLoaded();
    void onBlockUserButtonClicked();
    void onUnblockUserButtonClicked();
    void onGenresTabLoaded();
    void onAddGenreButtonClicked();
    void onEditGenreButtonClicked();
    void onDeleteGenreButtonClicked();
    void onLogsTabLoaded();

    void onArchiveLogsButtonClicked();
    void onDeleteLogsButtonClicked();
    void onImportLogsButtonClicked();
    void onRefreshLogsButtonClicked();

    void onCreateBackupButtonClicked();
    void onRestoreBackupButtonClicked();
    void onBackupsTabLoaded();

    void updateGenreStatistics();
    void updateBorrowingStatistics();
    void updateRatingStatistics();
    void updateUserActivityStatistics();
    void updateStatistics();

    void onMaintenanceButtonClicked();
    void onExportButtonClicked();
    void onImportButtonClicked();
    void onDatabaseTabChanged(int index);
    void onMaintenanceTimerTimeout();
    void updateMaintenanceStatus();

    void updateAll();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::AdminPanelWindow *ui;
    User *currentUser;
    AdminController *adminController;
    GameController *gameController;
    QTimer *statsUpdateTimer;
    QList<User*> usersList;
    QList<Genre> genresList;
    QList<Log> logsList;

    void setupConnections();
    void loadUsers();
    void loadGenres();
    void loadLogs();
    void loadBackups();
    void setupStatsUpdateTimer();

    QTimer* maintenanceTimer;
    QStandardItemModel* activeBorrowingsModel;
    QStandardItemModel* userStatsModel;
    QStandardItemModel* popularGamesModel;

    void setupMaintenanceTab();
    void setupDataManagementTab();
    void updateActiveBorrowings();
    void updateUserStatistics();
    void updatePopularGames();
    void updateDatabaseStatistics();
    bool exportStatisticsToCSV(const QString& fileName);

    void onTabChanged(int index);
    void onDataViewTabChanged(int index);
    void onExportTypeChanged(int index);
    void onImportTypeChanged(int index);
    void onActiveBorrowingDoubleClicked(const QModelIndex& index);
    void onUserStatsDoubleClicked(const QModelIndex& index);
    void onPopularGameDoubleClicked(const QModelIndex& index);
    void showError(const QString& error);
};

#endif // ADMINPANELWINDOW_H
