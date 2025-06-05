#pragma once

#include <flat_map>
#include <format>
#include <stdexcept>
#include <string_view>

namespace bookdb {

enum class Genre { Fiction, NonFiction, SciFi, Biography, Mystery, Unknown };

constexpr Genre GenreFromString(std::string_view s) {

    using bookdb::Genre;
    const std::flat_map<std::string_view, bookdb::Genre> m = {{"Fiction", Genre::Fiction},
                                                              {"NonFiction", Genre::NonFiction},
                                                              {"SciFi", Genre::SciFi},
                                                              {"Biography", Genre::Biography},
                                                              {"Mystery", Genre::Mystery}};
    if (auto it = m.find(s); it != m.end())
        return it->second;
    return Genre::Unknown;
}

constexpr std::string StringFromGenre(Genre g) {
    std::string genre_str;
    // clang-format off
    using bookdb::Genre;
    switch (g) {
        case Genre::Fiction:    genre_str = "Fiction"; break;
        case Genre::Mystery:    genre_str = "Mystery"; break;
        case Genre::NonFiction: genre_str = "NonFiction"; break;
        case Genre::SciFi:      genre_str = "SciFi"; break;
        case Genre::Biography:  genre_str = "Biography"; break;
        case Genre::Unknown:    genre_str = "Unknown"; break;
        default:
            throw std::logic_error{"Unsupported bookdb::Genre"};
        }
    // clang-format on
    return genre_str;
}

struct Book {
    // string_view для экономии памяти, чтобы ссылаться на оригинальную строку, хранящуюся в другом контейнере
    std::string_view author;
    std::string title;

    int year;
    Genre genre;
    double rating;
    int read_count;

    auto operator<=>(const Book &rhv) const = default;

    // Ваш код для конструкторов здесь
    constexpr Book(const std::string &title_, std::string_view author_, int year_, Genre genre_, double rating_,
                   int read_count_)
        : author(author_), title(title_), year(year_), genre(genre_), rating(rating_), read_count(read_count_) {}

    constexpr Book(const std::string &title_, std::string_view author_, int year_, std::string_view genre_,
                   double rating_, int read_count_)
        : author(author_), title(title_), year(year_), genre(GenreFromString(genre_)), rating(rating_),
          read_count(read_count_) {}
};

using ConstBookRef = std::reference_wrapper<const Book>;
}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::Genre, char> {
    template <typename FormatContext>
    auto format(const bookdb::Genre g, FormatContext &fc) const {
        const std::string genre_str = bookdb::StringFromGenre(g);
        return format_to(fc.out(), "{}", genre_str);
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

// Ваш код для std::formatter<Book> здесь
template <>
struct formatter<bookdb::Book, char> {
    template <typename FormatContext>
    auto format(const bookdb::Book b, FormatContext &fc) const {
        const std::string_view format_template = "title : {}, Author : {}, year : {}, "
                                                 "genre : {}, rating : {}, read_count : {} ";

        // genre_str приходится создать, так как std::make_format_arg "требует"  ссылку не на времменый объект, как я понял
        const std::string genre_str = StringFromGenre(b.genre);
        return format_to(fc.out(), "{}",
                         std::vformat(format_template, std::make_format_args(b.title, b.author, b.year, genre_str,
                                                                             b.rating, b.read_count)));
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

}  // namespace std
