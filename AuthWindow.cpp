#include "AuthWindow.h"
#include "ILibraryBackend.h"
#include "Alert.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QRegularExpression>

AuthWindow::AuthWindow(ILibraryBackend* backend, QWidget* parent) : QWidget(parent), backend_(backend) {
    setObjectName("root");
    setWindowTitle("Smart Library - Sign In");
    resize(520, 560);

    stack_ = new QStackedWidget;
    stack_->addWidget(buildLoginPage());
    stack_->addWidget(buildRegisterPage());

    auto* card = new QFrame; card->setObjectName("card"); card->setFixedWidth(400);
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 24, 28, 24);
    cardLayout->addWidget(stack_);

    auto* title = new QLabel("Smart Library & Digital Asset Management");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);
    title->setWordWrap(true); title->setFixedWidth(400); title->setMinimumHeight(54);

    auto* outer = new QVBoxLayout(this);
    outer->addStretch();
    outer->addWidget(title, 0, Qt::AlignHCenter);
    outer->addSpacing(14);
    outer->addWidget(card, 0, Qt::AlignHCenter);
    outer->addStretch();
}

static QLabel* makeErrorLabel() {
    auto* l = new QLabel; l->setObjectName("formError"); l->setWordWrap(true); l->setMinimumHeight(34);
    return l;
}
static QLineEdit* makePasswordEdit() {
    auto* e = new QLineEdit; e->setEchoMode(QLineEdit::Password); e->setMaxLength(128); return e;
}

QWidget* AuthWindow::buildLoginPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    auto* h = new QLabel("Sign in"); h->setObjectName("pageTitle");
    v->addWidget(h);

    loginEmail_ = new QLineEdit; loginEmail_->setPlaceholderText("name@example.com");
    loginPassword_ = makePasswordEdit();
    loginEmail_->setObjectName("loginEmail"); loginPassword_->setObjectName("loginPassword");
    auto* form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft);
    form->addRow("Username / Email", loginEmail_);
    form->addRow("Password", loginPassword_);
    v->addLayout(form);

    loginError_ = makeErrorLabel(); loginError_->setObjectName("loginError");
    v->addWidget(loginError_);

    auto* btn = new QPushButton("Login"); btn->setObjectName("primary"); btn->setDefault(true);
    v->addWidget(btn);

    auto* sw = new QPushButton("Need an account? Register"); sw->setObjectName("linkButton");
    v->addWidget(sw, 0, Qt::AlignHCenter);

    connect(btn, &QPushButton::clicked, this, &AuthWindow::attemptLogin);
    connect(loginEmail_, &QLineEdit::returnPressed, this, &AuthWindow::attemptLogin);
    connect(loginPassword_, &QLineEdit::returnPressed, this, &AuthWindow::attemptLogin);
    connect(sw, &QPushButton::clicked, this, [this] { loginError_->clear(); stack_->setCurrentIndex(1); regName_->setFocus(); });
    return page;
}

QWidget* AuthWindow::buildRegisterPage() {
    auto* page = new QWidget;
    auto* v = new QVBoxLayout(page);
    auto* h = new QLabel("Create account"); h->setObjectName("pageTitle");
    v->addWidget(h);

    regName_ = new QLineEdit; regName_->setMaxLength(80);
    regEmail_ = new QLineEdit; regEmail_->setPlaceholderText("name@example.com");
    regPassword_ = makePasswordEdit();
    regConfirm_ = makePasswordEdit();
    regName_->setObjectName("regName"); regEmail_->setObjectName("regEmail");
    regPassword_->setObjectName("regPassword"); regConfirm_->setObjectName("regConfirm");
    auto* form = new QFormLayout;
    form->addRow("Full name", regName_);
    form->addRow("Username / Email", regEmail_);
    form->addRow("Password", regPassword_);
    form->addRow("Confirm password", regConfirm_);
    v->addLayout(form);

    auto* hint = new QLabel("Password must be at least 8 characters."); hint->setObjectName("hint");
    v->addWidget(hint);
    regError_ = makeErrorLabel(); regError_->setObjectName("regError");
    v->addWidget(regError_);

    auto* btn = new QPushButton("Register"); btn->setObjectName("primary");
    v->addWidget(btn);
    auto* sw = new QPushButton("Already have an account? Sign in"); sw->setObjectName("linkButton");
    v->addWidget(sw, 0, Qt::AlignHCenter);

    connect(btn, &QPushButton::clicked, this, &AuthWindow::attemptRegister);
    connect(regConfirm_, &QLineEdit::returnPressed, this, &AuthWindow::attemptRegister);
    connect(sw, &QPushButton::clicked, this, [this] { regError_->clear(); stack_->setCurrentIndex(0); loginEmail_->setFocus(); });
    return page;
}

bool AuthWindow::isValidEmail(const QString& s) {
    static const QRegularExpression re(R"(^[A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,}$)");
    return re.match(s).hasMatch();
}

void AuthWindow::attemptLogin() {
    const QString email = loginEmail_->text().trimmed();
    const QString pw = loginPassword_->text();
    if (email.isEmpty() || pw.isEmpty()) { loginError_->setText("Please enter your username/email and password."); return; }
    try {
        UserInfo user;
        Result r = backend_->login(email, pw, user);
        if (!r.ok) { loginError_->setText(r.message); loginPassword_->clear(); return; }
        loginError_->clear();
        reset();
        emit loggedIn(user);
    } catch (const std::exception&) {
        Alert::showError(this, "An error occurred while signing in. Please try again.");
    }
}

void AuthWindow::attemptRegister() {
    const QString name = regName_->text().trimmed(), email = regEmail_->text().trimmed();
    const QString pw = regPassword_->text(), confirm = regConfirm_->text();
    if (name.isEmpty() || email.isEmpty() || pw.isEmpty() || confirm.isEmpty()) { regError_->setText("Please fill in all fields."); return; }
    if (!isValidEmail(email)) { regError_->setText("Enter a valid email address (e.g. name@example.com)."); return; }
    if (pw.size() < 8)        { regError_->setText("Password must be at least 8 characters."); return; }
    if (pw != confirm)        { regError_->setText("Passwords do not match."); regConfirm_->clear(); return; }
    try {
        Result r = backend_->registerUser(name, email, pw);
        if (!r.ok) { regError_->setText(r.message); return; }
        const QString registeredEmail = email;
        reset();
        stack_->setCurrentIndex(0);
        loginEmail_->setText(registeredEmail);
        loginPassword_->setFocus();
        Alert::showSuccess(this, "Account created successfully. You can now sign in.");
    } catch (const std::exception&) {
        Alert::showError(this, "An error occurred while saving the changes.");
    }
}

void AuthWindow::reset() {
    for (auto* e : {loginEmail_, loginPassword_, regName_, regEmail_, regPassword_, regConfirm_}) e->clear();
    loginError_->clear(); regError_->clear();
    stack_->setCurrentIndex(0);
}
