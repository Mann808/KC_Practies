#ifndef GENRE_H
#define GENRE_H

#include <QString>
#include <QList>

class Genre {
public:
    // Конструкторы
    Genre();
    Genre(int genreId, const QString& name);

    // Геттеры и сеттеры
    int getGenreId() const;
    void setGenreId(int value);

    QString getName() const;
    void setName(const QString& value);

    // Методы
    static QList<Genre> getAllGenres();
    static Genre getGenreById(int genreId);
    static bool addGenre(const Genre& genre);
    static bool updateGenre(const Genre& genre);
    static bool deleteGenre(int genreId);

private:
    int genreId;
    QString name;
};

#endif // GENRE_H
