#pragma once
#include <QApplication>
#include <QDebug>
#include <stdexcept>
#include "Alert.h"

class SafeApplication : public QApplication {
public:
    using QApplication::QApplication;

    bool notify(QObject* receiver, QEvent* event) override {
        try {
            return QApplication::notify(receiver, event);
        } catch (const std::out_of_range& e) {
            report("Something tried to access data that does not exist.", e.what());
        } catch (const std::exception& e) {
            report("An unexpected error occurred. The application is still running.", e.what());
        } catch (...) {
            report("An unknown error occurred. The application is still running.", "unknown");
        }
        return false;   
    }

private:
    void report(const QString& userMessage, const char* detail) {
        qWarning() << "[SafeApplication] caught:" << detail;
        if (showing_) return;         
        showing_ = true;
        Alert::showError(activeWindow(), userMessage);   
        showing_ = false;
    }
    bool showing_ = false;
};
