#pragma once
#include "Types.h"

// The ONLY surface the admin/auth GUI talks to. Backend (Members 1-3) implements it
// on top of DatabaseManager / LibraryManager. Every admin method receives the acting
// user's id and MUST enforce the ADMIN role itself (GUI checks are not security).
class ILibraryBackend {
public:
    virtual ~ILibraryBackend() = default;

    // --- Authentication ---
    virtual Result registerUser(const QString& name, const QString& email, const QString& password) = 0;
    virtual Result login(const QString& email, const QString& password, UserInfo& out) = 0;

    // --- Books (admin only) ---
    virtual Result listBooks(int actorId, const QString& query, std::vector<BookInfo>& out) = 0;
    virtual Result addBook(int actorId, const BookInfo& book) = 0;
    virtual Result updateBook(int actorId, const BookInfo& book) = 0;
    virtual Result deleteBook(int actorId, int bookId) = 0;

    // --- Users (admin only) ---
    virtual Result listUsers(int actorId, const QString& query, std::vector<UserInfo>& out) = 0;
    virtual Result deleteUser(int actorId, int userId) = 0;

    // --- Circulation ---
    virtual Result borrowBook(int userId, int bookId) = 0;                              // Member 3 (used by tests/demo)
    virtual Result listActiveLoans(int actorId, int userId, std::vector<LoanInfo>& out) = 0;
    virtual Result processReturn(int actorId, int loanId, double& fineCharged) = 0;     // admin only
};
