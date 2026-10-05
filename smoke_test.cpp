#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <iostream>
#include <QLabel>
#include <stdexcept>
#include "AuthWindow.h"
#include "AdminDashboard.h"
#include "AdminPages.h"
#include "InMemoryBackend.h"
#include "Alert.h"

static int failures = 0;
#define CHECK(c) do { std::cout << std::flush; if (!(c)) { std::cout << "FAIL: " #c " (line " << __LINE__ << ")\n"; ++failures; } else std::cout << "ok:   " #c "\n"; } while (0)

static QPushButton* btn(QWidget* w, const QString& text) {
    for (auto* b : w->findChildren<QPushButton*>()) if (b->text() == text) return b;
    return nullptr;
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    InMemoryBackend be;

    std::vector<std::pair<Alert::Kind, QString>> popups; bool confirmAnswer = true;
    Alert::setTestHook([&](Alert::Kind k, const QString& m) { popups.push_back({k, m}); return confirmAnswer; });
    auto last = [&] { return popups.empty() ? QString() : popups.back().second; };

    // ---- Registration validation + login ----
    AuthWindow auth(&be);
    auto E = [&](const char* n) { return auth.findChild<QLineEdit*>(n); };
    QLineEdit* edits[6] = {E("loginEmail"), E("loginPassword"), E("regName"), E("regEmail"), E("regPassword"), E("regConfirm")};
    for (auto* e : edits) if (!e) { std::cout << "missing field\n"; return 1; }
    CHECK(edits[1]->echoMode() == QLineEdit::Password && edits[4]->echoMode() == QLineEdit::Password);
    std::vector<UserInfo> got;
    QObject::connect(&auth, &AuthWindow::loggedIn, [&](const UserInfo& u) { got.push_back(u); });

    btn(&auth, "Register")->click();
    CHECK(auth.findChild<QLabel*>("regError")->text().contains("fill in all"));
    edits[2]->setText("Alice Admin"); edits[3]->setText("not-an-email"); edits[4]->setText("password123"); edits[5]->setText("password123");
    btn(&auth, "Register")->click();
    CHECK(auth.findChild<QLabel*>("regError")->text().contains("valid email"));
    edits[3]->setText("alice@lib.org"); edits[5]->setText("different");
    btn(&auth, "Register")->click();
    CHECK(auth.findChild<QLabel*>("regError")->text().contains("do not match"));
    edits[5]->setText("password123");
    btn(&auth, "Register")->click();
    CHECK(last().contains("Account created") && popups.back().first == Alert::Kind::Success);

    // duplicate email
    edits[2]->setText("Alice 2"); edits[3]->setText("ALICE@lib.org"); edits[4]->setText("password123"); edits[5]->setText("password123");
    btn(&auth, "Register")->click();
    CHECK(auth.findChild<QLabel*>("regError")->text().contains("already exists"));
    // second user (member)
    edits[2]->setText("Bob Member"); edits[3]->setText("bob@lib.org"); edits[5]->setText("password123");
    btn(&auth, "Register")->click();

    // bad login, empty login, good admin login, good member login
    edits[0]->setText("alice@lib.org"); edits[1]->setText("wrongpass"); btn(&auth, "Login")->click();
    CHECK(auth.findChild<QLabel*>("loginError")->text() == "Invalid username or password.");
    edits[1]->setText(""); btn(&auth, "Login")->click();
    CHECK(auth.findChild<QLabel*>("loginError")->text().contains("Please enter"));
    edits[0]->setText("alice@lib.org"); edits[1]->setText("password123"); btn(&auth, "Login")->click();
    CHECK(got.size() == 1);
    UserInfo admin = got.back(); got.clear();
    CHECK(admin.role == UserRole::ADMIN);
    edits[0]->setText("bob@lib.org"); edits[1]->setText("password123"); btn(&auth, "Login")->click();
    UserInfo bob = got.back(); got.clear();
    CHECK(bob.role == UserRole::MEMBER);

    // ---- Role enforcement ----
    bool threw = false; try { AdminDashboard d(&be, bob); } catch (const std::invalid_argument&) { threw = true; }
    CHECK(threw);
    std::vector<BookInfo> bl; CHECK(!be.listBooks(bob.id, "", bl).ok);
    CHECK(!be.addBook(bob.id, BookInfo{1, "T", "A", "", "", 1, 1}).ok);
    CHECK(!be.deleteUser(bob.id, admin.id).ok);

    // ---- Dashboard: add book ----
    AdminDashboard dash(&be, admin);
    auto* books = dash.findChild<BooksPage*>();
    popups.clear();
    BookInfo b{101, "C++ Primer", "Lippman", "Computer Science", "9780321714114", 2, 2};
    CHECK(be.addBook(admin.id, b).ok);
    CHECK(!be.addBook(admin.id, b).ok);   // duplicate id
    books->refresh();
    CHECK(books->findChild<QTableWidget*>()->rowCount() == 1);

    // ---- Delete user: cancel then confirm ----
    popups.clear(); confirmAnswer = false;
    auto* users = dash.findChild<UsersPage*>();
    auto* utable = users->findChild<QTableWidget*>();
    CHECK(utable->rowCount() == 2);
    utable->selectRow(1);                       // Bob
    btn(users, "Delete Selected User")->click();
    CHECK(popups.back().first == Alert::Kind::Confirm && last().contains("Are you sure you want to delete this user?"));
    CHECK(utable->rowCount() == 2);             // cancelled => nothing deleted
    // user with active loan cannot be deleted
    be.borrowBook(bob.id, 101);
    confirmAnswer = true; utable->selectRow(1); btn(users, "Delete Selected User")->click();
    CHECK(popups.back().first == Alert::Kind::Error && utable->rowCount() == 2);
    // self-delete blocked
    utable->selectRow(0); btn(users, "Delete Selected User")->click();
    CHECK(popups.back().first == Alert::Kind::Warning);

    // ---- Returns ----
    popups.clear();
    auto* returns = dash.findChild<ReturnsPage*>();
    returns->refresh();
    auto tables = returns->findChildren<QTableWidget*>();   // [0]=users [1]=loans
    tables[0]->selectRow(1);
    CHECK(tables[1]->rowCount() == 1);
    tables[1]->selectRow(0);
    btn(returns, "Process Return")->click();
    CHECK(popups.back().first == Alert::Kind::Success && last().contains("Return processed successfully"));
    CHECK(tables[1]->rowCount() == 0);
    be.listBooks(admin.id, "", bl); CHECK(bl[0].available == 2);

    // now deleting Bob works
    popups.clear(); users->refresh(); utable->selectRow(1);
    btn(users, "Delete Selected User")->click();
    CHECK(popups.back().second.contains("deleted successfully") && utable->rowCount() == 1);

    std::cout << (failures ? "\nSOME TESTS FAILED\n" : "\nALL TESTS PASSED\n");
    return failures;
}
