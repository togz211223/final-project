#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

void LibraryManager::returnAsset(int userId, Asset* asset, int daysOverdue) {
    if (asset == nullptr) {
        std::cout << "[Error] Cannot process return for null asset." << std::endl;
        return;
    }
    if (asset->getState() != AssetState::BORROWED) {
        std::cout << "[Error] Asset '" << asset->getTitle() << "' is not currently borrowed. Return ignored." << std::endl;
        return;
    }
    std::cout << "\n[System] Processing return for asset: " << asset->getTitle()<< " from User: " << userId << std::endl;
    if (daysOverdue > 0) {
        double fineAmount = asset->calculateLateFine(daysOverdue);
        std::ostringstream fineStr;
        fineStr << std::fixed << std::setprecision(2) << fineAmount;
        std::cout << "[Fine Alert] Asset returned " << daysOverdue << " days late. Calculated fine: $"<< fineStr.str() << std::endl;
        if (fineAmount > 0.0) {
            std::cout << "[DB Update] Added $" << fineStr.str()<< " to unpaid fines for User " << userId << std::endl;
        }
    }
    else {
        std::cout << "[Success] Item returned on time. No late fee assessed." << std::endl;
    }
    std::cout << "[DB Update] Borrow record closed for Asset ID: " << asset->getId() << std::endl;
    resolveWaitlistQueue(asset);
}