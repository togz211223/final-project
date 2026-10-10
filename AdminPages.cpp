#include "AdminPages.h"
#include "ILibraryBackend.h"
#include "Alert.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QFileDialog>

// ---------- helpers ----------
static QTableWidget* makeTable(const QStringList& headers) {
    auto* t = new QTableWidget(0, headers.size());
    t->setHorizontalHeaderLabels(headers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->verticalHeader()->setVisible(false);
    t->horizontalHeader()->setStretchLastSection(true);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    t->setAlternatingRowColors(false);
    return t;
}
static void setRow(QTableWidget* t, int row, const QStringList& cells) {
    for (int c = 0; c < cells.size(); ++c) t->setItem(row, c, new QTableWidgetItem(cells[c]));
}
static QLabel* title(const QString& text) { auto* l = new QLabel(text); l->setObjectName("pageTitle"); return l; }
static QPushButton* button(const QString& text, const char* role = nullptr) {
    auto* b = new QPushButton(text); if (role) b->setObjectName(role); return b;
}

// ---------- BookDialog ----------
BookDialog::BookDialog(QWidget* parent, const BookInfo* existing) : QDialog(parent), editing_(existing != nullptr) {
    setWindowTitle(editing_ ? "Edit Book" : "Add New Book");
    setModal(true);
    id_ = new QSpinBox; id_->setRange(1, 999999);
    title_ = new QLineEdit; title_->setMaxLength(200);
    author_ = new QLineEdit; author_->setMaxLength(120);
    category_ = new QComboBox; category_->setEditable(true);
    category_->addItems({"", "Computer Science", "Engineering", "Science", "Mathematics", "Literature", "History", "Reference", "Other"});
    isbn_ = new QLineEdit; isbn_->setPlaceholderText("Optional, e.g. 978-3-16-148410-0");
    quantity_ = new QSpinBox; quantity_->setRange(0, 9999); quantity_->setValue(1);
    available_ = new QSpinBox; available_->setRange(0, 9999); available_->setValue(1);

    auto* form = new QFormLayout;
    form->addRow("Book ID *", id_);
    form->addRow("Title *", title_);
    form->addRow("Author *", author_);
    form->addRow("Category", category_);
    form->addRow("ISBN", isbn_);
    form->addRow("Total copies *", quantity_);
    if (editing_) form->addRow("Available copies", available_);

    if (existing) {
        id_->setValue(existing->id); id_->setEnabled(false);
        title_->setText(existing->title); author_->setText(existing->author);
        category_->setCurrentText(existing->category); isbn_->setText(existing->isbn);
        quantity_->setValue(existing->quantity); available_->setValue(existing->available);
    }
    auto* ok = button(editing_ ? "Save Changes" : "Add Book", "primary");
    auto* cancel = button("Cancel");
    connect(ok, &QPushButton::clicked, this, &BookDialog::tryAccept);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    auto* row = new QHBoxLayout; row->addStretch(); row->addWidget(cancel); row->addWidget(ok);
    auto* v = new QVBoxLayout(this);
    v->addLayout(form);
    auto* note = new QLabel("* required"); note->setObjectName("hint"); v->addWidget(note);
    v->addLayout(row);
    setMinimumWidth(420);
}

void BookDialog::tryAccept() {
    if (title_->text().trimmed().isEmpty() || author_->text().trimmed().isEmpty()) {
        Alert::showWarning(this, "Please fill in all required fields (Title and Author)."); return;
    }
    const QString isbn = isbn_->text().trimmed();
    if (!isbn.isEmpty()) {
        QString digits = isbn; digits.remove('-').remove(' ');
        static const QRegularExpression re(R"(^(\d{9}[\dXx]|\d{13})$)");
        if (!re.match(digits).hasMatch()) { Alert::showWarning(this, "ISBN must contain 10 or 13 digits."); return; }
    }
    if (editing_ && available_->value() > quantity_->value()) {
        Alert::showWarning(this, "Available copies cannot exceed total copies."); return;
    }
    accept();
}

BookInfo BookDialog::book() const {
    BookInfo b;
    b.id = id_->value(); b.title = title_->text().trimmed(); b.author = author_->text().trimmed();
    b.category = category_->currentText().trimmed(); b.isbn = isbn_->text().trimmed();
    b.quantity = quantity_->value();
    b.available = editing_ ? available_->value() : quantity_->value();
    return b;
}

// ---------- BooksPage ----------
BooksPage::BooksPage(ILibraryBackend* backend, int actorId, QWidget* parent)
    : QWidget(parent), backend_(backend), actorId_(actorId) {
    search_ = new QLineEdit; search_->setPlaceholderText("Search by title, author, ISBN, category or ID"); search_->setClearButtonEnabled(true);
    table_ = makeTable({"ID", "Title", "Author", "Category", "ISBN", "Total", "Available"});
    auto* add = button("Add New Book", "primary"); auto* edit = button("Edit Selected"); auto* del = button("Delete Selected", "danger");

    auto* top = new QHBoxLayout; top->addWidget(search_, 1); top->addWidget(add); top->addWidget(edit); top->addWidget(del);
    auto* v = new QVBoxLayout(this); v->setContentsMargins(20, 16, 20, 16); v->setSpacing(10);
    v->addWidget(title("Books")); v->addLayout(top); v->addWidget(table_, 1);

    connect(search_, &QLineEdit::textChanged, this, [this] { refresh(); });
    connect(add, &QPushButton::clicked, this, &BooksPage::addBook);
    connect(edit, &QPushButton::clicked, this, &BooksPage::editBook);
    connect(del, &QPushButton::clicked, this, &BooksPage::deleteBook);
    connect(table_, &QTableWidget::cellDoubleClicked, this, [this] { editBook(); });
    refresh();
}

int BooksPage::selectedRow() const { return table_->currentRow(); }

void BooksPage::refresh() {
    try {
        Result r = backend_->listBooks(actorId_, search_->text().trimmed(), books_);
        if (!r.ok) { Alert::showError(this, r.message); return; }
        table_->setRowCount(0);
        for (const auto& b : books_) {
            int row = table_->rowCount(); table_->insertRow(row);
            setRow(table_, row, {QString::number(b.id), b.title, b.author, b.category, b.isbn,
                                 QString::number(b.quantity), QString::number(b.available)});
        }
        table_->resizeColumnsToContents();
        table_->horizontalHeader()->setStretchLastSection(true);
    } catch (const std::exception&) { Alert::showError(this, "An error occurred while loading books."); }
}

void BooksPage::addBook() {
    BookDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    try {
        Result r = backend_->addBook(actorId_, dlg.book());
        if (!r.ok) { Alert::showError(this, r.message.isEmpty() ? "Failed to add book. Please try again." : r.message); return; }
        Alert::showSuccess(this, "Book added successfully!");
        refresh();
    } catch (const std::exception&) { Alert::showError(this, "Failed to add book. Please try again."); }
}

void BooksPage::editBook() {
    int row = selectedRow();
    if (row < 0 || row >= (int)books_.size()) { Alert::showWarning(this, "Please select a book first."); return; }
    BookInfo current = books_[row];
    BookDialog dlg(this, &current);
    if (dlg.exec() != QDialog::Accepted) return;
    try {
        Result r = backend_->updateBook(actorId_, dlg.book());
        if (!r.ok) { Alert::showError(this, r.message.isEmpty() ? "Unable to update the book." : r.message); return; }
        Alert::showSuccess(this, "Book updated successfully!");
        refresh();
    } catch (const std::exception&) { Alert::showError(this, "An error occurred while saving the changes."); }
}

void BooksPage::deleteBook() {
    int row = selectedRow();
    if (row < 0 || row >= (int)books_.size()) { Alert::showWarning(this, "Please select a book first."); return; }
    const BookInfo b = books_[row];
    if (!Alert::showConfirmation(this, QString("Are you sure you want to delete \"%1\"?").arg(b.title), "Delete book", "Delete", "Cancel")) return;
    try {
        Result r = backend_->deleteBook(actorId_, b.id);
        if (!r.ok) { Alert::showError(this, r.message.isEmpty() ? "Unable to delete the book." : r.message); return; }
        Alert::showSuccess(this, "Book deleted successfully.");
        refresh();
    } catch (const std::exception&) { Alert::showError(this, "Unable to delete the book."); }
}

// ---------- UsersPage ----------
static QString roleText(UserRole r) { return r == UserRole::ADMIN ? "Librarian / Admin" : "Member"; }

UsersPage::UsersPage(ILibraryBackend* backend, int actorId, QWidget* parent)
    : QWidget(parent), backend_(backend), actorId_(actorId) {
    search_ = new QLineEdit; search_->setPlaceholderText("Search by name, email or ID"); search_->setClearButtonEnabled(true);
    table_ = makeTable({"ID", "Name", "Email", "Role", "Unpaid fines"});
    auto* del = button("Delete Selected User", "danger");
    auto* top = new QHBoxLayout; top->addWidget(search_, 1); top->addWidget(del);
    auto* v = new QVBoxLayout(this); v->setContentsMargins(20, 16, 20, 16); v->setSpacing(10);
    v->addWidget(title("Users")); v->addLayout(top); v->addWidget(table_, 1);
    connect(search_, &QLineEdit::textChanged, this, [this] { refresh(); });
    connect(del, &QPushButton::clicked, this, &UsersPage::deleteSelected);
    refresh();
}

void UsersPage::refresh() {
    try {
        Result r = backend_->listUsers(actorId_, search_->text().trimmed(), users_);
        if (!r.ok) { Alert::showError(this, r.message); return; }
        table_->setRowCount(0);
        for (const auto& u : users_) {
            int row = table_->rowCount(); table_->insertRow(row);
            setRow(table_, row, {QString::number(u.id), u.name, u.email, roleText(u.role), QString("$%1").arg(u.unpaidFines, 0, 'f', 2)});
        }
        table_->resizeColumnsToContents();
        table_->horizontalHeader()->setStretchLastSection(true);
    } catch (const std::exception&) { Alert::showError(this, "An error occurred while loading users."); }
}

void UsersPage::deleteSelected() {
    int row = table_->currentRow();
    if (row < 0 || row >= (int)users_.size()) { Alert::showWarning(this, "Please select a user first."); return; }
    const UserInfo u = users_[row];
    if (u.id == actorId_) { Alert::showWarning(this, "You cannot delete your own account."); return; }
    if (!Alert::showConfirmation(this, QString("Are you sure you want to delete this user?\n\n%1 (%2)").arg(u.name, u.email),
                                 "Delete user", "Confirm", "Cancel")) return;
    try {
        Result r = backend_->deleteUser(actorId_, u.id);
        if (!r.ok) { Alert::showError(this, r.message.isEmpty() ? "Unable to delete user." : r.message); return; }
        Alert::showSuccess(this, "User deleted successfully.");
        refresh();
    } catch (const std::exception&) { Alert::showError(this, "Unable to delete user."); }
}

// ---------- ReturnsPage ----------
ReturnsPage::ReturnsPage(ILibraryBackend* backend, int actorId, QWidget* parent)
    : QWidget(parent), backend_(backend), actorId_(actorId) {
    search_ = new QLineEdit; search_->setPlaceholderText("Step 1: search for the borrower by name, email or ID"); search_->setClearButtonEnabled(true);
    userTable_ = makeTable({"ID", "Name", "Email"});
    userTable_->setMaximumHeight(170);
    loanTable_ = makeTable({"Loan #", "Item", "Borrowed", "Due", "Status"});
    loanHint_ = new QLabel("Step 2: select the borrowed item to return."); loanHint_->setObjectName("hint");
    returnBtn_ = button("Process Return", "primary"); returnBtn_->setEnabled(false);

    auto* bottom = new QHBoxLayout; bottom->addStretch(); bottom->addWidget(returnBtn_);
    auto* v = new QVBoxLayout(this); v->setContentsMargins(20, 16, 20, 16); v->setSpacing(10);
    v->addWidget(title("Process Returns")); v->addWidget(search_); v->addWidget(userTable_);
    v->addWidget(loanHint_); v->addWidget(loanTable_, 1); v->addLayout(bottom);

    connect(search_, &QLineEdit::textChanged, this, [this] { refresh(); });
    connect(userTable_, &QTableWidget::itemSelectionChanged, this, &ReturnsPage::loadLoansForSelectedUser);
    connect(loanTable_, &QTableWidget::itemSelectionChanged, this, [this] { returnBtn_->setEnabled(loanTable_->currentRow() >= 0); });
    connect(returnBtn_, &QPushButton::clicked, this, &ReturnsPage::processSelectedReturn);
    refresh();
}

void ReturnsPage::refresh() {
    try {
        Result r = backend_->listUsers(actorId_, search_->text().trimmed(), users_);
        if (!r.ok) { Alert::showError(this, r.message); return; }
        userTable_->blockSignals(true);
        userTable_->setRowCount(0);
        for (const auto& u : users_) {
            int row = userTable_->rowCount(); userTable_->insertRow(row);
            setRow(userTable_, row, {QString::number(u.id), u.name, u.email});
        }
        userTable_->blockSignals(false);
        loans_.clear(); loanTable_->setRowCount(0); returnBtn_->setEnabled(false);
        userTable_->resizeColumnsToContents(); userTable_->horizontalHeader()->setStretchLastSection(true);
    } catch (const std::exception&) { Alert::showError(this, "An error occurred while loading users."); }
}

void ReturnsPage::loadLoansForSelectedUser() {
    loanTable_->setRowCount(0); loans_.clear(); returnBtn_->setEnabled(false);
    int row = userTable_->currentRow();
    if (row < 0 || row >= (int)users_.size()) return;
    try {
        Result r = backend_->listActiveLoans(actorId_, users_[row].id, loans_);
        if (!r.ok) { Alert::showError(this, r.message); return; }
        loanHint_->setText(loans_.empty() ? "This user has no borrowed items." : "Step 2: select the borrowed item to return.");
        for (const auto& l : loans_) {
            int rr = loanTable_->rowCount(); loanTable_->insertRow(rr);
            const qint64 late = l.dueDate.daysTo(QDate::currentDate());
            setRow(loanTable_, rr, {QString::number(l.loanId), l.assetTitle, l.borrowDate.toString("yyyy-MM-dd"),
                                    l.dueDate.toString("yyyy-MM-dd"), late > 0 ? QString("Overdue (%1 days)").arg(late) : "On time"});
        }
        loanTable_->resizeColumnsToContents(); loanTable_->horizontalHeader()->setStretchLastSection(true);
    } catch (const std::exception&) { Alert::showError(this, "An error occurred while loading borrowed items."); }
}

void ReturnsPage::processSelectedReturn() {
    int row = loanTable_->currentRow();
    if (row < 0 || row >= (int)loans_.size()) { Alert::showWarning(this, "Please select an item to return."); return; }
    const LoanInfo l = loans_[row];
    if (!Alert::showConfirmation(this, QString("Mark \"%1\" as returned?").arg(l.assetTitle), "Process return", "Process Return", "Cancel")) return;
    try {
        double fine = 0.0;
        Result r = backend_->processReturn(actorId_, l.loanId, fine);
        if (!r.ok) { Alert::showError(this, r.message.isEmpty() ? "Unable to process the return." : r.message); return; }
        Alert::showSuccess(this, fine > 0.0 ? QString("Return processed successfully! A late fee of $%1 was assessed.").arg(fine, 0, 'f', 2)
                                            : "Return processed successfully!");
        loadLoansForSelectedUser();
    } catch (const std::exception&) { Alert::showError(this, "Unable to process the return."); }
}

AnalyticsPage::AnalyticsPage(ILibraryBackend* backend, int actorId, QWidget* parent)
    : QWidget(parent), backend_(backend), actorId_(actorId) {
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(20, 20, 20, 20);
    v->setSpacing(16);

    v->addWidget(title("Library Analytics & Operational Reports"));

    // Metric Cards Frame
    auto* card = new QFrame;
    card->setObjectName("card");
    auto* form = new QFormLayout(card);
    form->setContentsMargins(20, 20, 20, 20);
    form->setSpacing(12);

    activeLoansLbl_ = new QLabel("0");
    overdueLbl_ = new QLabel("0");
    finesLbl_ = new QLabel("$0.00");
    topBookLbl_ = new QLabel("None");

    activeLoansLbl_->setStyleSheet("font-size: 13pt; font-weight: bold; color: #2563eb;");
    overdueLbl_->setStyleSheet("font-size: 13pt; font-weight: bold; color: #dc2626;");
    finesLbl_->setStyleSheet("font-size: 13pt; font-weight: bold; color: #b45309;");
    topBookLbl_->setStyleSheet("font-size: 12pt; font-weight: bold; color: #0f172a;");

    form->addRow("📖 Total Active Loans:", activeLoansLbl_);
    form->addRow("⚠️ Current Overdue Items:", overdueLbl_);
    form->addRow("💰 Total Outstanding Unpaid Fines:", finesLbl_);
    form->addRow("🏆 #1 Most Popular Book:", topBookLbl_);

    v->addWidget(card);

    // Export Action Row
    auto* exportBtn = button("Export Borrow History (.csv)", "primary");
    auto* refreshBtn = button("Refresh Metrics");
    connect(refreshBtn, &QPushButton::clicked, this, &AnalyticsPage::refresh);
    connect(exportBtn, &QPushButton::clicked, this, &AnalyticsPage::exportCSV);

    auto* btnRow = new QHBoxLayout;
    btnRow->addWidget(refreshBtn);
    btnRow->addWidget(exportBtn);
    btnRow->addStretch();

    v->addLayout(btnRow);
    v->addStretch();

    refresh();
}

void AnalyticsPage::refresh() {
    LibraryStats s;
    if (backend_->getSystemAnalytics(s).ok) {
        activeLoansLbl_->setText(QString::number(s.activeLoans));
        overdueLbl_->setText(QString::number(s.overdueLoans));
        finesLbl_->setText(QString("$%1").arg(s.totalUnpaidFines, 0, 'f', 2));
        topBookLbl_->setText(s.topBook);
    }
}

void AnalyticsPage::exportCSV() {
    QString path = QFileDialog::getSaveFileName(this, "Save Report", "Library_Report.csv", "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    Result r = backend_->exportHistoryToCSV(path);
    if (r.ok) {
        Alert::showSuccess(this, "Report exported successfully! You can open it in Excel.");
    } else {
        Alert::showError(this, r.message);
    }
}