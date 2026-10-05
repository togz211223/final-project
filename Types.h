#pragma once
// Shared DTOs between the GUI and the backend.
// INTEGRATION: when the template is split into headers, replace UserRole below
// with an #include of the real enum (do not keep two copies).
#include <QString>
#include <QDate>
#include <vector>

enum class UserRole { ADMIN, MEMBER };

struct Result {
    bool ok = false;
    QString message;
    static Result success(const QString& m = {}) { return {true, m}; }
    static Result failure(const QString& m)       { return {false, m}; }
};

struct UserInfo {
    int id = 0;
    QString name;
    QString email;
    UserRole role = UserRole::MEMBER;
    double unpaidFines = 0.0;
};

struct BookInfo {
    int id = 0;
    QString title, author, category, isbn;
    int quantity = 0;
    int available = 0;
};

struct LoanInfo {
    int loanId = 0;
    int userId = 0;
    int assetId = 0;
    QString assetTitle;
    QDate borrowDate;
    QDate dueDate;
};
