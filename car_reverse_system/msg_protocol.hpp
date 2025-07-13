#pragma once

#include <cstdint>

#define FAR_LEN 20
#define MEDIUM_LEN 10


enum class DistanceLevel: uint8_t
{
    FAR = 0,
    MEDIUM = 1,
    CLOSE = 2
};

struct DistanceMessage
{
    int cm;
    DistanceLevel level;
    uint64_t timestamp_us;
};

class DistanceClassifier
{
public:
    static DistanceLevel classify(int cm)
    {
        if (cm > FAR_LEN) return DistanceLevel::FAR;
        if (cm > MEDIUM_LEN) return DistanceLevel::MEDIUM;
        return DistanceLevel::CLOSE;
    }
};
