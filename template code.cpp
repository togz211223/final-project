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
#include <map>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>

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
        if (daysLate <= 0) {
            return 0.0;
        }
        double fine = daysLate * 0.50;
        if (fine > 20.0) {
            fine = 20.0;
        }
        return fine;
    }
};

class LaptopFine : public IFineStrategy {
public:
    double calculateFine(int daysLate) const override {
        if (daysLate <= 0) {
            return 0.0;
        }
        return daysLate * 5.00;
    }
};

class EBookFine : public IFineStrategy {
public:
    double calculateFine(int daysLate) const override {
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
        // TODO (Member 8 - Story 23): Implement writing to CSV
        cout << "[Exporter] Writing CSV report to " << filePath << "..." << endl;
        return true;
    }
};

class ReceiptExporter : public IReportExporter {
public:
    bool exportReport(const string& filePath, const ReportData& data) override {
        // TODO (Member 8 - Story 23): Implement checkout slip creation
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
        cout << "[Book] ID: " << assetId << " | " << title << " by " << author 
             << " (ISBN: " << isbn << ")" << endl;
    }
};

class EBook : public Asset {
public:
    EBook(int id, string t) 
        : Asset(id, t, new EBookFine()) {}

    void printDetails() const override {
        cout << "[E-Book] ID: " << assetId << " | " << title << endl;
    }
};

class Laptop : public Asset {
private:
    string serialNumber;
public:
    Laptop(int id, string t, string sn) 
        : Asset(id, t, new LaptopFine()), serialNumber(sn) {}

    void printDetails() const override {
        cout << "[Hardware] ID: " << assetId << " | " << title 
             << " (S/N: " << serialNumber << ")" << endl;
    }
};

// ============================================================
// DESIGN PATTERN 3: FACTORY (Asset Creation - Member 2)
// ============================================================

class AssetFactory {
public:
    static Asset* createAssetFromDBRow(int id, string title, string typeStr, string extraData1, string extraData2) {
        if (typeStr == "Book") {
            return new PhysicalBook(id, title, extraData1, extraData2);
        } else if (typeStr == "EBook") {
            return new EBook(id, title);
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
        // TODO (Member 1 - Story 3): "INSERT INTO Borrow_Records (...)"
    }
};

DatabaseManager* DatabaseManager::instance = nullptr;

// ============================================================
// BUSINESS LOGIC: CIRCULATION MANAGER (Member 3 Logic Merged)
// ============================================================

class LibraryManager {
private:
    DatabaseManager* db;
    // Multi-asset Queue System: Maps specific Asset ID to a Queue of User IDs
    std::map<int, std::queue<int>> waitlistMock;

public:
    LibraryManager() {
        db = DatabaseManager::getInstance();
    }

    void loadSystem() {
        cout << "[System] Connecting to database..." << endl;
        db->connectToPostgres("dbname=smart_lib user=admin password=root host=localhost");
    }

    // Member 3 Logic: Safe queue extraction algorithm
    bool resolveWaitlistQueue(Asset* asset) {
        if (asset == nullptr) return false;
        std::cout << "[System] Checking Waitlist queue for Asset ID: " << asset->getId() << std::endl;
        
        auto it = waitlistMock.find(asset->getId());
        bool hasWaitlistUser = (it != waitlistMock.end() && !it->second.empty());
        
        if (hasWaitlistUser) {
            int nextWaitingUserId = it->second.front();
            it->second.pop();
            
            // Clean up empty queue mappings from memory
            if (it->second.empty()) waitlistMock.erase(it);
            
            asset->setState(AssetState::WAITLISTED);
            std::cout << "[Automated Queue] Asset instantly reserved for next waitlisted User: "<< nextWaitingUserId << std::endl;
            return true;
        }
        
        asset->setState(AssetState::AVAILABLE);
        std::cout << "[System] No waitlist entries found. Asset '" << asset->getTitle() << "' is now AVAILABLE in circulation." << std::endl;
        return false;
    }

    // Member 3 Logic: Waitlist insertion
    bool addUserToWaitlist(int userId, int assetId) {
        if (userId <= 0 || assetId <= 0) {
            std::cout << "[Error] Invalid user or asset ID for waitlist." << std::endl;
            return false;
        }
        
        // Safety guard against adding same user twice
        std::queue<int> copy = waitlistMock[assetId];
        while (!copy.empty()) {
            if (copy.front() == userId) {
                std::cout << "[Waitlist] User " << userId << " is already waiting for Asset ID: " << assetId << std::endl;
                return false;
            }
            copy.pop();
        }
        
        waitlistMock[assetId].push(userId);
        std::cout << "[Waitlist] User " << userId << " added to waitlist queue for Asset ID: " << assetId << std::endl;
        return true;
    }

    // Member 3 Logic: Borrow algorithm with fine/state verification
    bool borrowAsset(int userId, Asset* asset) {
        if (asset == nullptr) {
            std::cout << "[Error] Invalid asset reference provided." << std::endl;
            return false;
        }
        if (asset->getState() == AssetState::MAINTENANCE) {
            std::cout << "[Denied] Asset '" << asset->getTitle() << "' is under maintenance." << std::endl;
            return false;
        }
        if (asset->getState() != AssetState::AVAILABLE) {
            std::cout << "[Denied] Asset '" << asset->getTitle() << "' is currently unavailable." << std::endl;
            std::cout << "[Action Required] Prompt user " << userId << " to join the Waitlist." << std::endl;
            return false;
        }

        const double MAX_UNPAID_FINE_LIMIT = 10.00;
        User* user = db->queryUserById(userId); // Checks db via Member 1 stub
        
        if (user != nullptr) {
            double fines = user->getFines();
            delete user; // safe memory cleanup
            
            if (fines >= MAX_UNPAID_FINE_LIMIT) {
                std::ostringstream finesStr, limitStr;
                finesStr << std::fixed << std::setprecision(2) << fines;
                limitStr << std::fixed << std::setprecision(2) << MAX_UNPAID_FINE_LIMIT;
                std::cout << "[Denied] User " << userId << " has unpaid fines ($" << finesStr.str() 
                          << ") reaching the limit ($" << limitStr.str() << "). Borrowing blocked." << std::endl;
                return false;
            }
        }
        else {
            std::cout << "[Warning] User " << userId << " not found in DB (stub) - continuing checkout simulation." << std::endl;
        }
        
        const int LOAN_DAYS = 14;
        std::time_t dueTime = std::time(nullptr) + static_cast<std::time_t>(LOAN_DAYS) * 24 * 60 * 60;
        char dueBuf[11];
        std::strftime(dueBuf, sizeof(dueBuf), "%Y-%m-%d", std::localtime(&dueTime));
        std::string dueDate(dueBuf);
        
        asset->setState(AssetState::BORROWED);
        db->updateBorrowRecord(asset->getId(), userId, dueDate);
        std::cout << "[Success] User " << userId << " checked out '" << asset->getTitle() << "' (due: " << dueDate << ")." << std::endl;
        return true;
    }

    // Member 3 Logic: Processing logic combining Fines + Strategy + Automated Queues
    void returnAsset(int userId, Asset* asset, int daysOverdue) {
        if (asset == nullptr) {
            std::cout << "[Error] Cannot process return for null asset." << std::endl;
            return;
        }
        if (asset->getState() != AssetState::BORROWED) {
            std::cout << "[Error] Asset '" << asset->getTitle() << "' is not currently borrowed. Return ignored." << std::endl;
            return;
        }
        
        std::cout << "\n[System] Processing return for asset: " << asset->getTitle() << " from User: " << userId << std::endl;
        
        // Execute Fine Algorithms
        if (daysOverdue > 0) {
            double fineAmount = asset->calculateLateFine(daysOverdue); // Polymorphic call
            std::ostringstream fineStr;
            fineStr << std::fixed << std::setprecision(2) << fineAmount;
            std::cout << "[Fine Alert] Asset returned " << daysOverdue << " days late. Calculated fine: $" << fineStr.str() << std::endl;
            
            if (fineAmount > 0.0) {
                std::cout << "[DB Update] Added $" << fineStr.str() << " to unpaid fines for User " << userId << std::endl;
            }
        }
        else {
            std::cout << "[Success] Item returned on time. No late fee assessed." << std::endl;
        }
        
        std::cout << "[DB Update] Borrow record closed for Asset ID: " << asset->getId() << std::endl;
        
        // Route back into automated workflow triggers
        resolveWaitlistQueue(asset);
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
        LibraryStats stats;
        stats.totalActiveLoans = 14;
        stats.overdueCount = 3;
        stats.totalUnpaidFines = 45.50;
        stats.topBorrowedAssets = {"C++ Primer", "MacBook Pro M3", "Clean Code"};
        return stats;
    }

    ReportData getFilteredBorrowHistory(string startDate, string endDate, string assetType) {
        ReportData report;
        report.reportTitle = "Borrow History Report (" + startDate + " to " + endDate + ")";
        report.headers = {"Record ID", "User ID", "Asset Title", "Borrow Date", "Status"};
        report.rows = {
            {"1001", "1", "C++ Primer", "2026-08-01", "RETURNED"},
            {"1002", "3", "MacBook Pro M3", "2026-08-05", "ACTIVE"}
        };
        return report;
    }
};
