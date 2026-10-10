#pragma once
#include "ILibraryBackend.h"

// TEMPORARY stand-in so the GUI runs before PostgreSQL is wired up.
// Starts EMPTY (no default credentials). The first account registered becomes ADMIN;
// later registrations are MEMBER. Replace with a PostgreSQL-backed implementation.
class InMemoryBackend : public ILibraryBackend {
public:
    Result registerUser(const QString& name, const QString& email, const QString& password) override;
    Result login(const QString& email, const QString& password, UserInfo& out) override;
    Result listBooks(int actorId, const QString& query, std::vector<BookInfo>& out) override;
    Result addBook(int actorId, const BookInfo& book) override;
    Result updateBook(int actorId, const BookInfo& book) override;
    Result deleteBook(int actorId, int bookId) override;
    Result listUsers(int actorId, const QString& query, std::vector<UserInfo>& out) override;
    Result deleteUser(int actorId, int userId) override;
    Result borrowBook(int userId, int bookId) override;
    Result listActiveLoans(int actorId, int userId, std::vector<LoanInfo>& out) override;
    Result processReturn(int actorId, int loanId, double& fineCharged) override;
    Result getSystemAnalytics(LibraryStats& out) override;
    Result exportHistoryToCSV(const QString& filePath) override;

private:
    struct Account { UserInfo info; QByteArray salt, hash; };
    static QByteArray hashPassword(const QString& pw, const QByteArray& salt);
    Account* findAccount(int id);
    bool isAdmin(int id);
    Result requireAdmin(int actorId);

    std::vector<Account> accounts_;
    std::vector<BookInfo> books_;
    std::vector<LoanInfo> loans_;
    int nextUserId_ = 1, nextLoanId_ = 1;
};
