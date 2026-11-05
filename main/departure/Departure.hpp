#pragma once

#include <string>

struct Departure {
    int64_t plannedDepartureTime;
    int64_t realtimeDepartureTime;
    std::string label;
    std::string destination;
    bool cancelled;
    int delayInMinutes;
};
