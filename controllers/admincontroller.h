#ifndef ADMINCONTROLLER_H
#define ADMINCONTROLLER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QCryptographicHash>
#include "../models/User.h"
#include "../models/Log.h"
#include "../models/Genre.h"

class AdminController : public QObject {
    Q_OBJECT
public:
    explicit AdminController(QObject *parent = nullptr);

    void setCurrentUserId(int userId) { currentUserId = userId; }

    QList<User*> getAllUsers();
    bool blockUser(int userId);
    bool unblockUser(int userId);
    QList<Log> getAllLogs();
    bool addGenre(const QString& genreName);
    bool updateGenre(const Genre& genre);
    bool deleteGenre(int genreId);
    QList<Genre> getAllGenres();

    bool archiveLogs(const QDate& startDate, const QDate& endDate, const QString& archiveFilePath);
    bool importLogs(const QString& archiveFilePath);
    bool deleteLogs(const QDate& startDate, const QDate& endDate);
    QList<Log> getLogsByDateRange(const QDate& startDate, const QDate& endDate);

    bool createBackup(const QString& backupFilePath);
    bool restoreFromBackup(const QString& backupFilePath);
    struct BackupInfo {
        int backupId;
        QDateTime backupDate;
        QString filePath;
        QString createdBy;
    };
    QList<BackupInfo> getBackupsList();

    struct Statistics {
        QMap<QString, int> gamesPerGenre;
        QMap<QDate, int> borrowingsPerDay;
        QMap<int, int> ratingsDistribution;
        QMap<QString, int> userActivity;
    };

    Statistics getStatistics();

    struct GenreStatistics {
        QMap<QString, int> gamesPerGenre;
        QMap<QString, int> borrowingsPerGenre;
        QMap<QString, double> averageRatingPerGenre;
        int totalGames;
        int totalGenres;
    };

    struct BorrowingStatistics {
        QMap<QDate, int> borrowingsPerDay;
        QMap<QString, int> borrowingsPerUser;
        QMap<QString, int> borrowingsPerGame;
        int totalBorrowings;
        int activeBorrowings;
        double averageBorrowingDuration;
    };

    struct RatingStatistics {
        QMap<int, int> ratingsDistribution;
        QMap<QString, double> averageRatingPerGame;
        QMap<QString, int> ratingsPerUser;
        double overallAverageRating;
        int totalRatings;
    };

    struct UserActivityStatistics {
        QMap<QString, int> userLoginCount;
        QMap<QString, int> userBorrowingCount;
        QMap<QString, int> userRatingCount;
        QMap<QString, int> userChatMessageCount;
        QMap<QDateTime, int> activityTimeline;
    };

    struct ActiveBorrowing {
        int borrowingId;
        QString borrowerName;
        QString lenderName;
        QString gameTitle;
        QDate startDate;
        QDate endDate;
    };

    struct UserStatistic {
        QString username;
        int ownedGames;
        int gamesLent;
        int gamesBorrowed;
        int ratingsGiven;
        double averageRating;
        int messagesSent;
        int messagesReceived;
        QDateTime lastActivity;
    };

    struct PopularGame {
        QString title;
        int ownersCount;
        int timesBorrowed;
        int ratingsCount;
        double averageRating;
        QString genres;
    };

    QList<ActiveBorrowing> getActiveBorrowings();
    QList<UserStatistic> getUserStatistics();
    QList<PopularGame> getPopularGames();
    bool runDatabaseMaintenance();

    GenreStatistics getGenreStatistics();
    BorrowingStatistics getBorrowingStatistics();
    RatingStatistics getRatingStatistics();
    UserActivityStatistics getUserActivityStatistics();
    bool exportStatisticsToCSV(const QString& filename);

signals:
    void errorOccurred(const QString& error);

private:
    int currentUserId;
    QByteArray encryptData(const QByteArray& data, const QByteArray& key);
    QByteArray decryptData(const QByteArray& data, const QByteArray& key);
    QString createCSVRow(const QStringList& fields);

};

#endif // ADMINCONTROLLER_H
