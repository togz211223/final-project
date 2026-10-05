#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>
#include "AuthWindow.h"
#include "AdminDashboard.h"
#include "InMemoryBackend.h"
#include "Alert.h"
#include "Style.h"

// Stand-in for Frontend Developer A's member screens. Replace with the real window.
static QWidget* makeMemberPlaceholder(const UserInfo& u, QWidget* logoutTarget) {
    auto* w = new QWidget; w->setObjectName("root"); w->setWindowTitle("Smart Library"); w->resize(520, 300);
    auto* v = new QVBoxLayout(w);
    auto* t = new QLabel(QString("Welcome, %1.\nThe member interface is provided by Frontend Developer A.").arg(u.name));
    t->setAlignment(Qt::AlignCenter);
    auto* b = new QPushButton("Log out");
    v->addStretch(); v->addWidget(t); v->addWidget(b, 0, Qt::AlignHCenter); v->addStretch();
    QObject::connect(b, &QPushButton::clicked, logoutTarget, [w, logoutTarget] { w->close(); w->deleteLater(); logoutTarget->show(); });
    return w;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyleSheet(appStyleSheet());

    InMemoryBackend backend;           // INTEGRATION: swap for the PostgreSQL-backed ILibraryBackend
    AuthWindow auth(&backend);
    QWidget* current = nullptr;

    QObject::connect(&auth, &AuthWindow::loggedIn, &auth, [&](const UserInfo& user) {
        try {
            if (user.role == UserRole::ADMIN) {
                auto* dash = new AdminDashboard(&backend, user);
                dash->setAttribute(Qt::WA_DeleteOnClose);
                QObject::connect(dash, &AdminDashboard::logoutRequested, &auth, [&, dash] { dash->close(); auth.show(); });
                current = dash;
            } else {
                current = makeMemberPlaceholder(user, &auth);
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
