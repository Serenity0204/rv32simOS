#pragma once
#include "ScopedCriticalSection.hpp"
#include <chrono>
#include <functional>
#include <queue>
#include <unordered_set>
#include <vector>

using AlarmCallback = std::function<void()>;

struct AlarmEvent
{
    std::chrono::steady_clock::time_point expiration;
    uint64_t id;

    bool operator>(const AlarmEvent& other) const
    {
        return this->expiration > other.expiration;
    }
};

struct AlarmInfo
{
    std::chrono::steady_clock::time_point expiration;
    AlarmCallback callback;
    bool active;
};

class Alarm
{
private:
    std::unordered_map<uint64_t, AlarmInfo> activeTimers;
    std::priority_queue<AlarmEvent, std::vector<AlarmEvent>, std::greater<AlarmEvent>> queue;
    uint64_t nextId = 0;

public:
    Alarm() = default;
    ~Alarm() = default;

    uint64_t registerTimer(int delayMs, AlarmCallback cb);
    void cancelTimer(uint64_t id);
    bool extendTimer(uint64_t id, int extraDelayMs);
    void tick();
};