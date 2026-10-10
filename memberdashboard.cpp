#include "MemberDashboard.h"
#include "ILibraryBackend.h"
#include "Alert.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

MemberDashboard::MemberDashboard(ILibraryBackend* backend, const UserInfo& user, QWidget* parent)
    : QWidget(parent), backend_(backend), user_(user) {
    setObjectName("root");
    setWindowTitle("Smart Library - Member Portal");
    resize(1000, 650);

    // --- Top Bar ---
    auto* top = new QFrame;
    top->setObjectName("topbar");
    top->setFixedHeight(54);
    auto* topLayout = new QHBoxLayout(top);
    topLayout->setContentsMargins(20, 0, 20, 0);

    auto* brand = new QLabel("<b>Smart Library</b>  |  Member Portal");
    finesLabel_ = new QLabel(QString("Unpaid Fines: $%1").arg(user_.unpaidFines, 0, 'f', 2));
    if (user_.unpaidFines >= 10.0) {
        finesLabel_->setStyleSheet("color: #dc2626; font-weight: bold;"); // Red warning if fines block borrowing
    }

    auto* who = new QLabel(QString("Welcome, <b>%1</b>").arg(user_.name));
    auto* logout = new QPushButton("Log out");
    connect(logout, &QPushButton::clicked, this, &MemberDashboard::logoutRequested);

    topLayout->addWidget(brand);
    topLayout->addStretch();
    topLayout->addWidget(finesLabel_);
    topLayout->addSpacing(20);
    topLayout->addWidget(who);
    topLayout->addSpacing(10);
    topLayout->addWidget(logout);

    // --- Tab Widget (Catalog vs My Loans) ---
    auto* tabs = new QTabWidget;
    tabs->addTab(createCatalogTab(), "📚 Browse Catalog");
    tabs->addTab(createLoansTab(), "📖 My Borrowed Books");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(top);
    mainLayout->addWidget(tabs, 1);

    // Load initial data
    refreshCatalog();
    refreshLoans();
}

QWidget* MemberDashboard::createCatalogTab() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    searchEdit_ = new QLineEdit;
    searchEdit_->setPlaceholderText("Search catalog by title, author, category, or ISBN...");
    searchEdit_->setClearButtonEnabled(true);
    connect(searchEdit_, &QLineEdit::textChanged, this, &MemberDashboard::refreshCatalog);

    catalogTable_ = new QTableWidget(0, 6);
    catalogTable_->setHorizontalHeaderLabels({"ID", "Title", "Author", "Category", "ISBN", "Available"});
    catalogTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    catalogTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    catalogTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    catalogTable_->verticalHeader()->setVisible(false);
    catalogTable_->horizontalHeader()->setStretchLastSection(true);

    borrowBtn_ = new QPushButton("Borrow Selected Book");
    borrowBtn_->setObjectName("primary");
    connect(borrowBtn_, &QPushButton::clicked, this, &MemberDashboard::borrowSelectedBook);

    auto* searchRow = new QHBoxLayout;
    searchRow->addWidget(searchEdit_, 1);
    searchRow->addWidget(borrowBtn_);

    layout->addLayout(searchRow);
    layout->addWidget(catalogTable_, 1);
    return page;
}

QWidget* MemberDashboard::createLoansTab() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    loansTable_ = new QTableWidget(0, 5);
    loansTable_->setHorizontalHeaderLabels({"Loan #", "Title", "Borrowed Date", "Due Date", "Status"});
    loansTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    loansTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    loansTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    loansTable_->verticalHeader()->setVisible(false);
    loansTable_->horizontalHeader()->setStretchLastSection(true);

    layout->addWidget(new QLabel("Here are the items you currently have checked out:"));
    layout->addWidget(loansTable_, 1);
    return page;
}

void MemberDashboard::refreshCatalog() {
    books_.clear();
    catalogTable_->setRowCount(0);

    Result r = backend_->listBooks(user_.id, searchEdit_->text().trimmed(), books_);
    if (!r.ok) return;

    for (const auto& b : books_) {
        int row = catalogTable_->rowCount();
        catalogTable_->insertRow(row);
        catalogTable_->setItem(row, 0, new QTableWidgetItem(QString::number(b.id)));
        catalogTable_->setItem(row, 1, new QTableWidgetItem(b.title));
        catalogTable_->setItem(row, 2, new QTableWidgetItem(b.author));
        catalogTable_->setItem(row, 3, new QTableWidgetItem(b.category));
        catalogTable_->setItem(row, 4, new QTableWidgetItem(b.isbn));
        catalogTable_->setItem(row, 5, new QTableWidgetItem(QString("%1 / %2").arg(b.available).arg(b.quantity)));
    }
}

void MemberDashboard::refreshLoans() {
    loans_.clear();
    loansTable_->setRowCount(0);

    Result r = backend_->listActiveLoans(user_.id, user_.id, loans_);
    if (!r.ok) return;

    for (const auto& l : loans_) {
        int row = loansTable_->rowCount();
        loansTable_->insertRow(row);
        loansTable_->setItem(row, 0, new QTableWidgetItem(QString::number(l.loanId)));
        loansTable_->setItem(row, 1, new QTableWidgetItem(l.assetTitle));
        loansTable_->setItem(row, 2, new QTableWidgetItem(l.borrowDate.toString("yyyy-MM-dd")));
        loansTable_->setItem(row, 3, new QTableWidgetItem(l.dueDate.toString("yyyy-MM-dd")));

        qint64 late = l.dueDate.daysTo(QDate::currentDate());
        auto* statusItem = new QTableWidgetItem(late > 0 ? QString("OVERDUE (%1 days)").arg(late) : "Active (On Time)");
        if (late > 0) statusItem->setForeground(QBrush(QColor("#dc2626")));
        loansTable_->setItem(row, 4, statusItem);
    }
}

void MemberDashboard::borrowSelectedBook() {
    int row = catalogTable_->currentRow();
    if (row < 0 || row >= (int)books_.size()) {
        Alert::showWarning(this, "Please select a book from the catalog first.");
        return;
    }

    if (user_.unpaidFines >= 10.0) {
        Alert::showError(this, "Borrowing blocked! You have $10.00 or more in unpaid fines.");
        return;
    }

    const BookInfo b = books_[row];
    if (b.available <= 0) {
        Alert::showWarning(this, "No copies available! This book has been requested on the Waitlist.");
        return;
    }

    if (!Alert::showConfirmation(this, QString("Borrow \"%1\" for 14 days?").arg(b.title), "Confirm Checkout")) {
        return;
    }

    Result r = backend_->borrowBook(user_.id, b.id);
    if (!r.ok) {
        Alert::showError(this, r.message);
        return;
    }

    Alert::showSuccess(this, QString("Successfully checked out \"%1\"! It is due in 14 days.").arg(b.title));
    refreshCatalog();
    refreshLoans();
}