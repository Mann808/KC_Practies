#include "ConfigManager.h"
#include <QCoreApplication>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() {
    QString configFilePath = QCoreApplication::applicationDirPath() + "/config.ini";
    settings = new QSettings(configFilePath, QSettings::IniFormat);
}

ConfigManager::~ConfigManager() {
    delete settings;
}

QString ConfigManager::getValue(const QString& key, const QVariant& defaultValue, const QString& group) {
    if (!group.isEmpty()) {
        settings->beginGroup(group);
    }
    QVariant value = settings->value(key, defaultValue);
    if (!group.isEmpty()) {
        settings->endGroup();
    }
    return value.toString();
}

void ConfigManager::setValue(const QString& key, const QVariant& value, const QString& group) {
    if (!group.isEmpty()) {
        settings->beginGroup(group);
    }
    settings->setValue(key, value);
    if (!group.isEmpty()) {
        settings->endGroup();
    }
}
