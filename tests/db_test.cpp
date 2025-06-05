#include "book.hpp"
#include <cstddef>
#include <gtest/gtest.h>

#include "comparators.hpp"
#include "filters.hpp"
#include "statsistics.hpp"
#include <book_database.hpp>
#include <string>

using namespace bookdb;
using BookContainer = std::vector<Book>;

auto InitDataBase() {
    // Create a book database
    BookDatabase<BookContainer> db;

    EXPECT_EQ(db.size(), 0);
    db.EmplaceBack("1984", "George Orwell", 1949, Genre::SciFi, 4., 190);
    db.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);
    db.EmplaceBack("The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120);
    db.EmplaceBack("To Kill a Mockingbird", "Harper Lee", 1960, Genre::Fiction, 4.8, 156);
    return db;
}

TEST(TesDataBaseMethods, EmplaceBackTest) {
    BookDatabase<BookContainer> db;
    EXPECT_EQ(db.size(), 0);
    db = InitDataBase();
    EXPECT_EQ(db.size(), 4);
    auto ref = db.EmplaceBack("Pride and Prejudice", "Jane Austen", 1813, Genre::Fiction, 4.7, 178);
    EXPECT_EQ(ref.title, "Pride and Prejudice");
    EXPECT_EQ(ref.genre, Genre::Fiction);
    EXPECT_EQ(ref.author, "Jane Austen");
    EXPECT_DOUBLE_EQ(ref.rating, 4.7);
    EXPECT_EQ(ref.read_count, 178);
    EXPECT_EQ(db.size(), 5);
}

TEST(TesDataBaseMethods, IteratorTest) {
    const auto db = InitDataBase();
    EXPECT_EQ(db.size(), 4);
    auto ref = *(db.cbegin() + 1);
    EXPECT_EQ(ref.title, "Animal Farm");
    EXPECT_EQ(ref.author, "George Orwell");

    ref = *(db.cbegin() + 3);
    EXPECT_EQ(ref.title, "To Kill a Mockingbird");
    EXPECT_EQ(ref.genre, Genre::Fiction);
    EXPECT_EQ(ref.author, "Harper Lee");
    EXPECT_EQ(ref.year, 1960);
    EXPECT_DOUBLE_EQ(ref.rating, 4.8);
    EXPECT_EQ(ref.read_count, 156);
    EXPECT_EQ(db.cbegin() + db.size(), db.cend());
}

TEST(TesDataBaseMethods, ClearMethodTest) {
    auto db = InitDataBase();
    EXPECT_EQ(db.size(), 4);
    db.Clear();
    EXPECT_EQ(db.size(), 0);
    db = InitDataBase();
    EXPECT_EQ(db.size(), 4);
    EXPECT_FALSE(db.HasAuthor("Jane Austen"));
    EXPECT_TRUE(db.HasAuthor("F. Scott Fitzgerald"));
    db.EmplaceBack("Pride and Prejudice", "Jane Austen", 1813, Genre::Fiction, 4.7, 178);
    EXPECT_TRUE(db.HasAuthor("Jane Austen"));
    db.Clear();
    EXPECT_FALSE(db.HasAuthor("F. Scott Fitzgerald"));
    EXPECT_FALSE(db.HasAuthor("Jane Austen"));
}

TEST(TesDataBaseMethods, GetBooksMethodTest) {
    auto db = InitDataBase();
    const auto &books = db.GetBooks();
    EXPECT_EQ(books.size(), db.size());
    const Book book("1984", "George Orwell", 1949, Genre::SciFi, 4., 190);
    EXPECT_TRUE(std::find(books.cbegin(), books.cend(), book) != books.cend());
}

TEST(TesDataBaseMethods, GetAuthorsMethodTest) {
    auto db = InitDataBase();
    const auto &authors = db.GetAuthors();
    EXPECT_TRUE(authors.find(std::string_view("F. Scott Fitzgerald")) != authors.end());
    EXPECT_FALSE(authors.find(std::string_view("Thomas Mayne Reid")) != authors.end());
    db.Clear();
    EXPECT_TRUE(authors.empty());
}

TEST(TesDataBase, SortByAuthorTest) {
    auto db = InitDataBase();
    // Sorts
    std::sort(db.begin(), db.end(), comp::LessByAuthor{});
    EXPECT_EQ(db.begin()->author, "F. Scott Fitzgerald");
    EXPECT_EQ(db.rbegin()->author, "Harper Lee");
}

TEST(TesDataBase, SortByPopularity) {
    auto db = InitDataBase();
    // Sorts
    std::sort(db.begin(), db.end(), comp::GreaterByPopularity{});
    EXPECT_EQ(db.begin()->title, "1984");
    EXPECT_EQ(db.rbegin()->title, "The Great Gatsby");
}

TEST(TesDataBase, buildAuthorHistogramFlatTest) {
    auto db = InitDataBase();
    auto histogram = buildAuthorHistogramFlat(db);
    EXPECT_EQ(histogram.begin()->first, "F. Scott Fitzgerald");
    EXPECT_EQ(histogram.begin()->second, 1);
    const auto it = std::next(histogram.begin());
    EXPECT_EQ(it->first, "George Orwell");
    EXPECT_EQ(it->second, 2);
}

TEST(TesDataBase, buildAuthorHistogramFlatByEmptyDataBaseTest) {
    auto db = InitDataBase();
    db.Clear();
    auto histogram = buildAuthorHistogramFlat(db);
    EXPECT_TRUE(histogram.empty());
}

TEST(TesDataBase, CalculateGenreRatingsByEmptyDataBaseTest) {
    auto db = InitDataBase();
    db.Clear();
    auto genreRatings = calculateGenreRatings(db.begin(), db.end());
    EXPECT_TRUE(genreRatings.empty());
}

TEST(TesDataBase, CalculateGenreRatingsTest) {
    auto db = InitDataBase();
    auto genreRatings = calculateGenreRatings(db.begin(), db.end());
    EXPECT_EQ(genreRatings.begin()->first, Genre::Fiction);
    EXPECT_DOUBLE_EQ(genreRatings.begin()->second, 4.5666666666666664);
    EXPECT_EQ(genreRatings.size(), 2);
}

TEST(TesDataBase, FilterWith_ALL_Of_Test) {
    auto db = InitDataBase();
    auto comp = all_of(YearBetween(1900, 1999), RatingAbove(4.5));
    auto filtered = filterBooks(db.begin(), db.end(), comp);
    EXPECT_EQ(filtered.size(), 1);
    EXPECT_EQ(filtered.begin()->get().author, "Harper Lee");
    EXPECT_EQ(filtered.begin()->get().title, "To Kill a Mockingbird");
}

TEST(TesDataBase, FilterWith_Any_Of_Test) {

    auto db = InitDataBase();
    auto comp = any_of(YearBetween(1900, 1999), RatingAbove(4.5));
    auto filtered = filterBooks(db.begin(), db.end(), comp);
    EXPECT_EQ(filtered.size(), 4);
    EXPECT_EQ(filtered.begin()->get().author, "George Orwell");
    EXPECT_EQ(filtered.begin()->get().title, "1984");

    EXPECT_EQ(filtered.rbegin()->get().author, "Harper Lee");
    EXPECT_EQ(filtered.rbegin()->get().title, "To Kill a Mockingbird");
}

TEST(TesDataBase, SampleRandomBooksTest) {

    const size_t N = 3;
    auto db = InitDataBase();
    auto randomBooks = sampleRandomBooks(db, N);
    EXPECT_EQ(randomBooks.size(), N);
}

TEST(TesDataBase, GetTopNByTest) {

    const size_t N = 2;
    auto db = InitDataBase();
    auto topBooks = getTopNBy(db, N, comp::GreaterByRating{});
    EXPECT_EQ(topBooks.size(), N);
    EXPECT_EQ(topBooks.begin()->get().author, "Harper Lee");
    EXPECT_EQ(topBooks.begin()->get().title, "To Kill a Mockingbird");

    EXPECT_EQ(topBooks.rbegin()->get().author, "F. Scott Fitzgerald");
    EXPECT_EQ(topBooks.rbegin()->get().title, "The Great Gatsby");
}

TEST(ConstructorDataOwnershipVerificationTest, ConstructorDataOwnershipVerificationTest) {
    std::string AuthorNameOne = "AuthorNameOne";
    std::string AuthorNameTwo = "AuthorNameTwo";
    BookDatabase<BookContainer> db{
        {"title1", AuthorNameOne, 1, Genre::Biography, 4.1, 210},
        {"title2", AuthorNameTwo, 2, Genre::Fiction, 4.2, 220},
        {"title3", AuthorNameOne, 3, Genre::NonFiction, 4.3, 230},
        {"title4", AuthorNameTwo, 4, Genre::Mystery, 4.4, 240},
        {"title5", AuthorNameOne, 5, Genre::SciFi, 4.5, 250},
        {"title6", AuthorNameTwo, 6, Genre::SciFi, 4.6, 260},
    };

    auto iter = db.cbegin();
    EXPECT_EQ(AuthorNameOne, iter->author);
    EXPECT_EQ(AuthorNameTwo, std::next(iter)->author);
    AuthorNameOne = "AuthorNameOne_and_Suname";
    AuthorNameTwo = "AuthorNameTwo_and_Suname";
    EXPECT_NE(AuthorNameOne, iter->author);
    EXPECT_NE(AuthorNameTwo, std::next(iter)->author);
}

TEST(PushBackDataOwnershipVerificationTest, PushBackDataOwnershipVerificationTest) {
    BookDatabase<BookContainer> db;
    std::string AuthorNameOne = "AuthorNameOne";
    std::string AuthorNameTwo = "AuthorNameTwo";
    Book book1{"title1", AuthorNameOne, 1, Genre::Biography, 4.1, 210};
    Book book2{"title1", AuthorNameTwo, 1, Genre::Biography, 4.2, 220};
    db.PushBack(book1);
    db.PushBack(book2);
    auto iter = db.cbegin();
    EXPECT_EQ(AuthorNameOne, iter->author);
    EXPECT_EQ(AuthorNameTwo, std::next(iter)->author);
    AuthorNameOne = "AuthorNameOne_and_Suname";
    AuthorNameTwo = "AuthorNameTwo_and_Suname";
    EXPECT_NE(AuthorNameOne, iter->author);
    EXPECT_NE(AuthorNameTwo, std::next(iter)->author);

    // проверяем PushBack для правосторонней ссылки
    AuthorNameOne = "AuthorNameOne";
    AuthorNameTwo = "AuthorNameTwo";

    // так как book1 и book2 хронят имена авторов как string_view, то переприсваеваем значения
    book1.author = AuthorNameOne;
    book2.author = AuthorNameTwo;
    db.Clear();
    db.PushBack(std::move(book1));
    db.PushBack(std::move(book2));
    iter = db.cbegin();
    EXPECT_EQ(AuthorNameOne, iter->author);
    EXPECT_EQ(AuthorNameTwo, std::next(iter)->author);
    AuthorNameOne = "AuthorNameOne_and_Suname";
    AuthorNameTwo = "AuthorNameTwo_and_Suname";
    EXPECT_NE(AuthorNameOne, iter->author);
    EXPECT_NE(AuthorNameTwo, std::next(iter)->author);
}

TEST(EmplaceBackkDataOwnershipVerificationTest, EmplaceBackDataOwnershipVerificationTest) {
    BookDatabase<BookContainer> db;
    std::string AuthorNameOne = "AuthorNameOne";
    std::string AuthorNameTwo = "AuthorNameTwo";
    Book book1{"title1", AuthorNameOne, 1, Genre::Biography, 4.1, 210};
    Book book2{"title1", AuthorNameTwo, 1, Genre::Biography, 4.2, 220};
    db.EmplaceBack("title1", AuthorNameOne, 1, Genre::Biography, 4.1, 210);
    db.EmplaceBack("title1", AuthorNameTwo, 1, Genre::Biography, 4.2, 220);

    auto iter = db.cbegin();
    EXPECT_EQ(AuthorNameOne, iter->author);
    EXPECT_EQ(AuthorNameTwo, std::next(iter)->author);
    AuthorNameOne = "AuthorNameOne_and_Suname";
    AuthorNameTwo = "AuthorNameTwo_and_Suname";
    EXPECT_NE(AuthorNameOne, iter->author);
    EXPECT_NE(AuthorNameTwo, std::next(iter)->author);
}