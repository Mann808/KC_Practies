#include "Logger.h"
#include "logger.h"
#include <QCoreApplication>
#include <QDateTime>
#include <qfiledevice.h>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    QString logFilePath = QCoreApplication::applicationDirPath() + "/application.log";
    logFile = new QFile(logFilePath);
    if (logFile->open(QIODevice::Append | QIODevice::Text)) {
        logStream = new QTextStream(logFile);
    } else {
        qWarning() << "Не удалось открыть файл лога:" << logFilePath;
    }
}

Logger::~Logger() {
    if (logFile->isOpen()) {
        logFile->close();
    }
    delete logStream;
    delete logFile;
}

void Logger::log(const QString& level, const QString& message) {
    QMutexLocker locker(&mutex);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString logMessage = QString("[%1] [%2] %3").arg(timestamp, level, message);

    if (logStream) {
        *logStream << logMessage << "\n";
        logStream->flush();
    }

    // Также выводим в консоль
    qDebug() << logMessage;
}

void Logger::logInfo(const QString& message) {
    log("INFO", message);
}

void Logger::logWarning(const QString& message) {
    log("WARNING", message);
}

void Logger::logError(const QString& message) {
    log("ERROR", message);
}
