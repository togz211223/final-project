/**
 * SMART LIBRARY & DIGITAL ASSET MANAGEMENT SYSTEM
 * Core Backend & Business Logic Template
 * 
 * Instructions for the Team:
 * 1. Do not add GUI code to this file. This file strictly handles OOP, 
 *    PostgreSQL logic, and design patterns.
 * 2. Look for the "// TODO:" tags to assign Jira tasks to team members.
 * 3. Once this console test runs successfully, this file will be split into
 *    proper .h and .cpp files inside your Qt project.
 */

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <ctime>

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
// DESIGN PATTERN 1: STRATEGY (Fine Calculation)
// ============================================================

class IFineStrategy {
public:
    virtual ~IFineStrategy() = default;
    
    // Calculates total fine based on how many days overdue
    virtual double calculateFine(int daysLate) const = 0;
};

class PhysicalBookFine : public IFineStrategy {
public:
    double calculateFine(int daysLate) const override {
        // TODO: Implement logic (e.g., $0.50 per day late, max cap of $20)
        return 0.0;
    }
};

class LaptopFine : public IFineStrategy {
public:
    double calculateFine(int daysLate) const override {
        // TODO: Implement logic (e.g., $5.00 per day late, no maximum cap)
        return 0.0;
    }
};

class EBookFine : public IFineStrategy {
public:
    double calculateFine(int daysLate) const override {
        // TODO: Implement logic (E-Books automatically return themselves, fine should be 0.0)
        return 0.0;
    }
};

// ============================================================
// CORE ENTITIES (Inheritance / Polymorphism)
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
    double getFines() const { return totalUnpaidFines; }
    
    // TODO: Add standard getters and setters
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

    // Polymorphic fine calculator mapping to the injected strategy
    double calculateLateFine(int daysLate) const {
        return fineStrategy->calculateFine(daysLate);
    }

    // Virtual function showing Polymorphism requirement
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
        // TODO: Implement cout statement specifically formatted for Books
    }
};

class Laptop : public Asset {
private:
    string serialNumber;
public:
    Laptop(int id, string t, string sn) 
        : Asset(id, t, new LaptopFine()), serialNumber(sn) {}

    void printDetails() const override {
        // TODO: Implement cout statement specifically formatted for Tech/Laptops
    }
};

// ============================================================
// DESIGN PATTERN 2: FACTORY (Database Entity Creation)
// ============================================================

class AssetFactory {
public:
    static Asset* createAssetFromDBRow(int id, string title, string typeStr, string extraData1, string extraData2) {
        // TODO: Implement Factory logic
        // 1. If typeStr == "Book", return new PhysicalBook(...)
        // 2. If typeStr == "Laptop", return new Laptop(...)
        // 3. This is essential when parsing the PostgreSQL 'SELECT * FROM Assets' query
        return nullptr;
    }
};

// ============================================================
// DESIGN PATTERN 3: SINGLETON (Database Manager)
// ============================================================

class DatabaseManager {
private:
    static DatabaseManager* instance;
    bool isConnected;
    
    // Private constructor guarantees Singleton pattern
    DatabaseManager() {
        isConnected = false;
    }

public:
    // Delete copy constructor and assignment operator to enforce Singleton
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static DatabaseManager* getInstance() {
        if (instance == nullptr) {
            instance = new DatabaseManager();
        }
        return instance;
    }

    bool connectToPostgres(string connectionString) {
        // TODO: Write Qt SQL or libpqxx connection setup here
        // If success:
        // isConnected = true; return true;
        // Else print DB error to console
        return true; 
    }

    void testConnection() {
        if(isConnected) cout << "SUCCESS: Connected to PostgreSQL." << endl;
        else cout << "ERROR: Database disconnected." << endl;
    }

    // --- C.R.U.D Stub Functions ---

    User* queryUserById(int userId) {
        // TODO: "SELECT * FROM Users WHERE user_id = userId;"
        // Reconstruct user object and return pointer
        return nullptr;
    }
    
    void updateBorrowRecord(int assetId, int userId, string dueDate) {
        // TODO: "INSERT INTO Borrow_Records (asset_id, user_id, due_date) VALUES (...);"
    }
};

// Initialize the static instance
DatabaseManager* DatabaseManager::instance = nullptr;

// ============================================================
// BUSINESS LOGIC MANAGER (State transitions & Logic)
// ============================================================

class LibraryManager {
private:
    DatabaseManager* db;
    // Mock memory structures just to run local tests before Postgres is wired up
    vector<Asset*> assetCache;
    queue<int> waitlistMock; // Simulating PostgreSQL Waitlist chronological query

public:
    LibraryManager() {
        db = DatabaseManager::getInstance();
    }

    void loadSystem() {
        cout << "[System] Connecting to database..." << endl;
        db->connectToPostgres("dbname=smart_lib user=admin password=root host=localhost");
    }

    /**
     * Borrow logic representing State Changes
     */
    bool borrowAsset(int userId, Asset* asset) {
        // Step 1: Validate
        if (asset->getState() != AssetState::AVAILABLE) {
            cout << "[Denied] " << asset->getTitle() << " is not available." << endl;
            // TODO: Here, prompt UI to insert user into the Postgres Waitlist table
            return false;
        }

        // TODO: Retrieve user from DB, verify user.totalUnpaidFines < 10.00
        
        // Step 2: Update states
        asset->setState(AssetState::BORROWED);
        
        // Step 3: Run Database Queries
        db->updateBorrowRecord(asset->getId(), userId, "2026-08-30");
        // db->updateAssetDbState(asset->getId(), 'BORROWED');
        
        cout << "[Success] User " << userId << " checked out " << asset->getTitle() << endl;
        return true;
    }

    /**
     * Return Logic with Waitlist & Fines (Queue constraint testing)
     */
    void returnAsset(int userId, Asset* asset, int daysOverdue) {
        // Step 1: Process Fines (Polymorphic strategy calculation)
        if (daysOverdue > 0) {
            double fine = asset->calculateLateFine(daysOverdue);
            cout << "[Fine Alert] Assessed late fee of $" << fine << " to User " << userId << endl;
            // TODO: "UPDATE Users SET total_fines = total_fines + fine WHERE id = userId"
        }

        // Step 2: Update original transaction record
        // TODO: "UPDATE Borrow_Records SET return_date = NOW() WHERE user_id = X AND asset_id = Y"

        // Step 3: SMART WAITLIST CHECK
        cout << "[System] Checking Waitlist for asset: " << asset->getTitle() << endl;
        // TODO: Implement Query: "SELECT user_id FROM Waitlist WHERE asset_id = X ORDER BY request_date ASC LIMIT 1;"
        
        bool waitlistExists = false; // Mock toggle
        
        if (waitlistExists) {
            asset->setState(AssetState::WAITLISTED);
            int nextUserId = 99; // Mock from DB
            cout << "[Automated] Asset instantly reserved for Waitlisted User: " << nextUserId << endl;
            // TODO: "DELETE FROM Waitlist WHERE user_id = 99 and asset_id = X;"
        } else {
            asset->setState(AssetState::AVAILABLE);
            cout << "[Returned] Asset is available in circulation." << endl;
        }
    }
};

// ============================================================
// MAIN FUNCTION (TEST HARNESS)
// ============================================================

int main() {
    cout << "========================================================\n";
    cout << " SMART LIBRARY & DIGITAL ASSET MGR - SKELETON HARNESS\n";
    cout << "========================================================\n\n";

    // 1. Startup & Connect DB
    LibraryManager library;
    library.loadSystem();

    // 2. Setup mock objects (Usually pulled via Factory & Postgres)
    cout << "\n[Test Phase 1] Building OOP Models..." << endl;
    Asset* cbpBook = new PhysicalBook(101, "C++ Object Oriented Programming", "978-3-16-148410-0", "Bjarne S.");
    Asset* uniLaptop = new Laptop(500, "MacBook Pro M3 - CS Dept", "SN-934X1221");
    
    // 3. Test Valid Borrow Transaction
    cout << "\n[Test Phase 2] Testing valid checkout..." << endl;
    library.borrowAsset(1, cbpBook);
    
    // 4. Test Waitlist Trigger (Item already borrowed)
    cout << "\n[Test Phase 3] Testing collision & waitlist..." << endl;
    library.borrowAsset(2, cbpBook); // Should deny and prompt waitlist
    
    // 5. Test Return and Overdue fines calculation (Strategy checking)
    cout << "\n[Test Phase 4] Returning objects with Fines..." << endl;
    cout << "Returning book 3 days late:" << endl;
    library.returnAsset(1, cbpBook, 3); // Polymorphic Fine Test 
    
    cout << "\nReturning Hardware 3 days late (Expected severe penalty):" << endl;
    uniLaptop->setState(AssetState::BORROWED);
    library.returnAsset(1, uniLaptop, 3); 

    // Clean up heap allocation
    delete cbpBook;
    delete uniLaptop;

    cout << "\n========================================================\n";
    cout << " End of Simulator Harness. Ensure //TODO sections run \n";
    cout << " cleanly through DB layer before hooking into GUI windows.\n";
    cout << "========================================================\n";
    return 0;
}