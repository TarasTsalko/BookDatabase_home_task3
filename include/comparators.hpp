#pragma once

#include "book.hpp"
#include <string_view>

namespace bookdb::comp {

struct LessByAuthor {
    bool operator()(const Book &lhv, const Book &rhv) const { return lhv.author < rhv.author; }
};

struct GreaterByPopularity {
    // используем >, так как поплярные книги должны находится в верху контейнера (списка)
    bool operator()(const Book &lhv, const Book &rhv) const { return lhv.read_count > rhv.read_count; }
};

struct GreaterByRating {
    bool operator()(const Book &lhv, const Book &rhv) const { return lhv.rating > rhv.rating; }
};

}  // namespace bookdb::comp