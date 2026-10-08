#pragma once
#include <QDebug>
#include <stdexcept>
#include "ILibraryBackend.h"

class SafeBackend : public ILibraryBackend {
public:
    explicit SafeBackend(ILibraryBackend* inner) : inner_(inner) {}

    Result registerUser(const QString& n, const QString& e, const QString& p) override {
        return guard("register the account", [&] { return inner_->registerUser(n, e, p); });
    }
    Result login(const QString& e, const QString& p, UserInfo& out) override {
        return guard("sign in", [&] { return inner_->login(e, p, out); });
    }
    Result listBooks(int actor, const QString& q, std::vector<BookInfo>& out) override {
        auto r = guard("load the books", [&] { return inner_->listBooks(actor, q, out); });
        if (!r.ok) out.clear();                
        return r;
    }
    Result addBook(int actor, const BookInfo& b) override {
        return guard("add the book", [&] { return inner_->addBook(actor, b); });
    }
    Result updateBook(int actor, const BookInfo& b) override {
        return guard("update the book", [&] { return inner_->updateBook(actor, b); });
    }
    Result deleteBook(int actor, int id) override {
        return guard("delete the book", [&] { return inner_->deleteBook(actor, id); });
    }
    Result listUsers(int actor, const QString& q, std::vector<UserInfo>& out) override {
        auto r = guard("load the users", [&] { return inner_->listUsers(actor, q, out); });
        if (!r.ok) out.clear();
        return r;
    }
    Result deleteUser(int actor, int id) override {
        return guard("delete the user", [&] { return inner_->deleteUser(actor, id); });
    }
    Result borrowBook(int user, int book) override {
        return guard("borrow the book", [&] { return inner_->borrowBook(user, book); });
    }
    Result listActiveLoans(int actor, int user, std::vector<LoanInfo>& out) override {
        auto r = guard("load the loans", [&] { return inner_->listActiveLoans(actor, user, out); });
        if (!r.ok) out.clear();
        return r;
    }
    Result processReturn(int actor, int loan, double& fine) override {
        auto r = guard("process the return", [&] { return inner_->processReturn(actor, loan, fine); });
        if (!r.ok) fine = 0.0;
        return r;
    }

private:
    template <typename F>
    static Result guard(const char* action, F&& f) {
        try {
            return f();
        } catch (const std::out_of_range& e) {
            qWarning() << "[SafeBackend] out_of_range while trying to" << action << ":" << e.what();
            return Result::failure(QString("Could not %1: the requested item or position does not exist.").arg(action));
        } catch (const std::exception& e) {
            qWarning() << "[SafeBackend] exception while trying to" << action << ":" << e.what();
            return Result::failure(QString("Could not %1 because of an internal error. Please try again.").arg(action));
        } catch (...) {
            qWarning() << "[SafeBackend] unknown exception while trying to" << action;
            return Result::failure(QString("Could not %1 because of an unexpected error.").arg(action));
        }
    }

    ILibraryBackend* inner_;   
};
