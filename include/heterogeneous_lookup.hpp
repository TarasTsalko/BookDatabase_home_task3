#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace bookdb {

struct TransparentStringLess {
    using is_transparent = void;
    bool operator()(std::string_view lhv, const std::string &rhv) const { return lhv < rhv; }
    bool operator()(const std::string &lhv, std::string_view rhv) const { return lhv < rhv; }
    bool operator()(const std::string &lhv, const std::string &rhv) const { return lhv < rhv; }
};

struct TransparentStringEqual {
    using is_transparent = void;
    auto operator()(std::string_view lhv, const std::string &rhv) const { return lhv == rhv; };
    auto operator()(const std::string &lhv, std::string_view rhv) const { return lhv == rhv; };
    auto operator()(std::string_view lhv, std::string_view rhv) const { return lhv == rhv; }
};

struct TransparentStringHash {
    using is_transparent = void;
    std::size_t operator()(const std::string &obj) const { return std::hash<std::string>()(obj); };
    std::size_t operator()(std::string_view obj) const { return std::hash<std::string_view>()(obj); };
};

}  // namespace bookdb
