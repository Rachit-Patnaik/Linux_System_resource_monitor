#include "ProcessMonitor.hpp"

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

int main() {
    SystemMonitor monitor;

    // Wait 500ms to calibrate initial CPU snapshot
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    const int total_samples = 5;

    for (int i = 0; i < total_samples; ++i) {
        double cpu = monitor.getCpuUsage();
        double mem = monitor.getMemoryUsage();
        double uptime = monitor.getUptime();

        std::cout << "\n--- System Resource Monitor ---\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "CPU Usage:     " << cpu << "%\n";
        std::cout << "Memory Usage:  " << mem << "%\n";
        std::cout << "System Uptime: " << uptime << " seconds\n";

        // Sleep between readings (except after the final reading)
        if (i < total_samples - 1) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    std::cout << "\nResource monitoring finished.\n";
    return 0;
}