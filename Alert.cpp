#include "Alert.h"
#include <QMessageBox>
#include <QPushButton>

static std::function<bool(Alert::Kind, const QString&)> g_hook;
void Alert::setTestHook(std::function<bool(Kind, const QString&)> hook) { g_hook = std::move(hook); }

void Alert::showAlert(QWidget* parent, Kind kind, const QString& message, const QString& title) {
    if (g_hook) { g_hook(kind, message); return; }
    QMessageBox box(parent);
    switch (kind) {
        case Kind::Success: box.setIcon(QMessageBox::Information); box.setWindowTitle(title.isEmpty() ? "Success" : title); break;
        case Kind::Warning: box.setIcon(QMessageBox::Warning);     box.setWindowTitle(title.isEmpty() ? "Warning" : title); break;
        case Kind::Error:   box.setIcon(QMessageBox::Critical);    box.setWindowTitle(title.isEmpty() ? "Error" : title);   break;
        default:            box.setIcon(QMessageBox::Information); box.setWindowTitle(title.isEmpty() ? "Information" : title); break;
    }
    box.setText(message);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

bool Alert::showConfirmation(QWidget* parent, const QString& message, const QString& title,
                             const QString& confirmText, const QString& cancelText) {
    if (g_hook) return g_hook(Kind::Confirm, message);
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(title);
    box.setText(message);
    auto* confirm = box.addButton(confirmText, QMessageBox::AcceptRole);
    auto* cancel  = box.addButton(cancelText, QMessageBox::RejectRole);
    box.setDefaultButton(cancel);
    box.setEscapeButton(cancel);
    box.exec();
    return box.clickedButton() == confirm;
}
