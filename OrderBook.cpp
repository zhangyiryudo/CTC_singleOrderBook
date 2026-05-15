#include "OrderBook.hpp"

OrderBook::OrderBook() : orderCount_(0), running_(false) {}

OrderBook::~OrderBook() {
    stopSnapshots();
}

void OrderBook::addOrder(const Order& order) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (order.isBid) {
        bids_[order.price].push_back(order);
    } else {
        asks_[order.price].push_back(order);
    }
    orderCount_.fetch_add(1, std::memory_order_relaxed);
}

void OrderBook::print() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "Bids (sorted descending):" << std::endl;
    for (const auto& level : bids_) {
        int totalQty = 0;
        for (const auto& order : level.second) {
            totalQty += order.qty;
        }
        std::cout << "Price: " << level.first << " Qty: " << totalQty << std::endl;
    }
    std::cout << "Asks (sorted ascending):" << std::endl;
    for (const auto& level : asks_) {
        int totalQty = 0;
        for (const auto& order : level.second) {
            totalQty += order.qty;
        }
        std::cout << "Price: " << level.first << " Qty: " << totalQty << std::endl;
    }
}

void OrderBook::takeSnapshot() {
    std::lock_guard<std::mutex> lock(mutex_);
    Snapshot snap;
    snap.timestamp = std::chrono::steady_clock::now();
    for (const auto& level : bids_) {
        int total = 0;
        for (const auto& o : level.second) total += o.qty;
        snap.bidLevels[level.first] = total;
    }
    for (const auto& level : asks_) {
        int total = 0;
        for (const auto& o : level.second) total += o.qty;
        snap.askLevels[level.first] = total;
    }
    snapshots_.push(snap);
}

void OrderBook::startSnapshots(std::chrono::milliseconds interval) {
    if (running_.load(std::memory_order_acquire)) return;
    running_.store(true, std::memory_order_release);
    snapshotThread_ = std::thread([this, interval]() {
        while (running_.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(interval);
            takeSnapshot();
        }
    });
}

void OrderBook::stopSnapshots() {
    running_.store(false, std::memory_order_release);
    if (snapshotThread_.joinable()) {
        snapshotThread_.join();
    }
}

Snapshot OrderBook::getLatestSnapshot() const {
    return snapshots_.getLatest();
}