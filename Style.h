#pragma once
#include <QString>
// One stylesheet for the whole app (login, dashboard, dialogs, popups).
inline QString appStyleSheet() {
    return R"(
* { font-family: "Segoe UI", "Noto Sans", "DejaVu Sans", sans-serif; font-size: 10pt; }
QWidget#root, QDialog, QMessageBox { background: #f4f5f7; color: #1f2933; }
QLabel#pageTitle { font-size: 15pt; font-weight: 600; }
QLabel#hint { color: #6b7280; }
QLabel#loginError, QLabel#regError { color: #b42318; }
QFrame#card { background: #ffffff; border: 1px solid #d5d9e0; border-radius: 4px; }
QFrame#sidebar { background: #1f2d3d; }
QFrame#topbar { background: #ffffff; border-bottom: 1px solid #d5d9e0; }
QPushButton#navButton { color: #cfd8e3; background: transparent; border: none; text-align: left; padding: 10px 18px; }
QPushButton#navButton:hover { background: #2a3b4f; }
QPushButton#navButton:checked { background: #34506e; color: #ffffff; }
QLineEdit, QSpinBox, QComboBox { background: #ffffff; border: 1px solid #b8c0cc; border-radius: 3px; padding: 5px 7px; }
QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1px solid #2f6db5; }
QPushButton { background: #ffffff; border: 1px solid #b8c0cc; border-radius: 3px; padding: 6px 16px; }
QPushButton:hover { background: #eef1f5; }
QPushButton:disabled { color: #9aa3af; background: #eceef1; }
QPushButton#primary { background: #2f6db5; border: 1px solid #285d9a; color: #ffffff; }
QPushButton#primary:hover { background: #285d9a; }
QPushButton#danger { background: #b42318; border: 1px solid #9a1d14; color: #ffffff; }
QPushButton#danger:hover { background: #9a1d14; }
QPushButton#linkButton { border: none; background: transparent; color: #2f6db5; padding: 2px; }
QTableWidget { background: #ffffff; border: 1px solid #d5d9e0; gridline-color: #e5e8ed; selection-background-color: #d6e4f5; selection-color: #1f2933; }
QHeaderView::section { background: #e9edf2; border: none; border-bottom: 1px solid #d5d9e0; padding: 6px; font-weight: 600; }
)";
}
