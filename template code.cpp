/**
 * SMART LIBRARY & DIGITAL ASSET MANAGEMENT SYSTEM
 * Core Backend & Business Logic Template
 * 
 * Instructions for the Team:
 * 1. Do not add GUI code to this file. This file strictly handles OOP, 
 *    PostgreSQL logic, design patterns, and file I/O.
 * 2. Look for the "// TODO (Member X):" tags to assign Jira tasks to team members.
 * 3. Once this console test runs successfully, this file will be split into
 *    proper .h and .cpp files inside your Qt project.
 */

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <ctime>
#include <fstream>
#include <sstream>

// Use #include <QSqlDatabase> if using Qt, or <pqxx/pqxx> if using pure C++ driver
// #include <QSqlDatabase> 
// #include <QSqlQuery>

using namespace std;

// ============================================================
// ENUMS & UTILITY
// ============================================================

enum class AssetState { AVAILABLE, BORROWED, WAITLISTED, MAINTENANCE };
enum class AssetType { PHYSICAL_BOOK, E_BOOK, LAPTOP };
enum class UserRole { ADMIN, MEMBER };

// ============================================================
// DESIGN PATTERN 1: STRATEGY (Fine Calculation - Member 2)
// ============================================================

class IFineStrategy {
public:
    virtual ~IFineStrategy() = default;
    virtual double calculateFine(int daysLate) const = 0;
};

class PhysicalBookFine : public IFineStrategy {
public:
    double calculateFine(int daysLate) const override {
        // TODO (Member 2): Implement logic (e.g., $0.50 per day late, max cap of $20)
        return daysLate * 0.50;
    }
};

class LaptopFine : public IFineStrategy {
public:
    double calculateFine(int daysLate) const override {
        // TODO (Member 2): Implement logic (e.g., $5.00 per day late, no maximum cap)
        return daysLate * 5.00;
    }
};

class EBookFine : public IFineStrategy {
public:
    double calculateFine(int daysLate) const override {
        // TODO (Member 2): E-Books automatically revoke access; fine is always $0.00
        return 0.0;
    }
};

// ============================================================
// DESIGN PATTERN 2: STRATEGY (Report & Receipt Exporting - Member 8)
// ============================================================

struct ReportData {
    string reportTitle;
    vector<string> headers;
    vector<vector<string>> rows;
};

class IReportExporter {
public:
    virtual ~IReportExporter() = default;
    virtual bool exportReport(const string& filePath, const ReportData& data) = 0;
};

class CSVReportExporter : public IReportExporter {
public:
    bool exportReport(const string& filePath, const ReportData& data) override {
        // TODO (Member 8 - Story 23):
        // 1. Open ofstream file at filePath.
        // 2. Write headers separated by commas.
        // 3. Write each row separated by commas.
        // 4. Handle fstream errors cleanly.
        cout << "[Exporter] Writing CSV report to " << filePath << "..." << endl;
        return true;
    }
};

class ReceiptExporter : public IReportExporter {
public:
    bool exportReport(const string& filePath, const ReportData& data) override {
        // TODO (Member 8 - Story 23):
        // 1. Open ofstream file at filePath.
        // 2. Format a clean checkout slip (Title, Borrower, Due Date, Timestamp).
        // 3. Close file safely.
        cout << "[Exporter] Printing checkout slip to " << filePath << "..." << endl;
        return true;
    }
};

// ============================================================
// CORE ENTITIES (Inheritance / Polymorphism - Member 2)
// ============================================================

class User {
private:
    int userId;
    string name;
    string email;
    UserRole role;
    double totalUnpaidFines;

public:
    User(int id, string n, string e, UserRole r) 
        : userId(id), name(n), email(e), role(r), totalUnpaidFines(0.0) {}
    
    int getId() const { return userId; }
    string getName() const { return name; }
    double getFines() const { return totalUnpaidFines; }
    void addFine(double amount) { totalUnpaidFines += amount; }
};

class Asset {
protected:
    int assetId;
    string title;
    AssetState state;
    IFineStrategy* fineStrategy;

public:
    Asset(int id, string t, IFineStrategy* strategy) 
        : assetId(id), title(t), state(AssetState::AVAILABLE), fineStrategy(strategy) {}
    
    virtual ~Asset() {
        delete fineStrategy;
    }

    int getId() const { return assetId; }
    AssetState getState() const { return state; }
    string getTitle() const { return title; }
    void setState(AssetState newState) { state = newState; }

    double calculateLateFine(int daysLate) const {
        return fineStrategy->calculateFine(daysLate);
    }

    virtual void printDetails() const = 0; 
};

class PhysicalBook : public Asset {
private:
    string isbn;
    string author;
public:
    PhysicalBook(int id, string t, string i, string a) 
        : Asset(id, t, new PhysicalBookFine()), isbn(i), author(a) {}

    void printDetails() const override {
        cout << "[Book] ID: " << assetId << " | " << title << " by " << author << " (ISBN: " << isbn << ")" << endl;
    }
};

class Laptop : public Asset {
private:
    string serialNumber;
public:
    Laptop(int id, string t, string sn) 
        : Asset(id, t, new LaptopFine()), serialNumber(sn) {}

    void printDetails() const override {
        cout << "[Hardware] ID: " << assetId << " | " << title << " (S/N: " << serialNumber << ")" << endl;
    }
};

// ============================================================
// DESIGN PATTERN 3: FACTORY (Asset Creation - Member 2)
// ============================================================

class AssetFactory {
public:
    static Asset* createAssetFromDBRow(int id, string title, string typeStr, string extraData1, string extraData2) {
        // TODO (Member 2): Parse DB type string and instantiate proper subclass
        if (typeStr == "Book") {
            return new PhysicalBook(id, title, extraData1, extraData2);
        } else if (typeStr == "Laptop") {
            return new Laptop(id, title, extraData1);
        }
        return nullptr;
    }
};

// ============================================================
// DESIGN PATTERN 4: SINGLETON (Database Manager - Member 1)
// ============================================================

class DatabaseManager {
private:
    static DatabaseManager* instance;
    bool isConnected;
    
    DatabaseManager() {
        isConnected = false;
    }

public:
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static DatabaseManager* getInstance() {
        if (instance == nullptr) {
            instance = new DatabaseManager();
        }
        return instance;
    }

    bool connectToPostgres(string connectionString) {
        // TODO (Member 1 - Story 2): Add real PostgreSQL connection logic here
        isConnected = true; 
        return true; 
    }

    void testConnection() {
        if (isConnected) cout << "SUCCESS: Connected to PostgreSQL." << endl;
        else cout << "ERROR: Database disconnected." << endl;
    }

    // --- C.R.U.D Helper Stubs ---
    User* queryUserById(int userId) {
        // TODO (Member 1 - Story 3): "SELECT * FROM Users WHERE user_id = userId;"
        return nullptr;
    }
    
    void updateBorrowRecord(int assetId, int userId, string dueDate) {
        // TODO (Member 1 - Story 3): "INSERT INTO Borrow_Records (asset_id, user_id, due_date) VALUES (...);"
    }
};

DatabaseManager* DatabaseManager::instance = nullptr;

// ============================================================
// BUSINESS LOGIC: CIRCULATION MANAGER (Member 3)
// ============================================================

class LibraryManager {
private:
    DatabaseManager* db;

public:
    LibraryManager() {
        db = DatabaseManager::getInstance();
    }

    void loadSystem() {
        cout << "[System] Connecting to database..." << endl;
        db->connectToPostgres("dbname=smart_lib user=admin password=root host=localhost");
    }

    bool borrowAsset(int userId, Asset* asset) {
        // TODO (Member 3 - Story 7): 
        // 1. Check if asset state is AVAILABLE
        // 2. Verify user fines < $10.00
        if (asset->getState() != AssetState::AVAILABLE) {
            cout << "[Denied] " << asset->getTitle() << " is not available." << endl;
            return false;
        }

        asset->setState(AssetState::BORROWED);
        db->updateBorrowRecord(asset->getId(), userId, "2026-08-30");
        cout << "[Success] User " << userId << " checked out " << asset->getTitle() << endl;
        return true;
    }

    void returnAsset(int userId, Asset* asset, int daysOverdue) {
        // TODO (Member 3 - Story 8): 
        // 1. Calculate fine via asset->calculateLateFine(daysOverdue)
        // 2. Update DB borrow record
        if (daysOverdue > 0) {
            double fine = asset->calculateLateFine(daysOverdue);
            cout << "[Fine Alert] Assessed late fee of $" << fine << " to User " << userId << endl;
        }

        // TODO (Member 3 - Story 9): Waitlist FIFO Resolution Algorithm
        cout << "[System] Checking Waitlist for asset: " << asset->getTitle() << endl;
        bool waitlistExists = false; // Mock toggle

        if (waitlistExists) {
            asset->setState(AssetState::WAITLISTED);
            int nextUserId = 99; // Mock from FIFO query
            cout << "[Automated] Asset instantly reserved for Waitlisted User: " << nextUserId << endl;
        } else {
            asset->setState(AssetState::AVAILABLE);
            cout << "[Returned] Asset is available in circulation." << endl;
        }
    }
};

// ============================================================
// BUSINESS LOGIC: ANALYTICS & REPORTING ENGINE (Member 8)
// ============================================================

struct LibraryStats {
    int totalActiveLoans;
    int overdueCount;
    double totalUnpaidFines;
    vector<string> topBorrowedAssets;
};

class AnalyticsEngine {
private:
    DatabaseManager* db;

public:
    AnalyticsEngine() {
        db = DatabaseManager::getInstance();
    }

    LibraryStats calculateSystemStats() {
        // TODO (Member 8 - Story 22):
        // Run aggregation queries on PostgreSQL:
        // 1. "SELECT COUNT(*) FROM Borrow_Records WHERE return_date IS NULL;"
        // 2. "SELECT COUNT(*) FROM Borrow_Records WHERE return_date IS NULL AND due_date < NOW();"
        // 3. "SELECT SUM(fine_amount) FROM Borrow_Records;"
        // 4. "SELECT title, COUNT(*) FROM Borrow_Records GROUP BY title ORDER BY count DESC LIMIT 3;"
        
        LibraryStats stats;
        stats.totalActiveLoans = 14;
        stats.overdueCount = 3;
        stats.totalUnpaidFines = 45.50;
        stats.topBorrowedAssets = {"C++ Primer", "MacBook Pro M3", "Clean Code"};
        return stats;
    }

    ReportData getFilteredBorrowHistory(string startDate, string endDate, string assetType) {
        // TODO (Member 8 - Story 24):
        // Build parameterized SQL query using filters:
        // "SELECT record_id, user_id, asset_id, borrow_date FROM Borrow_Records WHERE ...;"
        
        ReportData report;
        report.reportTitle = "Borrow History Report (" + startDate + " to " + endDate + ")";
        report.headers = {"Record ID", "User ID", "Asset Title", "Borrow Date", "Status"};
        report.rows = {
            {"1001", "1", "C++ Primer", "2026-08-01", "RETURNED"},
            {"1002", "3", "MacBook Pro M3", "2026-08-05", "ACTIVE"},
            {"1003", "2", "Clean Code", "2026-08-10", "OVERDUE"}
        };
        return report;
    }
};

// ============================================================
// MAIN FUNCTION (TEST HARNESS)
// ============================================================

int main() {
    cout << "========================================================\n";
    cout << " SMART LIBRARY & DIGITAL ASSET MGR - SKELETON HARNESS\n";
    cout << "========================================================\n\n";

    // 1. Startup & Connect DB (Member 1)
    LibraryManager library;
    library.loadSystem();

    // 2. Setup mock objects via Factory (Member 2)
    cout << "\n[Test Phase 1] Building OOP Models..." << endl;
    Asset* cbpBook = AssetFactory::createAssetFromDBRow(101, "C++ Object Oriented Programming", "Book", "978-3-16", "Bjarne S.");
    Asset* uniLaptop = AssetFactory::createAssetFromDBRow(500, "MacBook Pro M3", "Laptop", "SN-934X1221", "");

    cbpBook->printDetails();
    uniLaptop->printDetails();
    
    // 3. Test Valid Borrow Transaction (Member 3)
    cout << "\n[Test Phase 2] Testing valid checkout..." << endl;
    library.borrowAsset(1, cbpBook);
    
    // 4. Test Waitlist Trigger (Member 3)
    cout << "\n[Test Phase 3] Testing collision & waitlist..." << endl;
    library.borrowAsset(2, cbpBook); // Denied -> prompts waitlist
    
    // 5. Test Return and Strategy Fine Calculations (Member 2 & 3)
    cout << "\n[Test Phase 4] Returning objects with Fines..." << endl;
    library.returnAsset(1, cbpBook, 3); // Book: $0.50 * 3 = $1.50
    
    uniLaptop->setState(AssetState::BORROWED);
    library.returnAsset(1, uniLaptop, 3); // Laptop: $5.00 * 3 = $15.00

    // 6. Test Analytics Engine (Member 8 - Story 22 & 24)
    cout << "\n[Test Phase 5] Testing Analytics Engine (Member 8)..." << endl;
    AnalyticsEngine analytics;
    LibraryStats stats = analytics.calculateSystemStats();
    cout << "Active Loans: " << stats.totalActiveLoans << endl;
    cout << "Overdue Count: " << stats.overdueCount << endl;
    cout << "Unpaid Fines: $" << stats.totalUnpaidFines << endl;
    cout << "Top Borrowed Asset: " << stats.topBorrowedAssets[0] << endl;

    // 7. Test Strategy Report Exporter (Member 8 - Story 23)
    cout << "\n[Test Phase 6] Testing Report Exporter Strategy (Member 8)..." << endl;
    ReportData sampleReport = analytics.getFilteredBorrowHistory("2026-08-01", "2026-08-31", "All");
    
    IReportExporter* csvExporter = new CSVReportExporter();
    csvExporter->exportReport("August_Report.csv", sampleReport);
    
    IReportExporter* receiptExporter = new ReceiptExporter();
    receiptExporter->exportReport("Receipt_1001.txt", sampleReport);

    // Clean up heap allocation
    delete cbpBook;
    delete uniLaptop;
    delete csvExporter;
    delete receiptExporter;

    cout << "\n========================================================\n";
    cout << " End of Simulator Harness. All 8 Roles Verified.\n";
    cout << "========================================================\n";
    return 0;
}
