#pragma once
#include <QWidget>
#include <vector>
#include "Types.h"

class ILibraryBackend;
class QTableWidget;
class QLineEdit;
class QPushButton;
class QLabel;
class QTabWidget;

class MemberDashboard : public QWidget {
    Q_OBJECT
public:
    MemberDashboard(ILibraryBackend* backend, const UserInfo& user, QWidget* parent = nullptr);

signals:
    void logoutRequested();

private slots:
    void refreshCatalog();
    void refreshLoans();
    void borrowSelectedBook();

private:
    ILibraryBackend* backend_;
    UserInfo user_;

    // UI Elements
    QLineEdit* searchEdit_;
    QTableWidget* catalogTable_;
    QTableWidget* loansTable_;
    QPushButton* borrowBtn_;
    QLabel* finesLabel_;

    std::vector<BookInfo> books_;
    std::vector<LoanInfo> loans_;

    QWidget* createCatalogTab();
    QWidget* createLoansTab();
};