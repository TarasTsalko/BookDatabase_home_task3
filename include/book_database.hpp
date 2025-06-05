#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <print>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
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
    using AuthorContainer = std::unordered_set<std::string, TransparentStringHash, TransparentStringEqual>;

    BookDatabase() = default;
    BookDatabase(std::initializer_list<value_type> _l) {

        books_.reserve(_l.size());
        std::for_each(_l.begin(), _l.end(), [this](const Book &_val) { PushBack(_val); });
    }

    void Clear() {
        books_.clear();
        authors_.clear();
    }

    // Standard container interface methods

    iterator begin() noexcept { return books_.begin(); }
    iterator end() noexcept { return books_.end(); };

    const_iterator begin() const noexcept { return books_.cbegin(); }
    const_iterator end() const noexcept { return books_.cend(); };

    const_iterator cbegin() const noexcept { return books_.cbegin(); }
    const_iterator cend() const noexcept { return books_.cend(); };

    reverse_iterator rbegin() noexcept { return books_.rbegin(); }
    reverse_iterator rend() noexcept { return books_.rend(); };

    constexpr void PushBack(const value_type &_val) {
        books_.push_back(_val);
        CopyAndAdd(books_.back());
    }

    constexpr void PushBack(value_type &&_val) {
        books_.push_back(std::move(_val));
        CopyAndAdd(books_.back());
    }

    template <typename... Args>
    constexpr reference EmplaceBack(Args &&...args) {
        reference ref = books_.emplace_back(std::forward<Args>(args)...);
        CopyAndAdd(ref);
        return ref;
    }

    const AuthorContainer &GetAuthors() const noexcept { return authors_; }

    const BookContainer &GetBooks() const noexcept { return books_; }

    size_t size() const { return books_.size(); }

    bool empty() const noexcept { return books_.empty(); }

    bool HasAuthor(std::string_view author) const { return authors_.find(author) != authors_.end(); }

    // Ваш код здесь
private:
    // функция копируют информацию об авторе и обновляет ссылку в объекте типа Book
    void CopyAndAdd(Book &ref) {
        const auto insertRes = authors_.emplace(ref.author);
        ref.author = *insertRes.first;
    }

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
