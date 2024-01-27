#pragma once
extern "C" {
	#include "spdk/stdinc.h"

	#include "spdk/nvme.h"
	#include "spdk/vmd.h"
	#include "spdk/nvme_zns.h"
	#include "spdk/env.h"
	#include "spdk/string.h"
	#include "spdk/log.h"
	#include "spdk/env_dpdk.h"
}

#include <config.h>
#include <chrono>
#include <vector>
#include <mutex>
#define DATA_BUFFER_STRING "Hello world!"
// #define CHUNK_SIZE 0x2000000

static TAILQ_HEAD(, ctrlr_entry) g_controllers = TAILQ_HEAD_INITIALIZER(g_controllers);
static TAILQ_HEAD(, ns_entry) g_namespaces = TAILQ_HEAD_INITIALIZER(g_namespaces);
static struct spdk_nvme_transport_id g_trid = {};
static std::vector<struct hello_world_sequence *> spdk_sequence_queue;
static std::vector<struct hello_world_sequence *> spdk_sequence_queue2;
static bool g_vmd = false;
static float w_speed, r_speed;
static std::mutex locker;
struct hello_world_sequence {
	struct ns_entry	*ns_entry;
	char		*buf;
	unsigned        using_cmb_io;
	int		is_completed;
	void		*returnBuf;
	__u64	copy_size;
};

struct ctrlr_entry {
	struct spdk_nvme_ctrlr		*ctrlr;
	TAILQ_ENTRY(ctrlr_entry)	link;
	char				name[1024];
};

struct ns_entry {
	struct spdk_nvme_ctrlr	*ctrlr;
	struct spdk_nvme_ns	*ns;
	TAILQ_ENTRY(ns_entry)	link;
	struct spdk_nvme_qpair	*qpair;
};

static void register_ns(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *ns);
static void read_complete(void *arg, const struct spdk_nvme_cpl *completion);
static void write_complete(void *arg, const struct spdk_nvme_cpl *completion);
static void reset_zone_complete(void *arg, const struct spdk_nvme_cpl *completion);
static void reset_zone_and_wait_for_completion(struct hello_world_sequence *sequence);
static void hello_world(void *buffer, __u16 op, __u64 lba, __u64 size);
static bool probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid, struct spdk_nvme_ctrlr_opts *opts);
static void attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid, struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts);
static void cleanup(void);
int processor(void *buffer, __u16 op, __u64 lba, __u64 size);
void* mem_allocation(size_t size);
void mem_free(void* buffer);

#include <map>
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
    /**
     * @brief Reset the logger by clearing the start times and end times.
     */
    void reset() {
        startTimes.clear();
        endTimes.clear();
    }

    /**
     * @brief Start recording the duration of a test.
     * @param testName The name of the test.
     */
    void startTest(const std::string& testName) {
        startTimes[testName] = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief End recording the duration of a test.
     * @param testName The name of the test.
     */
    void endTest(const std::string& testName) {
        endTimes[testName] = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Show the logs of the recorded test durations and the properties of
     * the test configuration.
     * @param config The test configuration.
     */
    void showLogs() {

        std::cout << "Test,StartTime,EndTime,Duration(ms)" << std::endl;

        // Calculate total duration
        double totalDuration = 0.0;
        for (const auto& pair : startTimes) {
            const std::string& testName = pair.first;
            auto startTime = pair.second;
            auto endTime = endTimes[testName];

            double duration =
                std::chrono::duration_cast<std::chrono::microseconds>(endTime -
                                                                    startTime)
                    .count() /
                1000.0;
            totalDuration += duration;
        }

        // Print each test's duration and percentage
    for (const auto& pair : startTimes) {
        const std::string& testName = pair.first;
        auto startTime = pair.second;
        auto endTime = endTimes[testName];

        double duration =
            std::chrono::duration_cast<std::chrono::microseconds>(endTime -
                                                                  startTime)
                .count() /
            1000.0;
        double percentage = (duration / totalDuration) * 100;

        // print out Test, StartTime, EndTime, Duration(ms)
        printf("%s,%ld,%ld,%.5f\n", testName.c_str(), startTime.time_since_epoch().count(), endTime.time_since_epoch().count(), duration);

        // printf("%s,%.5f,%.2f\n", testName.c_str(), duration, percentage);
    }
}
};

// global logger, please carefully to use it in multi-thread environment.
