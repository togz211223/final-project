#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>
#include "AuthWindow.h"
#include "AdminDashboard.h"
#include "PostgresBackend.h"
#include "MemberDashboard.h"
#include "Alert.h"
#include "Style.h"

// Stand-in for Frontend Developer A's member screens. Replace with the real window later.
static QWidget* makeMemberPlaceholder(const UserInfo& u, QWidget* logoutTarget) {
    auto* w = new QWidget;
    w->setObjectName("root");
    w->setWindowTitle("Smart Library - Member Portal");
    w->resize(520, 300);

    auto* v = new QVBoxLayout(w);
    auto* t = new QLabel(QString("Welcome, %1.\nThe member interface is being built by Frontend Developer A.").arg(u.name));
    t->setAlignment(Qt::AlignCenter);

    auto* b = new QPushButton("Log out");
    b->setObjectName("primary"); // Gives it a nice blue styling

    v->addStretch();
    v->addWidget(t);
    v->addWidget(b, 0, Qt::AlignHCenter);
    v->addStretch();

    QObject::connect(b, &QPushButton::clicked, logoutTarget, [w, logoutTarget] {
        w->close(); w->deleteLater(); logoutTarget->show();
    });
    return w;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(appStyleSheet());

    // ============================================================
    // THE MASTER SWAP: Using the Real Postgres Backend
    // ============================================================
    PostgresBackend backend;

    // The UI only cares that it gets *some* pointer to ILibraryBackend.
    // It doesn't know or care that we swapped Memory for Postgres!
    AuthWindow auth(&backend);
    QWidget* current = nullptr;

    // Handle Authentication & Routing
    QObject::connect(&auth, &AuthWindow::loggedIn, &auth, [&](const UserInfo& user) {
        try {
            if (user.role == UserRole::ADMIN) {
                // Launch Admin GUI
                auto* dash = new AdminDashboard(&backend, user);
                dash->setAttribute(Qt::WA_DeleteOnClose);

                QObject::connect(dash, &AdminDashboard::logoutRequested, &auth, [&, dash] {
                    dash->close(); auth.show();
                });
                current = dash;
            } else {
                // Launch Member GUI
                current = makeMemberPlaceholder(user, &auth);auto* memberDash = new MemberDashboard(&backend, user);
                memberDash->setAttribute(Qt::WA_DeleteOnClose);
                QObject::connect(memberDash, &MemberDashboard::logoutRequested, &auth, [&, memberDash] {
                    memberDash->close();
                    auth.show();
                });
                current = memberDash;
            }
            auth.hide();
            current->show();
        } catch (const std::exception&) {
            Alert::showError(&auth, "You do not have permission to open this screen.");
        }
    });

    auth.show();
    return app.exec();
}