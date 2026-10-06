#pragma once
#include <QString>
#include <functional>
class QWidget;

// Single, app-wide popup system. All screens must use this instead of QMessageBox.
class Alert {
public:
    enum class Kind { Info, Success, Warning, Error, Confirm };

    static void showAlert(QWidget* parent, Kind kind, const QString& message, const QString& title = {});
    static void showInfo(QWidget* parent, const QString& message)    { showAlert(parent, Kind::Info, message); }
    static void showSuccess(QWidget* parent, const QString& message) { showAlert(parent, Kind::Success, message); }
    static void showWarning(QWidget* parent, const QString& message) { showAlert(parent, Kind::Warning, message); }
    static void showError(QWidget* parent, const QString& message)   { showAlert(parent, Kind::Error, message); }
    // Returns true only if the user explicitly presses the confirm button (Cancel is the default).
    static bool showConfirmation(QWidget* parent, const QString& message, const QString& title = "Please confirm",
                                 const QString& confirmText = "Confirm", const QString& cancelText = "Cancel");

    // Automated tests: replace modal dialogs. Hook receives kind+message, returns the confirmation answer.
    static void setTestHook(std::function<bool(Kind, const QString&)> hook);
};
