#pragma once
#include <QWidget>
#include "Types.h"
class ILibraryBackend;
class QLineEdit; class QLabel; class QStackedWidget;

class AuthWindow : public QWidget {
    Q_OBJECT
public:
    explicit AuthWindow(ILibraryBackend* backend, QWidget* parent = nullptr);
    void reset();   // clears fields (used after logout)
signals:
    void loggedIn(const UserInfo& user);   // controller decides which UI to open based on user.role
private:
    QWidget* buildLoginPage();
    QWidget* buildRegisterPage();
    void attemptLogin();
    void attemptRegister();
    static bool isValidEmail(const QString& s);

    ILibraryBackend* backend_;
    QStackedWidget* stack_;
    QLineEdit *loginEmail_, *loginPassword_;
    QLabel* loginError_;
    QLineEdit *regName_, *regEmail_, *regPassword_, *regConfirm_;
    QLabel* regError_;
};
