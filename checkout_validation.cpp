#include <iostream>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

bool LibraryManager::borrowAsset(int userId, Asset* asset) {
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
    User* user = db->queryUserById(userId);
    if (user != nullptr) {
        double fines = user->getFines();
        delete user;
        if (fines >= MAX_UNPAID_FINE_LIMIT) {
            std::ostringstream finesStr, limitStr;
            finesStr << std::fixed << std::setprecision(2) << fines;
            limitStr << std::fixed << std::setprecision(2) << MAX_UNPAID_FINE_LIMIT;
            std::cout << "[Denied] User " << userId << " has unpaid fines ($" << finesStr.str()<< ") reaching the limit ($" << limitStr.str() << "). Borrowing blocked." << std::endl;
            return false;
        }
    }
    else {
        std::cout << "[Warning] User " << userId << " not found in DB (stub) - continuing." << std::endl;
    }
    const int LOAN_DAYS = 14;
    std::time_t dueTime = std::time(nullptr) + static_cast<std::time_t>(LOAN_DAYS) * 24 * 60 * 60;
    char dueBuf[11];
    std::strftime(dueBuf, sizeof(dueBuf), "%Y-%m-%d", std::localtime(&dueTime));
    std::string dueDate(dueBuf);
    asset->setState(AssetState::BORROWED);
    db->updateBorrowRecord(asset->getId(), userId, dueDate);
    std::cout << "[Success] User " << userId << " checked out '" << asset->getTitle()<< "' (due: " << dueDate << ")." << std::endl;
    return true;
}