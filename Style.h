#pragma once
#include <QString>

inline QString appStyleSheet() {
    return R"(
* { 
    font-family: "Segoe UI", "Noto Sans", sans-serif; 
    font-size: 10pt; 
    color: #1e293b; 
}

QWidget#root, QDialog, QMessageBox { 
    background-color: #f8fafc; 
    color: #0f172a; 
}

/* Card Container */
QFrame#card { 
    background-color: #ffffff; 
    border: 1px solid #cbd5e1; 
    border-radius: 8px; 
}

/* Topbar & Headers */
QFrame#topbar { 
    background-color: #ffffff; 
    border-bottom: 1px solid #e2e8f0; 
}

QLabel#pageTitle { 
    font-size: 16pt; 
    font-weight: 700; 
    color: #0f172a; 
}

QLabel { 
    color: #334155; 
    font-weight: 500;
}

QLabel#hint { 
    color: #64748b; 
    font-size: 9pt; 
}

QLabel#loginError, QLabel#regError { 
    color: #dc2626; 
    font-weight: 600; 
}

/* Inputs */
QLineEdit, QSpinBox, QComboBox { 
    background-color: #ffffff; 
    border: 1px solid #cbd5e1; 
    border-radius: 6px; 
    padding: 8px 10px; 
    color: #0f172a;
}

QLineEdit:focus, QSpinBox:focus, QComboBox:focus { 
    border: 2px solid #2563eb; 
}

/* Buttons */
QPushButton { 
    background-color: #ffffff; 
    border: 1px solid #cbd5e1; 
    border-radius: 6px; 
    padding: 8px 16px; 
    font-weight: 600;
    color: #334155;
}

QPushButton:hover { 
    background-color: #f1f5f9; 
    border-color: #94a3b8;
}

QPushButton#primary { 
    background-color: #2563eb; 
    border: none; 
    color: #ffffff; 
}

QPushButton#primary:hover { 
    background-color: #1d4ed8; 
}

QPushButton#danger { 
    background-color: #dc2626; 
    border: none; 
    color: #ffffff; 
}

QPushButton#danger:hover { 
    background-color: #b91c1c; 
}

QPushButton#linkButton { 
    border: none; 
    background: transparent; 
    color: #2563eb; 
    font-weight: 600;
}

QPushButton#linkButton:hover { 
    text-decoration: underline; 
}

/* Sidebar Navigation */
QFrame#sidebar { 
    background-color: #0f172a; 
}

QPushButton#navButton { 
    color: #94a3b8; 
    background: transparent; 
    border: none; 
    text-align: left; 
    padding: 12px 20px; 
    font-size: 11pt;
}

QPushButton#navButton:hover { 
    background-color: #1e293b; 
    color: #f8fafc;
}

QPushButton#navButton:checked { 
    background-color: #2563eb; 
    color: #ffffff; 
    font-weight: bold;
}

/* ============================================================
   TABLE STYLING (FIXES THE BLACK BOX)
   ============================================================ */
QTableWidget { 
    background-color: #ffffff; 
    color: #0f172a;
    border: 1px solid #cbd5e1; 
    border-radius: 6px;
    gridline-color: #f1f5f9; 
    selection-background-color: #dbeafe; /* Soft blue highlight */
    selection-color: #1e40af;             /* Dark blue text when selected */
}

QTableWidget::item { 
    padding: 6px; 
    color: #0f172a;
}

QHeaderView::section { 
    background-color: #f1f5f9; 
    color: #475569; 
    border: none; 
    border-bottom: 2px solid #cbd5e1; 
    padding: 8px 10px; 
    font-weight: 700; 
    font-size: 9.5pt;
}

QTableCornerButton::section {
    background-color: #f1f5f9;
    border: none;
}
)";
}