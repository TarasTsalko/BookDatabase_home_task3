#pragma once

#include "book.hpp"
#include "concepts.hpp"

#include <cassert>
#include <iterator>
#include <stdexcept>

namespace bookdb {

template <BookPredicate... Predicates>
auto all_of(Predicates... preds) {
    auto preds_tuple = std::make_tuple(std::forward<Predicates>(preds)...);
    return [p = std::move(preds_tuple)](const Book &value) {
        return std::apply([&](auto &&...pred) { return (pred(value) && ...); }, p);
    };
}

template <BookPredicate... Predicates>
auto any_of(Predicates... preds) {
    auto preds_tuple = std::make_tuple(std::forward<Predicates>(preds)...);
    return [p = std::move(preds_tuple)](const Book &value) {
        return std::apply([&](auto &&...pred) { return (pred(value) || ...); }, p);
    };
}

constexpr auto YearBetween(int startPeriudYear, int endPeriudYear) {
    if (startPeriudYear > endPeriudYear)
        throw std::runtime_error(std::format("Invalid date range specified {} : {}\n", startPeriudYear, endPeriudYear));

    return [startPeriudYear, endPeriudYear](const Book &book) {
        return startPeriudYear < book.year && book.year < endPeriudYear;
    };
}

constexpr auto RatingAbove(double rating) {
    return [rating](const Book &book) { return book.rating > rating; };
}

constexpr auto GenreIs(const Genre genre) {
    return [genre](const Book &book) { return book.genre == genre; };
}

template <BookIterator T>
auto filterBooks(T beginIt, BookSentinel<T> auto endIt, auto &f) {
    std::vector<ConstBookRef> results;
    results.reserve(std::distance(beginIt, endIt));
    while (beginIt != endIt) {
        if (f(*beginIt))
            results.emplace_back(*beginIt);
        beginIt++;
    }
    return results;
}

}  // namespace bookdb