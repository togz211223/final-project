#pragma once
#include "ILibraryBackend.h"
#include <QSqlDatabase>

// ============================================================
// SINGLETON DESIGN PATTERN
// ============================================================
class PostgresBackend : public ILibraryBackend {
public:

    PostgresBackend();
    ~PostgresBackend();


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
    QSqlDatabase db;
    bool requireAdmin(int actorId); // Security Helper
};