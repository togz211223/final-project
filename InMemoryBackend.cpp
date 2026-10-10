#include "InMemoryBackend.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <algorithm>

QByteArray InMemoryBackend::hashPassword(const QString& pw, const QByteArray& salt) {
    // Stub only. The real backend should use a slow KDF (bcrypt/argon2/pgcrypto crypt()).
    return QCryptographicHash::hash(salt + pw.toUtf8(), QCryptographicHash::Sha256);
}

InMemoryBackend::Account* InMemoryBackend::findAccount(int id) {
    for (auto& a : accounts_) if (a.info.id == id) return &a;
    return nullptr;
}
bool InMemoryBackend::isAdmin(int id) {
    auto* a = findAccount(id);
    return a && a->info.role == UserRole::ADMIN;
}
Result InMemoryBackend::requireAdmin(int actorId) {
    return isAdmin(actorId) ? Result::success() : Result::failure("You do not have permission to perform this action.");
}

Result InMemoryBackend::registerUser(const QString& name, const QString& email, const QString& password) {
    const QString e = email.trimmed().toLower();
    for (auto& a : accounts_)
        if (a.info.email == e) return Result::failure("An account with this email already exists.");
    Account acc;
    acc.info = {nextUserId_++, name.trimmed(), e, accounts_.empty() ? UserRole::ADMIN : UserRole::MEMBER, 0.0};
    acc.salt = QByteArray::number(QRandomGenerator::system()->generate64(), 16);
    acc.hash = hashPassword(password, acc.salt);
    accounts_.push_back(acc);
    return Result::success();
}

Result InMemoryBackend::login(const QString& email, const QString& password, UserInfo& out) {
    const QString e = email.trimmed().toLower();
    for (auto& a : accounts_)
        if (a.info.email == e && a.hash == hashPassword(password, a.salt)) { out = a.info; return Result::success(); }
    return Result::failure("Invalid username or password.");
}

static bool contains(const QString& hay, const QString& q) { return hay.contains(q, Qt::CaseInsensitive); }

Result InMemoryBackend::listBooks(int actorId, const QString& q, std::vector<BookInfo>& out) {
    if (auto r = requireAdmin(actorId); !r.ok) return r;
    out.clear();
    for (auto& b : books_)
        if (q.isEmpty() || contains(b.title, q) || contains(b.author, q) || contains(b.isbn, q)
            || contains(b.category, q) || QString::number(b.id) == q.trimmed())
            out.push_back(b);
    return Result::success();
}

Result InMemoryBackend::addBook(int actorId, const BookInfo& b) {
    if (auto r = requireAdmin(actorId); !r.ok) return r;
    for (auto& x : books_) if (x.id == b.id) return Result::failure("A book with this ID already exists.");
    books_.push_back(b);
    return Result::success();
}

Result InMemoryBackend::updateBook(int actorId, const BookInfo& b) {
    if (auto r = requireAdmin(actorId); !r.ok) return r;
    for (auto& x : books_) if (x.id == b.id) { x = b; return Result::success(); }
    return Result::failure("Book not found.");
}

Result InMemoryBackend::deleteBook(int actorId, int id) {
    if (auto r = requireAdmin(actorId); !r.ok) return r;
    for (auto& l : loans_) if (l.assetId == id) return Result::failure("This book is currently borrowed and cannot be deleted.");
    auto it = std::remove_if(books_.begin(), books_.end(), [&](const BookInfo& b) { return b.id == id; });
    if (it == books_.end()) return Result::failure("Book not found.");
    books_.erase(it, books_.end());
    return Result::success();
}

Result InMemoryBackend::listUsers(int actorId, const QString& q, std::vector<UserInfo>& out) {
    if (auto r = requireAdmin(actorId); !r.ok) return r;
    out.clear();
    for (auto& a : accounts_)
        if (q.isEmpty() || contains(a.info.name, q) || contains(a.info.email, q) || QString::number(a.info.id) == q.trimmed())
            out.push_back(a.info);
    return Result::success();
}

Result InMemoryBackend::deleteUser(int actorId, int userId) {
    if (auto r = requireAdmin(actorId); !r.ok) return r;
    if (actorId == userId) return Result::failure("You cannot delete your own account.");
    for (auto& l : loans_) if (l.userId == userId) return Result::failure("This user still has borrowed items. Process their returns first.");
    auto it = std::remove_if(accounts_.begin(), accounts_.end(), [&](const Account& a) { return a.info.id == userId; });
    if (it == accounts_.end()) return Result::failure("User not found.");
    accounts_.erase(it, accounts_.end());
    return Result::success();
}

Result InMemoryBackend::borrowBook(int userId, int bookId) {
    if (!findAccount(userId)) return Result::failure("User not found.");
    for (auto& b : books_) if (b.id == bookId) {
        if (b.available <= 0) return Result::failure("No copies available.");
        --b.available;
        loans_.push_back({nextLoanId_++, userId, bookId, b.title, QDate::currentDate(), QDate::currentDate().addDays(14)});
        return Result::success();
    }
    return Result::failure("Book not found.");
}

Result InMemoryBackend::listActiveLoans(int actorId, int userId, std::vector<LoanInfo>& out) {
    if (auto r = requireAdmin(actorId); !r.ok) return r;
    out.clear();
    for (auto& l : loans_) if (l.userId == userId) out.push_back(l);
    return Result::success();
}

Result InMemoryBackend::processReturn(int actorId, int loanId, double& fine) {
    if (auto r = requireAdmin(actorId); !r.ok) return r;
    auto it = std::find_if(loans_.begin(), loans_.end(), [&](const LoanInfo& l) { return l.loanId == loanId; });
    if (it == loans_.end()) return Result::failure("Loan record not found.");
    const int late = std::max<qint64>(0, it->dueDate.daysTo(QDate::currentDate()));
    fine = late * 0.50;   // Real backend: asset->calculateLateFine(late) via IFineStrategy
    if (auto* u = findAccount(it->userId)) u->info.unpaidFines += fine;
    for (auto& b : books_) if (b.id == it->assetId) ++b.available;   // Real backend: waitlist FIFO check (Story 9)
    loans_.erase(it);
    return Result::success();
}

Result InMemoryBackend::getSystemAnalytics(LibraryStats& out) {
    return Result::success();
}

Result InMemoryBackend::exportHistoryToCSV(const QString& filePath) {
    return Result::success();
}
