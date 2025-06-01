#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <print>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "book.hpp"
#include "concepts.hpp"
#include "heterogeneous_lookup.hpp"

namespace bookdb {

template <BookContainerLike BookContainer = std::vector<Book>>
class BookDatabase {
public:
    // Type aliases
    using iterator = typename BookContainer::iterator;
    using reverse_iterator = typename BookContainer::reverse_iterator;
    using const_iterator = typename BookContainer::const_iterator;
    using value_type = typename BookContainer::value_type;
    using reference = typename BookContainer::reference;
    using AuthorContainer = std::unordered_set<std::string_view, TransparentStringHash, TransparentStringEqual>;

    BookDatabase() = default;
    BookDatabase(std::initializer_list<value_type> _l) {
        books_.insert(end(), _l);
        std::transform(begin(), end(), std::inserter(authors_, authors_.end()),
                       [](const auto &book) { return book.author; });
    }

    void Clear() {
        books_.clear();
        authors_.clear();
    }

    // Standard container interface methods

    iterator begin() noexcept { return books_.begin(); }
    iterator end() noexcept { return books_.end(); };

    const_iterator cbegin() const noexcept { return books_.begin(); }
    const_iterator cend() const noexcept { return books_.end(); };

    reverse_iterator rbegin() noexcept { return books_.rbegin(); }
    reverse_iterator rend() noexcept { return books_.rend(); };

    constexpr void PushBack(const value_type &_val) {
        authors_.insert(_val.author);
        books_.push_back(_val);
    }

    constexpr void PushBack(value_type &&_val) {
        authors_.insert(_val.author);
        books_.push_back(std::move(_val));
    }

    template <typename... Args>
    constexpr reference EmplaceBack(Args &&...args) {
        reference ref = books_.emplace_back(std::forward<Args>(args)...);
        authors_.insert(ref.author);
        return ref;
    }

    const AuthorContainer &GetAuthors() const noexcept { return authors_; }

    const BookContainer &GetBooks() const noexcept { return books_; }

    size_t size() const { return books_.size(); }

    bool HasAuthor(const std::string &author) const { return authors_.find(author) != authors_.end(); }

    // Ваш код здесь

private:
    BookContainer books_;
    AuthorContainer authors_;
};

}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::BookDatabase<std::vector<bookdb::Book>>> {
    template <typename FormatContext>
    auto format(const bookdb::BookDatabase<std::vector<bookdb::Book>> &db, FormatContext &fc) const {

        // Раскомментируйте, когда bookdb::BookDatabase поддержит интерфейсы, доступные стандартным контейнерам
        //(size/begin/...)

        format_to(fc.out(), "BookDatabase (size = {}): ", db.size());

        format_to(fc.out(), "Books:\n");
        for (const auto &book : db.GetBooks()) {
            format_to(fc.out(), "- {}\n", book);
        }

        format_to(fc.out(), "Authors:\n");
        for (const auto &author : db.GetAuthors()) {
            format_to(fc.out(), "- {}\n", author);
        }

        return fc.out();
    }

    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();  // Просто игнорируем пользовательский формат
    }
};
}  // namespace std
