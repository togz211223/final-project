#pragma once
#include <QWidget>
#include <vector>
#include "Types.h"
class ILibraryBackend; class QStackedWidget; class QPushButton;

// Throws std::invalid_argument if the user is not an ADMIN. Every backend call
// additionally re-checks the role on the backend side.
class AdminDashboard : public QWidget {
    Q_OBJECT
public:
    AdminDashboard(ILibraryBackend* backend, const UserInfo& admin, QWidget* parent = nullptr);
signals:
    void logoutRequested();
private:
    void showSection(int index);
    QStackedWidget* pages_;
    std::vector<QPushButton*> navButtons_;
};
