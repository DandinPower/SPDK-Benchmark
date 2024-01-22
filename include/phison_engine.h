
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
#include <chrono>
#include <vector>
#include <mutex>
#define DATA_BUFFER_STRING "Hello world!"
#define CHUNK_SIZE 0x200000
// #define CHUNK_SIZE 0x2000000
#define ALIGN_SIZE 4096 // 4KB

static TAILQ_HEAD(, ctrlr_entry) g_controllers = TAILQ_HEAD_INITIALIZER(g_controllers);
static TAILQ_HEAD(, ns_entry) g_namespaces = TAILQ_HEAD_INITIALIZER(g_namespaces);
static struct spdk_nvme_transport_id g_trid = {};
static std::vector<struct hello_world_sequence *> spdk_sequence_queue;
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