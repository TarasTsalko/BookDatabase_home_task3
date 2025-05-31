#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <flat_map>
#include <iterator>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

#include "book.hpp"
#include "book_database.hpp"

#include <print>
#include <utility>

namespace bookdb {

template <typename Comparator>
using histogramMap = std::flat_map<std::string, size_t, Comparator>;

template <BookContainerLike T, typename Comparator = TransparentStringLess>
auto buildAuthorHistogramFlat(const BookDatabase<T> &cont, Comparator comp = {}) {
    histogramMap<Comparator> histogram;
    // std::flat_map<std::string, size_t, Comparator > histogram;
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

}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::histogramMap<bookdb::TransparentStringLess>, char> {
    template <typename FormatContext>
    auto format(const bookdb::histogramMap<bookdb::TransparentStringLess> &m, FormatContext &fc) const {

        for (const auto &val : m)
            format_to(fc.out(), "Author : {} works count : {}\n", val.first, val.second);
        return fc.out();
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};
}  // namespace std
