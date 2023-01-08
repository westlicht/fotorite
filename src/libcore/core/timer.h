#pragma once

#include "defs.h"

#include <chrono>

FR_NAMESPACE_BEGIN

class Timer {
public:
    Timer() { reset(); }

    void reset() { _start_time = std::chrono::high_resolution_clock::now(); }

    double elapsed() {
        std::chrono::time_point<std::chrono::high_resolution_clock> cur_time =
            std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = cur_time - _start_time;
        return duration.count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
};

FR_NAMESPACE_END
