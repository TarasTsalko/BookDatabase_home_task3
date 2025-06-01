#pragma once

#include "book.hpp"
#include "concepts.hpp"

#include <cassert>
#include <iterator>
#include <stdexcept>

namespace bookdb {

template <typename... Predicates>
auto all_of(Predicates... preds) {
    return [=](const Book &value) { return (preds(value) && ...); };
}

template <typename... Predicates>
auto any_of(Predicates... preds) {
    return [=](const Book &value) { return (preds(value) || ...); };
}

inline auto YearBetween(int startPeriudYear, int endPeriudYear) {
    if (startPeriudYear > endPeriudYear)
        throw std::runtime_error(std::format("Invalid date range specified {} : {}\n", startPeriudYear, endPeriudYear));

    return [startPeriudYear, endPeriudYear](const Book &book) {
        return startPeriudYear < book.year && book.year < endPeriudYear;
    };
}

inline auto RatingAbove(double rating) {
    return [rating](const Book &book) { return book.rating > rating; };
}

inline auto GenreIs(const Genre genre) {
    return [genre](const Book &book) { return book.genre == genre; };
}

template <BookIterator T>
auto filterBooks(T beginIt, T endIt, auto &f) {
    std::vector<constBookRef> results;
    results.reserve(std::distance(beginIt, endIt));
    while (beginIt != endIt) {
        if (f(*beginIt))
            results.emplace_back(*beginIt);
        beginIt++;
    }
    return results;
}

}  // namespace bookdb