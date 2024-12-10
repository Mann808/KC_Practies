#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include "views/LoginWindow.h"
#include "utils/DatabaseManager.h"
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Загрузка файла стилей
    QFile styleFile(":/styles/styles.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning() << "Не удалось загрузить файл стилей.";
    }

    // Инициализация подключения к базе данных
    DatabaseManager& dbManager = DatabaseManager::instance();
    if (!dbManager.getDB().isOpen()) {
        QMessageBox::critical(nullptr, "Ошибка подключения к БД", "Не удалось подключиться к базе данных.");
        return -1;
    }

    AuthController authController;
    authController.initializeAdmin();

    // Отображение окна авторизации
    LoginWindow loginWindow;
    loginWindow.show();

    // Добавляем таймер для автоматического обслуживания базы данных
    QTimer* maintenanceTimer = new QTimer();
    QObject::connect(maintenanceTimer, &QTimer::timeout, []() {
        QSqlQuery query;
        // Обновляем статусы бронирований
        query.exec("CALL update_borrowing_statuses()");
        // Очищаем старые логи (старше 30 дней)
        query.exec("CALL cleanup_old_logs(30)");
    });
    maintenanceTimer->start(3600000); // Запуск каждый час

    return app.exec();
}
