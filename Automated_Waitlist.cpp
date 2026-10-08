#include <iostream>
#include <map>
#include <queue>

bool LibraryManager::resolveWaitlistQueue(Asset* asset) {
    if (asset == nullptr) return false;
    std::cout << "[System] Checking Waitlist queue for Asset ID: " << asset->getId() << std::endl;
    auto it = waitlistMock.find(asset->getId());
    bool hasWaitlistUser = (it != waitlistMock.end() && !it->second.empty());
    if (hasWaitlistUser) {
        int nextWaitingUserId = it->second.front();
        it->second.pop();
        if (it->second.empty()) waitlistMock.erase(it);
        asset->setState(AssetState::WAITLISTED);
        std::cout << "[Automated Queue] Asset instantly reserved for next waitlisted User: "<< nextWaitingUserId << std::endl;
        return true;
    }
    asset->setState(AssetState::AVAILABLE);
    std::cout << "[System] No waitlist entries found. Asset '" << asset->getTitle()<< "' is now AVAILABLE in circulation." << std::endl;
    return false;
}
bool LibraryManager::addUserToWaitlist(int userId, int assetId) {
    if (userId <= 0 || assetId <= 0) {
        std::cout << "[Error] Invalid user or asset ID for waitlist." << std::endl;
        return false;
    }
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