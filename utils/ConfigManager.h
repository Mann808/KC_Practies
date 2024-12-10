#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>

class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager& instance();

    QString getValue(const QString& key, const QVariant& defaultValue = QVariant(), const QString& group = QString());
    void setValue(const QString& key, const QVariant& value, const QString& group = QString());

private:
    ConfigManager();
    ~ConfigManager();

    // Запрещаем копирование и присваивание
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    QSettings* settings;
};

#endif // CONFIGMANAGER_H
