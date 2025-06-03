#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <flat_map>
#include <iterator>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>

#include "book.hpp"
#include "book_database.hpp"
#include "concepts.hpp"

#include <print>
#include <utility>
#include <vector>

namespace bookdb {

template <typename Comparator>
using histogramMap = std::flat_map<std::string, size_t, Comparator>;
template <typename Comparator>
using genreRatingMapImpl = std::flat_map<std::string, std::pair<double, size_t>, Comparator>;
template <typename Comparator>
using genreRatingMap = std::flat_map<std::string, double, Comparator>;

template <BookContainerLike T, typename Comparator = TransparentStringLess>
auto buildAuthorHistogramFlat(const BookDatabase<T> &cont, Comparator comp = {}) {
    histogramMap<Comparator> histogram;
    auto begIt = cont.cbegin();
    while (begIt != cont.cend()) {
        auto it = histogram.find(begIt->author);
        if (it == histogram.end())
            histogram[std::string(begIt->author)] = 1;
        else
            it->second++;
        begIt++;
    }
    return histogram;
}

template <BookIterator T, typename Comparator = TransparentStringLess>
auto calculateGenreRatings(T begIt, T endIt, Comparator comp = {}) {
    // Можно решить через std::array, без использовния map, но тогда при изменении enum-а
    // нужно будет менять и функция
    genreRatingMapImpl<Comparator> genreRating;
    while (begIt != endIt) {
        std::string genreStr = bookdb::StringFromGenre(begIt->genre);
        auto it = genreRating.find(genreStr);
        if (it == genreRating.end()) {
            genreRating[std::move(genreStr)] = std::make_pair(begIt->rating, 1);
        } else {
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
    std::mt19937 generator(std::random_device{}());
    size_t min_value = 0;
    size_t max_value = cont.size() - 1;
    for (size_t i = 0; i < N; i++) {
        const size_t random_index = min_value + (generator() % (max_value - min_value + 1));
        assert(random_index <= max_value);
        results.emplace_back(*(cont.cbegin() + random_index));
    }
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
struct formatter<bookdb::genreRatingMap<bookdb::TransparentStringLess>, char> {
    template <typename FormatContext>
    auto format(const bookdb::genreRatingMap<bookdb::TransparentStringLess> &m, FormatContext &fc) const {
        format_to(fc.out(), "\n");
        for (const auto &val : m) {
            const std::string &genre_str = val.first;
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
