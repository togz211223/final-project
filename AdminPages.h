#pragma once
#include <QWidget>
#include <QDialog>
#include <vector>
#include "Types.h"
class ILibraryBackend;
class QTableWidget; class QLineEdit; class QSpinBox; class QComboBox; class QPushButton; class QLabel;

// Add / Edit book form with input validation.
class BookDialog : public QDialog {
    Q_OBJECT
public:
    explicit BookDialog(QWidget* parent, const BookInfo* existing = nullptr);
    BookInfo book() const;
private:
    void tryAccept();
    bool editing_;
    QSpinBox *id_, *quantity_, *available_;
    QLineEdit *title_, *author_, *isbn_;
    QComboBox* category_;
};

class BooksPage : public QWidget {
    Q_OBJECT
public:
    BooksPage(ILibraryBackend* backend, int actorId, QWidget* parent = nullptr);
    void refresh();
private:
    void addBook(); void editBook(); void deleteBook();
    int selectedRow() const;
    ILibraryBackend* backend_; int actorId_;
    QLineEdit* search_; QTableWidget* table_;
    std::vector<BookInfo> books_;
};

class UsersPage : public QWidget {
    Q_OBJECT
public:
    UsersPage(ILibraryBackend* backend, int actorId, QWidget* parent = nullptr);
    void refresh();
private:
    void deleteSelected();
    ILibraryBackend* backend_; int actorId_;
    QLineEdit* search_; QTableWidget* table_;
    std::vector<UserInfo> users_;
};

class ReturnsPage : public QWidget {
    Q_OBJECT
public:
    ReturnsPage(ILibraryBackend* backend, int actorId, QWidget* parent = nullptr);
    void refresh();
private:
    void loadLoansForSelectedUser();
    void processSelectedReturn();
    ILibraryBackend* backend_; int actorId_;
    QLineEdit* search_; QTableWidget *userTable_, *loanTable_;
    QPushButton* returnBtn_; QLabel* loanHint_;
    std::vector<UserInfo> users_; std::vector<LoanInfo> loans_;
};
