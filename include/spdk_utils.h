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
#include <logger.h>
#include <unistd.h>
#include <vector>
#include <mutex>
#include <chrono>
#include <iostream>
#include <thread>

struct hello_world_sequence {
	struct ns_entry	*ns_entry;
	char		*buf;
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

void register_ns(struct spdk_nvme_ctrlr *ctrlr,
                        struct spdk_nvme_ns *ns);

bool probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
                     struct spdk_nvme_ctrlr_opts *opts);

void attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
                      struct spdk_nvme_ctrlr *ctrlr,
                      const struct spdk_nvme_ctrlr_opts *opts);

void cleanup(void);
void read_complete(void *arg, const struct spdk_nvme_cpl *completion);
void write_complete(void *arg, const struct spdk_nvme_cpl *completion);
void reset_zone_complete(void *arg, const struct spdk_nvme_cpl *completion);
void allocate_io_qpair(struct ns_entry *ns_entry);
int spdk_init(void);
void *spdk_mem_allocation(size_t size);
void spdk_mem_free(void *buffer);

void _io_helper(
    void *buffer, __u16 op, __u64 remain_size, __u64 lba_offset,
    __u64 byte_offset,
    std::vector<struct hello_world_sequence *> &spdk_sequence_queue_local);
void hello_world(void *buffer, __u16 op, __u64 lba, __u64 size);
int processor(void *buffer, __u16 op, __u64 lba, __u64 size);