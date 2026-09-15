#pragma once

#include <cstdint>

struct CpuSnapshot {
    uint64_t idle{0};
    uint64_t total{0};
};

class SystemMonitor {
public:
    SystemMonitor();

    double getCpuUsage();
    double getMemoryUsage();
    double getUptime();

private:
    CpuSnapshot m_prevCpu;
    CpuSnapshot readCpuRaw();
};