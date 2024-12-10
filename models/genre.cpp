#include "Genre.h"
#include "../utils/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

Genre::Genre() : genreId(-1) {}

Genre::Genre(int genreId, const QString& name)
    : genreId(genreId), name(name) {}

// Геттеры и сеттеры

int Genre::getGenreId() const {
    return genreId;
}

void Genre::setGenreId(int value) {
    genreId = value;
}

QString Genre::getName() const {
    return name;
}

void Genre::setName(const QString& value) {
    name = value;
}

// Методы

QList<Genre> Genre::getAllGenres() {
    QList<Genre> genres;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    if (!query.exec("SELECT * FROM Genres")) {
        qDebug() << "Ошибка получения списка жанров:" << query.lastError().text();
        return genres;
    }

    while (query.next()) {
        Genre genre;
        genre.setGenreId(query.value("genre_id").toInt());
        genre.setName(query.value("name").toString());
        genres.append(genre);
    }

    return genres;
}

Genre Genre::getGenreById(int genreId) {
    Genre genre;
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("SELECT * FROM Genres WHERE genre_id = :genre_id");
    query.bindValue(":genre_id", genreId);

    if (!query.exec()) {
        qDebug() << "Ошибка получения жанра по ID:" << query.lastError().text();
        return genre;
    }

    if (query.next()) {
        genre.setGenreId(query.value("genre_id").toInt());
        genre.setName(query.value("name").toString());
    } else {
        qDebug() << "Жанр с указанным ID не найден.";
    }

    return genre;
}

bool Genre::addGenre(const Genre& genre) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("INSERT INTO Genres (name) VALUES (:name)");
    query.bindValue(":name", genre.getName());

    if (!query.exec()) {
        qDebug() << "Ошибка добавления жанра:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Genre::updateGenre(const Genre& genre) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("UPDATE Genres SET name = :name WHERE genre_id = :genre_id");
    query.bindValue(":name", genre.getName());
    query.bindValue(":genre_id", genre.getGenreId());

    if (!query.exec()) {
        qDebug() << "Ошибка обновления жанра:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Genre::deleteGenre(int genreId) {
    QSqlDatabase db = DatabaseManager::instance().getDB();
    QSqlQuery query(db);

    query.prepare("DELETE FROM Genres WHERE genre_id = :genre_id");
    query.bindValue(":genre_id", genreId);

    if (!query.exec()) {
        qDebug() << "Ошибка удаления жанра:" << query.lastError().text();
        return false;
    }

    return true;
}
