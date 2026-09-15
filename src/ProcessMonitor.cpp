#include "ProcessMonitor.hpp"

#include <fstream>
#include <sstream>
#include <string>

SystemMonitor::SystemMonitor() {
    m_prevCpu = readCpuRaw();
}

CpuSnapshot SystemMonitor::readCpuRaw() {
    std::ifstream file("/proc/stat");
    std::string line;
    CpuSnapshot snap;

    if (file.is_open() && std::getline(file, line)) {
        std::istringstream iss(line);
        std::string cpuLabel;
        uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;

        iss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        snap.idle = idle + iowait;
        snap.total = user + nice + system + idle + iowait + irq + softirq + steal;
    }
    return snap;
}

double SystemMonitor::getCpuUsage() {
    CpuSnapshot current = readCpuRaw();

    uint64_t totalDelta = current.total - m_prevCpu.total;
    uint64_t idleDelta = current.idle - m_prevCpu.idle;

    m_prevCpu = current;

    if (totalDelta == 0) return 0.0;
    return (1.0 - (static_cast<double>(idleDelta) / totalDelta)) * 100.0;
}

double SystemMonitor::getMemoryUsage() {
    std::ifstream file("/proc/meminfo");
    std::string key;
    uint64_t value = 0;
    std::string unit;

    uint64_t totalMem = 0;
    uint64_t freeMem = 0;
    uint64_t buffers = 0;
    uint64_t cached = 0;

    while (file >> key >> value >> unit) {
        if (key == "MemTotal:")      totalMem = value;
        else if (key == "MemFree:")  freeMem = value;
        else if (key == "Buffers:")  buffers = value;
        else if (key == "Cached:")   cached = value;

        if (totalMem && freeMem && buffers && cached) break;
    }

    if (totalMem == 0) return 0.0;

    // Actual memory used excludes buffers and cache
    uint64_t usedMem = totalMem - (freeMem + buffers + cached);
    return (static_cast<double>(usedMem) / totalMem) * 100.0;
}

double SystemMonitor::getUptime() {
    std::ifstream file("/proc/uptime");
    double uptime = 0.0;
    if (file.is_open()) {
        file >> uptime;
    }
    return uptime;
}