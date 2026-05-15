#include "OrderBook.hpp"
#include <iostream>
#include <thread>

int main() {
    OrderBook ob;

    // Add some orders
    ob.addOrder({100.0, 10, true}); // bid
    ob.addOrder({101.0, 5, true});  // bid
    ob.addOrder({100.0, 15, true}); // bid at same price
    ob.addOrder({99.0, 20, false}); // ask
    ob.addOrder({98.0, 25, false}); // ask

    // Print the order book
    ob.print();

    // Start snapshots every 1 second
    ob.startSnapshots(std::chrono::seconds(1));

    // Wait a bit
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Stop snapshots
    ob.stopSnapshots();

    // Get latest snapshot
    try {
        Snapshot snap = ob.getLatestSnapshot();
        std::cout << "Latest snapshot timestamp: " << std::chrono::duration_cast<std::chrono::seconds>(snap.timestamp.time_since_epoch()).count() << std::endl;
        std::cout << "Bid levels:" << std::endl;
        for (const auto& p : snap.bidLevels) {
            std::cout << "Price: " << p.first << " Qty: " << p.second << std::endl;
        }
        std::cout << "Ask levels:" << std::endl;
        for (const auto& p : snap.askLevels) {
            std::cout << "Price: " << p.first << " Qty: " << p.second << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "No snapshots available: " << e.what() << std::endl;
    }

    return 0;
}