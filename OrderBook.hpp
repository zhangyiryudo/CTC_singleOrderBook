#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <map>
#include <list>
#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <array>
#include <iostream>

struct Order {
    double price;
    int qty;
    bool isBid; // true for bid, false for ask
};

struct Snapshot {
    std::chrono::steady_clock::time_point timestamp;
    std::map<double, int> bidLevels; // price to aggregated qty
    std::map<double, int> askLevels; // price to aggregated qty
};

template<typename T, size_t N>
class RingBuffer {
private:
    std::array<T, N> buffer_;
    size_t head_ = 0;
    size_t size_ = 0;
    mutable std::mutex mutex_;

public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        buffer_[head_] = item;
        head_ = (head_ + 1) % N;
        if (size_ < N) size_++;
    }

    // Retrieve the latest snapshot
    T getLatest() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (size_ == 0) throw std::runtime_error("Ring buffer is empty");
        size_t index = (head_ + N - 1) % N;
        return buffer_[index];
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return size_;
    }
};

class OrderBook {
private:
    std::map<double, std::list<Order>, std::greater<double>> bids_; // descending order for bids
    std::map<double, std::list<Order>> asks_; // ascending order for asks
    mutable std::mutex mutex_;
    std::atomic<size_t> orderCount_;
    RingBuffer<Snapshot, 100> snapshots_; // ring buffer for snapshots
    std::thread snapshotThread_;
    std::atomic<bool> running_;

    void takeSnapshot();

public:
    OrderBook();
    ~OrderBook();

    void addOrder(const Order& order);
    void print() const;
    void startSnapshots(std::chrono::milliseconds interval);
    void stopSnapshots();
    Snapshot getLatestSnapshot() const;
};

#endif // ORDER_BOOK_HPP