#include "AdminDashboard.h"
#include "AdminPages.h"
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <stdexcept>

AdminDashboard::AdminDashboard(ILibraryBackend* backend, const UserInfo& admin, QWidget* parent) : QWidget(parent) {
    if (admin.role != UserRole::ADMIN) throw std::invalid_argument("Admin dashboard requires an ADMIN user");
    setObjectName("root");
    setWindowTitle("Smart Library - Librarian Dashboard");
    resize(1100, 680);

    pages_ = new QStackedWidget;
    pages_->addWidget(new BooksPage(backend, admin.id));
    pages_->addWidget(new UsersPage(backend, admin.id));
    pages_->addWidget(new ReturnsPage(backend, admin.id));

    auto* sidebar = new QFrame; sidebar->setObjectName("sidebar"); sidebar->setFixedWidth(200);
    auto* sv = new QVBoxLayout(sidebar); sv->setContentsMargins(0, 12, 0, 12); sv->setSpacing(0);
    const QStringList names = {"Books", "Users", "Returns"};
    for (int i = 0; i < names.size(); ++i) {
        auto* b = new QPushButton(names[i]); b->setObjectName("navButton"); b->setCheckable(true);
        connect(b, &QPushButton::clicked, this, [this, i] { showSection(i); });
        navButtons_.push_back(b); sv->addWidget(b);
    }
    sv->addStretch();

    auto* top = new QFrame; top->setObjectName("topbar"); top->setFixedHeight(48);
    auto* th = new QHBoxLayout(top); th->setContentsMargins(20, 0, 16, 0);
    auto* who = new QLabel(QString("%1  (Librarian / Admin)").arg(admin.name));
    auto* logout = new QPushButton("Log out");
    th->addWidget(new QLabel("<b>Smart Library</b>")); th->addStretch(); th->addWidget(who); th->addWidget(logout);
    connect(logout, &QPushButton::clicked, this, &AdminDashboard::logoutRequested);

    auto* right = new QVBoxLayout; right->setSpacing(0); right->setContentsMargins(0, 0, 0, 0);
    right->addWidget(top); right->addWidget(pages_, 1);
    auto* root = new QHBoxLayout(this); root->setSpacing(0); root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(sidebar); root->addLayout(right, 1);
    showSection(0);
}

void AdminDashboard::showSection(int index) {
    pages_->setCurrentIndex(index);
    for (int i = 0; i < (int)navButtons_.size(); ++i) navButtons_[i]->setChecked(i == index);
    if (auto* p = pages_->currentWidget()) {
        if (auto* b = qobject_cast<BooksPage*>(p)) b->refresh();
        else if (auto* u = qobject_cast<UsersPage*>(p)) u->refresh();
        else if (auto* r = qobject_cast<ReturnsPage*>(p)) r->refresh();
    }
}
