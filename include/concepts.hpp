#pragma once

#include <compare>
#include <concepts>
#include <iterator>
#include <utility>

#include "book.hpp"

namespace bookdb {

template <typename T>
concept BookContainerLike = requires(T cont) {
    typename T::value_type;
    typename T::iterator;
    typename T::const_iterator;
    typename T::reference;
    typename T::value_type;
    typename T::reverse_iterator;
    cont.begin();
    cont.end();
    cont.cbegin();
    cont.cend();
    cont.push_back(std::declval<typename T::value_type>());
    cont.emplace_back(std::declval<typename T::value_type>());
};

template <typename T>
concept BookIterator = std::bidirectional_iterator<T>;

// TODO: уточнить у ревьювера где его можно пременить
template <typename S, typename I>
concept BookSentinel = std::sentinel_for<S, I>;

// TODO: уточнить у ревьювера где его можно пременит
template <typename P>
concept BookPredicate = requires() {
    { std::declval<Book>() < std::declval<Book>() } -> std::convertible_to<std::strong_ordering>;
};

template <typename C>
concept BookComparator = requires(C comp) {
    { comp(std::declval<Book>(), std::declval<Book>()) } -> std::convertible_to<bool>;
};

}  // namespace bookdb