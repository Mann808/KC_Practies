#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger : public QObject {
    Q_OBJECT
public:
    static Logger& instance();

    void logInfo(const QString& message);
    void logWarning(const QString& message);
    void logError(const QString& message);

private:
    Logger();
    ~Logger();

    // Запрещаем копирование и присваивание
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QFile* logFile;
    QTextStream* logStream;
    QMutex mutex;

    void log(const QString& level, const QString& message);
};

#endif // LOGGER_H
