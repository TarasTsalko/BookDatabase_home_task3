#pragma once

#include <string>
#include <string_view>

namespace bookdb {

struct TransparentStringLess {
    using is_transparent = void;
    bool operator()(std::string_view lhv, const std::string &rhv) const { return lhv < rhv; }
    bool operator()(const std::string &lhv, std::string_view rhv) const { return lhv < rhv; }
    bool operator()(const std::string &lhv, const std::string &rhv) const { return lhv < rhv; }
};

struct TransparentStringEqual {};

struct TransparentStringHash {};

}  // namespace bookdb
