#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <flat_map>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>

#include "book.hpp"
#include "book_database.hpp"
#include "comparators.hpp"
#include "concepts.hpp"

#include <print>
#include <utility>
#include <vector>

namespace bookdb {

template <typename Comparator>
using histogramMap = std::flat_map<std::string, size_t, Comparator>;
template <typename Comparator>
using genreRatingMap = std::flat_map<bookdb::Genre, double, Comparator>;

template <BookContainerLike T, typename Comparator = TransparentStringLess>
auto buildAuthorHistogramFlat(const BookDatabase<T> &cont, Comparator comp = {}) {
    histogramMap<Comparator> histogram;
    for (const auto &val : cont) {
        auto [it, inserted] = histogram.try_emplace(std::string(val.author), 1);
        if (!inserted)
            it->second++;
    }
    return histogram;
}

template <BookIterator T, typename Comparator = comp::LessByGenre>
auto calculateGenreRatings(T begIt, T endIt, Comparator comp = {}) {
    class GenreStats {
    public:
        GenreStats(double _rating, size_t _count) : total_rating(_rating), count(_count) {}

        void Add(double rating) {
            total_rating += rating;
            count++;
        }
        double Average() const { return count != 0 ? total_rating / count : 0.0; }

    private:
        double total_rating = 0.0;
        size_t count = 0;
    };

    using genreRatingMapImpl = std::flat_map<bookdb::Genre, GenreStats, Comparator>;
    genreRatingMapImpl genreRating;
    while (begIt != endIt) {
        auto [it, inserted] = genreRating.try_emplace(begIt->genre, GenreStats{begIt->rating, 1});
        if (!inserted) {
            it->second.Add(begIt->rating);
        }
        begIt++;
    }

    genreRatingMap<Comparator> genreRatingOut;
    std::transform(
        genreRating.begin(), genreRating.end(), std::inserter(genreRatingOut, genreRatingOut.end()),
        [](const genreRatingMapImpl::value_type &val) { return std::make_pair(val.first, val.second.Average()); });

    return genreRatingOut;
}

template <BookContainerLike T>
auto calculateAverageRating(BookDatabase<T> &cont) {

    const double sum = std::transform_reduce(cont.begin(), cont.end(), 0.0, std::plus<>(),
                                             [](const auto &item) { return item.rating; });
    return !cont.empty() ? sum / cont.size() : 0.0;
}

template <BookContainerLike T>
    requires BookIterator<typename T::const_iterator>
auto sampleRandomBooks(const BookDatabase<T> &cont, size_t N) {
    if (cont.size() < N)
        throw std::runtime_error(std::format(
            "The number of books {} transferred exceeds the number of books in the database {}\n", N, cont.size()));

    std::vector<ConstBookRef> results;
    results.reserve(N);
    std::sample(cont.begin(), cont.end(), std::back_inserter(results), N, std::mt19937{std::random_device{}()});
    return results;
}

template <BookContainerLike T, BookComparator Comparator>
auto getTopNBy(BookDatabase<T> &cont, size_t N, Comparator comp) {
    const size_t size = cont.size();
    if (size < N)
        throw std::runtime_error(std::format(
            "The number of books {} transferred exceeds the number of books in the database {}\n", N, cont.size()));

    // если количество книг, которые нужно "извлечь" сильно меньше чем книг в базе
    if (std::log(size) > 2 * N) {
        std::nth_element(cont.begin(), cont.begin() + N, cont.end(), comp);
        auto topN = std::vector<ConstBookRef>(cont.begin(), cont.begin() + N);
        std::stable_sort(topN.begin(), topN.end(), comp);
        return topN;
    } else {
        std::stable_sort(cont.begin(), cont.end(), comp);
        return std::vector<ConstBookRef>(cont.begin(), cont.begin() + N);
    }
}

}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::histogramMap<bookdb::TransparentStringLess>, char> {
    template <typename FormatContext>
    auto format(const bookdb::histogramMap<bookdb::TransparentStringLess> &m, FormatContext &fc) const {

        format_to(fc.out(), "\n");
        for (const auto &val : m)
            format_to(fc.out(), "Author : {} works count : {}\n", val.first, val.second);
        return fc.out();
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

template <>
struct formatter<bookdb::genreRatingMap<bookdb::comp::LessByGenre>, char> {
    template <typename FormatContext>
    auto format(const bookdb::genreRatingMap<bookdb::comp::LessByGenre> &m, FormatContext &fc) const {
        format_to(fc.out(), "\n");
        for (const auto &val : m) {
            const std::string genre_str = bookdb::StringFromGenre(val.first);
            const double rating = val.second;
            format_to(fc.out(), "Genre : {} avg rating : {}\n", genre_str, rating);
        }
        return fc.out();
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};

}  // namespace std
