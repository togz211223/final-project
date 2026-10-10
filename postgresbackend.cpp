#include "PostgresBackend.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDate>
#include <iostream>
#include <QFile>
#include <QTextStream>

PostgresBackend::PostgresBackend() {
    // 1. Establish the connection to PostgreSQL on port 5432
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setPort(5432);
    db.setDatabaseName("smart_lib");
    db.setUserName("postgres");

    // REPLACE THIS with your actual postgres password!
    db.setPassword("12344321");

    if (!db.open()) {
        std::cerr << "FATAL: Could not connect to Postgres! "
                  << db.lastError().text().toStdString() << std::endl;
    } else {
        std::cout << "SUCCESS: Connected to Real PostgreSQL Database." << std::endl;
    }
}

PostgresBackend::~PostgresBackend() {
    db.close();
}

// Security Guard to stop Members from doing Admin actions
bool PostgresBackend::requireAdmin(int actorId) {
    QSqlQuery q;
    q.prepare("SELECT role_id FROM Users WHERE user_id = :id");
    q.bindValue(":id", actorId);
    if(q.exec() && q.next()) {
        return q.value(0).toInt() == 1; // 1 = Admin
    }
    return false;
}

// =========================================================
// AUTHENTICATION & USERS (CRUD KAN-6)
// =========================================================

Result PostgresBackend::registerUser(const QString& name, const QString& email, const QString& password) {
    QSqlQuery q;
    q.prepare("SELECT 1 FROM Users WHERE email = :email");
    q.bindValue(":email", email);
    q.exec();
    if(q.next()) return Result::failure("An account with this email already exists.");

    // Everyone who registers from GUI is a MEMBER (role=2)
    q.prepare("INSERT INTO Users (name, email, password_hash, role_id) VALUES (:n, :e, :p, 2)");
    q.bindValue(":n", name);
    q.bindValue(":e", email);
    q.bindValue(":p", password); // Simple store for prototype

    if(!q.exec()) return Result::failure("DB Error: " + q.lastError().text());
    return Result::success();
}

Result PostgresBackend::login(const QString& email, const QString& password, UserInfo& out) {
    QSqlQuery q;
    // Protect against SQL injection with bindValue!
    q.prepare("SELECT user_id, name, email, role_id, unpaid_fines FROM Users WHERE email = :email AND password_hash = :pass");
    q.bindValue(":email", email.trimmed().toLower());
    q.bindValue(":pass", password);

    if(q.exec() && q.next()) {
        out.id = q.value(0).toInt();
        out.name = q.value(1).toString();
        out.email = q.value(2).toString();
        out.role = (q.value(3).toInt() == 1) ? UserRole::ADMIN : UserRole::MEMBER;
        out.unpaidFines = q.value(4).toDouble();
        return Result::success();
    }
    return Result::failure("Invalid username or password.");
}

Result PostgresBackend::listUsers(int actorId, const QString& query, std::vector<UserInfo>& out) {
    if(!requireAdmin(actorId)) return Result::failure("Unauthorized.");

    out.clear();
    QSqlQuery q("SELECT user_id, name, email, role_id, unpaid_fines FROM Users");
    while(q.next()) {
        UserInfo u;
        u.id = q.value(0).toInt();
        u.name = q.value(1).toString();
        u.email = q.value(2).toString();
        u.role = (q.value(3).toInt() == 1) ? UserRole::ADMIN : UserRole::MEMBER;
        u.unpaidFines = q.value(4).toDouble();
        out.push_back(u);
    }
    return Result::success();
}


Result PostgresBackend::deleteUser(int actorId, int userId) {
    if (!requireAdmin(actorId)) return Result::failure("Unauthorized. Only librarians can delete accounts.");
    if (actorId == userId) return Result::failure("Action Denied: You cannot delete your own active administrator account.");

    // EDGE CASE 1: Debt Guard - Block deleting users who owe money
    QSqlQuery checkFines;
    checkFines.prepare("SELECT unpaid_fines FROM Users WHERE user_id = :uid");
    checkFines.bindValue(":uid", userId);
    if (checkFines.exec() && checkFines.next()) {
        double debt = checkFines.value(0).toDouble();
        if (debt > 0.0) {
            return Result::failure(QString("Action Denied! This user owes $%1 in unpaid fines. Outstanding balance must be settled before deletion.")
                                       .arg(debt, 0, 'f', 2));
        }
    }

    // EDGE CASE 2: Active Loan Guard - Block deleting users who have unreturned items
    QSqlQuery checkLoans;
    checkLoans.prepare("SELECT COUNT(*) FROM Borrow_Records WHERE user_id = :uid AND return_date IS NULL");
    checkLoans.bindValue(":uid", userId);
    if (checkLoans.exec() && checkLoans.next() && checkLoans.value(0).toInt() > 0) {
        return Result::failure("Action Denied! User still has active loans. All library assets must be returned first.");
    }

    // Safe to delete
    QSqlQuery q;
    q.prepare("DELETE FROM Users WHERE user_id = :uid");
    q.bindValue(":uid", userId);
    return q.exec() ? Result::success() : Result::failure("Database error while deleting user record.");
}


// =========================================================
// ASSETS & BOOKS (CRUD KAN-6)
// =========================================================

Result PostgresBackend::listBooks(int actorId, const QString& query, std::vector<BookInfo>& out) {
    out.clear();
    QSqlQuery q("SELECT asset_id, title, author, category, isbn, quantity, available FROM Assets WHERE type='Book'");
    while(q.next()) {
        BookInfo b;
        b.id = q.value(0).toInt();
        b.title = q.value(1).toString();
        b.author = q.value(2).toString();
        b.category = q.value(3).toString();
        b.isbn = q.value(4).toString();
        b.quantity = q.value(5).toInt();
        b.available = q.value(6).toInt();
        out.push_back(b);
    }
    return Result::success();
}

Result PostgresBackend::addBook(int actorId, const BookInfo& b) {
    if(!requireAdmin(actorId)) return Result::failure("Unauthorized.");
    QSqlQuery q;
    q.prepare("INSERT INTO Assets (title, author, type, category, isbn, quantity, available) VALUES (:t, :a, 'Book', :c, :i, :q, :av)");
    q.bindValue(":t", b.title);
    q.bindValue(":a", b.author);
    q.bindValue(":c", b.category);
    q.bindValue(":i", b.isbn);
    q.bindValue(":q", b.quantity);
    q.bindValue(":av", b.available);

    return q.exec() ? Result::success() : Result::failure("Failed to add to database.");
}

Result PostgresBackend::updateBook(int actorId, const BookInfo& b) {
    if (!requireAdmin(actorId)) return Result::failure("Unauthorized.");

    // EDGE CASE 4: Negative Inventory Guard - Total copies cannot be less than borrowed copies
    QSqlQuery checkCurrent;
    checkCurrent.prepare("SELECT quantity, available FROM Assets WHERE asset_id = :id");
    checkCurrent.bindValue(":id", b.id);
    if (checkCurrent.exec() && checkCurrent.next()) {
        int oldTotal = checkCurrent.value(0).toInt();
        int oldAvailable = checkCurrent.value(1).toInt();
        int currentlyBorrowed = oldTotal - oldAvailable;

        if (b.quantity < currentlyBorrowed) {
            return Result::failure(QString("Action Denied! Cannot reduce total copies to %1. %2 copy/copies are currently on loan.")
                                       .arg(b.quantity).arg(currentlyBorrowed));
        }
    }

    QSqlQuery q;
    q.prepare("UPDATE Assets SET title=:t, author=:a, category=:c, isbn=:i, quantity=:q, available=:av WHERE asset_id=:id");
    q.bindValue(":t", b.title);
    q.bindValue(":a", b.author);
    q.bindValue(":c", b.category);
    q.bindValue(":i", b.isbn);
    q.bindValue(":q", b.quantity);
    q.bindValue(":av", b.available);
    q.bindValue(":id", b.id);
    return q.exec() ? Result::success() : Result::failure("Database error while updating book.");
}

Result PostgresBackend::deleteBook(int actorId, int bookId) {
    if (!requireAdmin(actorId)) return Result::failure("Unauthorized. Only librarians can delete catalog items.");

    // EDGE CASE 3: Active Item Guard - Cannot delete a book someone has at home
    QSqlQuery checkLoans;
    checkLoans.prepare("SELECT COUNT(*) FROM Borrow_Records WHERE asset_id = :id AND return_date IS NULL");
    checkLoans.bindValue(":id", bookId);
    if (checkLoans.exec() && checkLoans.next() && checkLoans.value(0).toInt() > 0) {
        int activeCopies = checkLoans.value(0).toInt();
        return Result::failure(QString("Action Denied! Cannot delete book: %1 copy/copies currently checked out by members.")
                                   .arg(activeCopies));
    }

    QSqlQuery q;
    q.prepare("DELETE FROM Assets WHERE asset_id = :id");
    q.bindValue(":id", bookId);
    return q.exec() ? Result::success() : Result::failure("Failed to delete book from database.");
}

// =========================================================
// CIRCULATION LOGIC (CHECKOUT & RETURN)
// =========================================================

Result PostgresBackend::borrowBook(int userId, int bookId) {
    // EDGE CASE 5: Fine Limit Guard - Block checkout if user owes >= $10.00
    QSqlQuery checkFines;
    checkFines.prepare("SELECT unpaid_fines FROM Users WHERE user_id = :uid");
    checkFines.bindValue(":uid", userId);
    if (checkFines.exec() && checkFines.next()) {
        double fines = checkFines.value(0).toDouble();
        if (fines >= 10.00) {
            return Result::failure(QString("Checkout Blocked! You have $%1 in unpaid fines. Library policy blocks borrowing for debts of $10.00 or more.")
                                       .arg(fines, 0, 'f', 2));
        }
    }

    // EDGE CASE 6: Anti-Hoarding Guard - Cannot borrow two copies of the same book
    QSqlQuery checkDuplicate;
    checkDuplicate.prepare("SELECT 1 FROM Borrow_Records WHERE user_id = :u AND asset_id = :a AND return_date IS NULL");
    checkDuplicate.bindValue(":u", userId);
    checkDuplicate.bindValue(":a", bookId);
    if (checkDuplicate.exec() && checkDuplicate.next()) {
        return Result::failure("Action Denied! You already have an active borrowed copy of this book. Duplicate checkouts are not permitted.");
    }

    // EDGE CASE 7: Concurrency Race Condition Lock (FOR UPDATE)
    QSqlQuery q;
    q.prepare("SELECT available, title FROM Assets WHERE asset_id = :id FOR UPDATE");
    q.bindValue(":id", bookId);
    if (q.exec() && q.next()) {
        int available = q.value(0).toInt();
        QString title = q.value(1).toString();

        if (available <= 0) {
            return Result::failure(QString("Checkout Denied: All copies of \"%1\" are currently checked out. Please join the waitlist.").arg(title));
        }

        // Deduct inventory
        QSqlQuery updateQ;
        updateQ.prepare("UPDATE Assets SET available = available - 1 WHERE asset_id = :id");
        updateQ.bindValue(":id", bookId);
        updateQ.exec();

        // Create 14-day loan record
        QSqlQuery borrowQ;
        borrowQ.prepare("INSERT INTO Borrow_Records (user_id, asset_id, borrow_date, due_date) "
                        "VALUES (:u, :a, CURRENT_DATE, CURRENT_DATE + INTERVAL '14 days')");
        borrowQ.bindValue(":u", userId);
        borrowQ.bindValue(":a", bookId);
        borrowQ.exec();

        return Result::success();
    }

    return Result::failure("Book not found in database.");
}

Result PostgresBackend::listActiveLoans(int actorId, int userId, std::vector<LoanInfo>& out) {
    if(!requireAdmin(actorId) && actorId != userId) return Result::failure("Unauthorized.");
    out.clear();

    QSqlQuery q;
    q.prepare("SELECT br.loan_id, a.asset_id, a.title, br.borrow_date, br.due_date FROM Borrow_Records br JOIN Assets a ON br.asset_id = a.asset_id WHERE br.user_id = :uid AND br.return_date IS NULL");
    q.bindValue(":uid", userId);
    if(q.exec()) {
        while(q.next()) {
            LoanInfo l;
            l.loanId = q.value(0).toInt();
            l.assetId = q.value(1).toInt();
            l.assetTitle = q.value(2).toString();
            l.borrowDate = q.value(3).toDate();
            l.dueDate = q.value(4).toDate();
            out.push_back(l);
        }
    }
    return Result::success();
}

Result PostgresBackend::processReturn(int actorId, int loanId, double& fineCharged) {
    if(!requireAdmin(actorId)) return Result::failure("Unauthorized.");

    QSqlQuery q;
    q.prepare("SELECT due_date, asset_id, user_id FROM Borrow_Records WHERE loan_id = :lid AND return_date IS NULL");
    q.bindValue(":lid", loanId);

    if(q.exec() && q.next()) {
        QDate dueDate = q.value(0).toDate();
        int assetId = q.value(1).toInt();
        int userId = q.value(2).toInt();

        int lateDays = std::max(0LL, dueDate.daysTo(QDate::currentDate()));
        fineCharged = lateDays * 0.50; // Using base strategy rule

        // 1. Close record
        QSqlQuery updateRecord;
        updateRecord.prepare("UPDATE Borrow_Records SET return_date = CURRENT_DATE, fine_assessed = :f WHERE loan_id = :lid");
        updateRecord.bindValue(":f", fineCharged);
        updateRecord.bindValue(":lid", loanId);
        updateRecord.exec();

        // 2. Add fine to user if exists
        if(fineCharged > 0) {
            QSqlQuery addFine;
            addFine.prepare("UPDATE Users SET unpaid_fines = unpaid_fines + :f WHERE user_id = :uid");
            addFine.bindValue(":f", fineCharged);
            addFine.bindValue(":uid", userId);
            addFine.exec();
        }

        // 3. Put asset back in inventory
        QSqlQuery addInventory;
        addInventory.prepare("UPDATE Assets SET available = available + 1 WHERE asset_id = :id");
        addInventory.bindValue(":id", assetId);
        addInventory.exec();

        return Result::success();
    }
    return Result::failure("Valid open loan not found.");
}

Result PostgresBackend::getSystemAnalytics(LibraryStats& out) {
    // 1. Total Active Loans
    QSqlQuery q1("SELECT COUNT(*) FROM Borrow_Records WHERE return_date IS NULL");
    if (q1.next()) out.activeLoans = q1.value(0).toInt();

    // 2. Overdue Loans
    QSqlQuery q2("SELECT COUNT(*) FROM Borrow_Records WHERE return_date IS NULL AND due_date < CURRENT_DATE");
    if (q2.next()) out.overdueLoans = q2.value(0).toInt();

    // 3. Total Unpaid Fines
    QSqlQuery q3("SELECT COALESCE(SUM(unpaid_fines), 0) FROM Users");
    if (q3.next()) out.totalUnpaidFines = q3.value(0).toDouble();

    // 4. Most Borrowed Book
    QSqlQuery q4("SELECT a.title, COUNT(br.loan_id) AS cnt "
                 "FROM Assets a JOIN Borrow_Records br ON a.asset_id = br.asset_id "
                 "GROUP BY a.title ORDER BY cnt DESC LIMIT 1");
    if (q4.next()) {
        out.topBook = QString("%1 (%2 borrows)").arg(q4.value(0).toString()).arg(q4.value(1).toInt());
    } else {
        out.topBook = "No checkout history yet";
    }

    return Result::success();
}

// ============================================================
// KAN-26: MULTI-FILTER REPORT & CSV EXPORT
// ============================================================
Result PostgresBackend::exportHistoryToCSV(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return Result::failure("Could not create file at specified location.");
    }

    QTextStream out(&file);
    // CSV Header row
    out << "Loan ID,Borrower Name,Book Title,Borrow Date,Due Date,Return Date,Fine Incurred\n";

    QSqlQuery q("SELECT br.loan_id, u.name, a.title, br.borrow_date, br.due_date, "
                "COALESCE(TO_CHAR(br.return_date, 'YYYY-MM-DD'), 'ACTIVE'), br.fine_assessed "
                "FROM Borrow_Records br "
                "JOIN Users u ON br.user_id = u.user_id "
                "JOIN Assets a ON br.asset_id = a.asset_id "
                "ORDER BY br.loan_id ASC");

    while (q.next()) {
        out << q.value(0).toString() << ","
            << "\"" << q.value(1).toString() << "\","
            << "\"" << q.value(2).toString() << "\","
            << q.value(3).toString() << ","
            << q.value(4).toString() << ","
            << q.value(5).toString() << ","
            << QString("$%1").arg(q.value(6).toDouble(), 0, 'f', 2) << "\n";
    }

    file.close();
    return Result::success();
}