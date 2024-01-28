#pragma once

#include <chrono>
#include <map>
#include <string>
#include <iostream>

/**
 * @class HighPrecisionLogger
 * @brief A class for logging high precision test durations.
 */
class HighPrecisionLogger {
   private:
    std::map<std::string, std::chrono::high_resolution_clock::time_point>
        startTimes; /**< Map to store start times of tests */
    std::map<std::string, std::chrono::high_resolution_clock::time_point>
        endTimes; /**< Map to store end times of tests */

   public:
    void reset();
    void startTest(const std::string& testName);
    void endTest(const std::string& testName);
    void showLogs();
};