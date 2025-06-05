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
using genreRatingMapImpl = std::flat_map<bookdb::Genre, std::pair<double, size_t>, Comparator>;
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
    // Можно решить через std::array, без использовния map, но тогда при изменении enum-а
    // нужно будет менять и функция
    genreRatingMapImpl<Comparator> genreRating;
    while (begIt != endIt) {
        auto [it, inserted] = genreRating.try_emplace(begIt->genre, begIt->rating, 1);
        if (!inserted) {
            double &rating = it->second.first;
            size_t &count = it->second.second;
            rating += begIt->rating;
            count++;
        }
        begIt++;
    }

    genreRatingMap<Comparator> genreRatingOut;
    std::transform(genreRating.begin(), genreRating.end(), std::inserter(genreRatingOut, genreRatingOut.end()),
                   [](const genreRatingMapImpl<Comparator>::value_type &val) {
                       const double rating = val.second.first;
                       const size_t count = val.second.second;
                       assert(count != 0);
                       return std::make_pair(val.first, rating / count);
                   });

    return genreRatingOut;
}

template <BookContainerLike T>
auto calculateAverageRating(BookDatabase<T> &cont) {

    return std::accumulate(cont.begin(), cont.end(), 0.0,
                           [](double lhv, const auto &rhv) { return lhv + rhv.rating; }) /
           cont.size();
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
    if (cont.size() < N)
        throw std::runtime_error(std::format(
            "The number of books {} transferred exceeds the number of books in the database {}\n", N, cont.size()));
    std::stable_sort(cont.begin(), cont.end(), comp);
    return std::vector<ConstBookRef>(cont.begin(), cont.begin() + N);
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
