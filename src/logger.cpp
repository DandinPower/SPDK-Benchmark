#include <logger.h>

/**
 * @brief Reset the logger by clearing the start times and end times.
 */
void HighPrecisionLogger::reset() {
    startTimes.clear();
    endTimes.clear();
}

/**
 * @brief Start recording the duration of a test.
 * @param testName The name of the test.
 */
void HighPrecisionLogger::startTest(const std::string& testName) {
    startTimes[testName] = std::chrono::high_resolution_clock::now();
}

/**
 * @brief End recording the duration of a test.
 * @param testName The name of the test.
 */
void HighPrecisionLogger::endTest(const std::string& testName) {
    endTimes[testName] = std::chrono::high_resolution_clock::now();
}

/**
 * @brief Show the logs of the recorded test durations and the properties of
 * the test configuration.
 * @param config The test configuration.
 */
void HighPrecisionLogger::showLogs() {
    std::cout << "Test,StartTime,EndTime,Duration(ms)" << std::endl;

    // Calculate total duration
    double totalDuration = 0.0;
    for (const auto& pair : startTimes) {
        const std::string& testName = pair.first;
        auto startTime = pair.second;
        auto endTime = endTimes[testName];

        double duration = std::chrono::duration_cast<std::chrono::microseconds>(
                              endTime - startTime)
                              .count() /
                          1000.0;
        totalDuration += duration;
    }

    // Print each test's duration and percentage
    for (const auto& pair : startTimes) {
        const std::string& testName = pair.first;
        auto startTime = pair.second;
        auto endTime = endTimes[testName];

        double duration = std::chrono::duration_cast<std::chrono::microseconds>(
                              endTime - startTime)
                              .count() /
                          1000.0;
        double percentage = (duration / totalDuration) * 100;

        // print out Test, StartTime, EndTime, Duration(ms)
        printf("%s,%ld,%ld,%.5f\n", testName.c_str(),
               startTime.time_since_epoch().count(),
               endTime.time_since_epoch().count(), duration);
    }
}