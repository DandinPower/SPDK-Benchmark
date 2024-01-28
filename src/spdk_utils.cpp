#include <spdk_utils.h>

bool init;
TAILQ_HEAD(, ctrlr_entry) g_controllers = TAILQ_HEAD_INITIALIZER(g_controllers);
TAILQ_HEAD(, ns_entry) g_namespaces = TAILQ_HEAD_INITIALIZER(g_namespaces);
struct spdk_nvme_transport_id g_trid = {};
std::vector<struct hello_world_sequence *> spdk_sequence_queue;
bool g_vmd = false;
float w_speed, r_speed;
std::mutex locker;

void register_ns(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *ns) {
    struct ns_entry *entry;

    if (!spdk_nvme_ns_is_active(ns)) {
        return;
    }

    entry = (struct ns_entry *)malloc(sizeof(struct ns_entry));
    if (entry == NULL) {
        perror("ns_entry malloc");
        exit(1);
    }

    entry->ctrlr = ctrlr;
    entry->ns = ns;
    TAILQ_INSERT_TAIL(&g_namespaces, entry, link);

    printf("  Namespace ID: %d size: %juGB\n", spdk_nvme_ns_get_id(ns),
           spdk_nvme_ns_get_size(ns) / 1000000000);
}

bool probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
              struct spdk_nvme_ctrlr_opts *opts) {
    return true;
}

void attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
               struct spdk_nvme_ctrlr *ctrlr,
               const struct spdk_nvme_ctrlr_opts *opts) {
    int nsid;
    struct ctrlr_entry *entry;
    struct spdk_nvme_ns *ns;
    const struct spdk_nvme_ctrlr_data *cdata;

    entry = (struct ctrlr_entry *)malloc(sizeof(struct ctrlr_entry));
    if (entry == NULL) {
        perror("ctrlr_entry malloc");
        exit(1);
    }
    /*
     * spdk_nvme_ctrlr is the logical abstraction in SPDK for an NVMe
     *  controller.  During initialization, the IDENTIFY data for the
     *  controller is read using an NVMe admin command, and that data
     *  can be retrieved using spdk_nvme_ctrlr_get_data() to get
     *  detailed information on the controller.  Refer to the NVMe
     *  specification for more details on IDENTIFY for NVMe controllers.
     */
    cdata = spdk_nvme_ctrlr_get_data(ctrlr);

    snprintf(entry->name, sizeof(entry->name), "%-20.20s (%-20.20s)", cdata->mn,
             cdata->sn);

    entry->ctrlr = ctrlr;
    TAILQ_INSERT_TAIL(&g_controllers, entry, link);

    /*
     * Each controller has one or more namespaces.  An NVMe namespace is
     * basically equivalent to a SCSI LUN.  The controller's IDENTIFY data tells
     * us how many namespaces exist on the controller.  For Intel(R) P3X00
     * controllers, it will just be one namespace.
     *
     * Note that in NVMe, namespace IDs start at 1, not 0.
     */
    for (nsid = spdk_nvme_ctrlr_get_first_active_ns(ctrlr); nsid != 0;
         nsid = spdk_nvme_ctrlr_get_next_active_ns(ctrlr, nsid)) {
        ns = spdk_nvme_ctrlr_get_ns(ctrlr, nsid);
        if (ns == NULL) {
            continue;
        }
        register_ns(ctrlr, ns);
    }
}

void cleanup(void) {
    struct ns_entry *ns_entry, *tmp_ns_entry;
    struct ctrlr_entry *ctrlr_entry, *tmp_ctrlr_entry;
    struct spdk_nvme_detach_ctx *detach_ctx = NULL;

    TAILQ_FOREACH_SAFE(ns_entry, &g_namespaces, link, tmp_ns_entry) {
        TAILQ_REMOVE(&g_namespaces, ns_entry, link);
        free(ns_entry);
    }

    TAILQ_FOREACH_SAFE(ctrlr_entry, &g_controllers, link, tmp_ctrlr_entry) {
        TAILQ_REMOVE(&g_controllers, ctrlr_entry, link);
        spdk_nvme_detach_async(ctrlr_entry->ctrlr, &detach_ctx);
        free(ctrlr_entry);
    }

    if (detach_ctx) {
        spdk_nvme_detach_poll(detach_ctx);
    }

    spdk_env_fini();
}

void read_complete(void *arg, const struct spdk_nvme_cpl *completion) {
    struct hello_world_sequence *sequence = (struct hello_world_sequence *)arg;

    /* Assume the I/O was successful */

    /* See if an error occurred. If so, display information
     * about it, and set completion value so that I/O
     * caller is aware that an error occurred.
     */
    if (spdk_nvme_cpl_is_error(completion)) {
        spdk_nvme_qpair_print_completion(sequence->ns_entry->qpair,
                                         (struct spdk_nvme_cpl *)completion);
        fprintf(stderr, "I/O error status: %s\n",
                spdk_nvme_cpl_get_status_string(&completion->status));
        fprintf(stderr, "Read I/O failed, aborting run\n");
        sequence->is_completed = 2;
        exit(1);
    }

    sequence->is_completed = 1;
}

void write_complete(void *arg, const struct spdk_nvme_cpl *completion) {
    struct hello_world_sequence *sequence = (struct hello_world_sequence *)arg;
    struct ns_entry *ns_entry = sequence->ns_entry;
    int rc;

    /* See if an error occurred. If so, display information
     * about it, and set completion value so that I/O
     * caller is aware that an error occurred.
     */
    if (spdk_nvme_cpl_is_error(completion)) {
        spdk_nvme_qpair_print_completion(sequence->ns_entry->qpair,
                                         (struct spdk_nvme_cpl *)completion);
        fprintf(stderr, "I/O error status: %s\n",
                spdk_nvme_cpl_get_status_string(&completion->status));
        fprintf(stderr, "Write I/O failed, aborting run\n");
        sequence->is_completed = 2;
        exit(1);
    }
    /*
     * The write I/O has completed.  Free the buffer associated with
     *  the write I/O and allocate a new zeroed buffer for reading
     *  the data back from the NVMe namespace.
     */

    sequence->is_completed = 1;
}

void reset_zone_complete(void *arg, const struct spdk_nvme_cpl *completion) {
    struct hello_world_sequence *sequence = (struct hello_world_sequence *)arg;

    /* Assume the I/O was successful */
    sequence->is_completed = 1;
    /* See if an error occurred. If so, display information
     * about it, and set completion value so that I/O
     * caller is aware that an error occurred.
     */
    if (spdk_nvme_cpl_is_error(completion)) {
        spdk_nvme_qpair_print_completion(sequence->ns_entry->qpair,
                                         (struct spdk_nvme_cpl *)completion);
        fprintf(stderr, "I/O error status: %s\n",
                spdk_nvme_cpl_get_status_string(&completion->status));
        fprintf(stderr, "Reset zone I/O failed, aborting run\n");
        sequence->is_completed = 2;
        exit(1);
    }
}

void reset_zone_and_wait_for_completion(struct hello_world_sequence *sequence) {
    if (spdk_nvme_zns_reset_zone(sequence->ns_entry->ns,
                                 sequence->ns_entry->qpair,
                                 0,     /* starting LBA of the zone to reset */
                                 false, /* don't reset all zones */
                                 reset_zone_complete, sequence)) {
        fprintf(stderr, "starting reset zone I/O failed\n");
        exit(1);
    }
    while (!sequence->is_completed) {
        spdk_nvme_qpair_process_completions(sequence->ns_entry->qpair, 0);
    }
    sequence->is_completed = 0;
}

void allocate_io_qpair(struct ns_entry *ns_entry) {
    struct spdk_nvme_io_qpair_opts opts;
    spdk_nvme_ctrlr_get_default_io_qpair_opts(ns_entry->ctrlr, &opts,
                                              sizeof(opts));
    // QUEUE DEPTH Modification
    // opts.io_queue_size = 128;
    // opts.io_queue_requests = 256;

    /*
     * Allocate an I/O qpair that we can use to submit read/write requests
     *  to namespaces on the controller.  NVMe controllers typically support
     *  many qpairs per controller.  Any I/O qpair allocated for a controller
     *  can submit I/O to any namespace on that controller.
     *
     * The SPDK NVMe driver provides no synchronization for qpair accesses -
     *  the application must ensure only a single thread submits I/O to a
     *  qpair, and that same thread must also check for completions on that
     *  qpair.  This enables extremely efficient I/O processing by making all
     *  I/O operations completely lockless.
     */
    ns_entry->qpair = spdk_nvme_ctrlr_alloc_io_qpair(ns_entry->ctrlr, &opts, 0);
    if (ns_entry->qpair == NULL) {
        printf("ERROR: spdk_nvme_ctrlr_alloc_io_qpair() failed\n");
        return;
    }
}

int spdk_init(void) {
    int rc = 0;

    /*
     * SPDK relies on an abstraction around the local environment
     * named env that handles memory allocation and PCI device operations.
     * This library must be initialized first.
     *
     */
    if (init == false) {
        struct spdk_env_opts opts;
        spdk_env_opts_init(&opts);
        opts.name = "hello_world";
        if (spdk_env_init(&opts) < 0) {
            fprintf(stderr, "Unable to initialize SPDK env\n");
            return 1;
        }
    }
    /*
     * Start the SPDK NVMe enumeration process.  probe_cb will be called
     *  for each NVMe controller found, giving our application a choice on
     *  whether to attach to each controller.  attach_cb will then be
     *  called for each controller after the SPDK NVMe driver has completed
     *  initializing the controller we chose to attach.
     */
    if (init == false) {
        spdk_nvme_transport_id_parse(&g_trid, "trtype:PCIe traddr:73:00.0");
        int rc = spdk_nvme_probe(&g_trid, NULL, probe_cb, attach_cb, NULL);
        if (rc != 0) {
            fprintf(stderr, "spdk_nvme_probe() failed\n");
            rc = 1;
            goto exit;
        }

        if (TAILQ_EMPTY(&g_controllers)) {
            fprintf(stderr, "no NVMe controllers found\n");
            rc = 1;
            goto exit;
        }
    }
    init = true;

exit:
    return rc;
}

void *spdk_mem_allocation(size_t size) {
    int ret = spdk_init();

    if (ret != 0) {
        fprintf(stderr, "spdk_init() failed\n");
        return NULL;
    }
    return spdk_zmalloc(size, ALIGN_SIZE, NULL, SPDK_ENV_SOCKET_ID_ANY,
                        SPDK_MALLOC_DMA);
}

void spdk_mem_free(void *buffer) { spdk_free(buffer); }

void _io_helper(
    void *buffer, __u16 op, __u64 remain_size, __u64 lba_offset,
    __u64 byte_offset,
    std::vector<struct hello_world_sequence *> &spdk_sequence_queue_local) {
    struct ns_entry *ns_entry;
    int rc;
    size_t sz;
    __u64 lbas = CHUNK_SIZE >> 9;
    spdk_sequence_queue_local.clear();

    HighPrecisionLogger logger;

    logger.reset();

    TAILQ_FOREACH(ns_entry, &g_namespaces, link) {
        allocate_io_qpair(ns_entry);
        int ret = 0;
        __u16 submit_task = 0;
        __u16 complete_task = 0;
        // char *charBuffer = reinterpret_cast<char *>(buffer);

        while (remain_size > 0) {
            logger.startTest("task:" + std::to_string(submit_task) +
                             " doing I/O");

            struct hello_world_sequence *sequence =
                (struct hello_world_sequence *)malloc(
                    sizeof(struct hello_world_sequence));
            spdk_sequence_queue_local.push_back(sequence);
            if (remain_size > CHUNK_SIZE) {
                lbas = CHUNK_SIZE >> 9;
                remain_size -= CHUNK_SIZE;
                spdk_sequence_queue_local[submit_task]->copy_size = CHUNK_SIZE;
            } else {
                lbas = remain_size >> 9;
                spdk_sequence_queue_local[submit_task]->copy_size = remain_size;
                remain_size = 0;
            }

            spdk_sequence_queue_local[submit_task]->is_completed = 0;
            spdk_sequence_queue_local[submit_task]->ns_entry = ns_entry;
#if MEM_CPY_MODE == 0
            spdk_sequence_queue_local[submit_task]->buf =
                (char *)buffer + byte_offset;
#else
            spdk_sequence_queue_local[submit_task]->buf =
                (char *)spdk_mem_allocation(
                    spdk_sequence_queue_local[submit_task]->copy_size);
            spdk_sequence_queue_local[submit_task]->returnBuf =
                (char *)buffer + byte_offset;
#endif

            if (op == 1) {
#if MEM_CPY_MODE == 1
                memcpy(spdk_sequence_queue_local[submit_task]->buf,
                       spdk_sequence_queue_local[submit_task]->returnBuf,
                       spdk_sequence_queue_local[submit_task]->copy_size);
#endif
                rc = spdk_nvme_ns_cmd_write(
                    ns_entry->ns, ns_entry->qpair,
                    spdk_sequence_queue_local[submit_task]->buf,
                    lba_offset, /* LBA start */
                    lbas,       /* number of LBAs */
                    write_complete, spdk_sequence_queue_local[submit_task], 0);
            } else if (op == 0) {
                rc = spdk_nvme_ns_cmd_read(
                    ns_entry->ns, ns_entry->qpair,
                    spdk_sequence_queue_local[submit_task]->buf,
                    lba_offset, /* LBA start */
                    lbas,       /* number of LBAs */
                    read_complete, spdk_sequence_queue_local[submit_task], 0);
            }
            if (rc != 0) {
                fprintf(stderr, "starting I/O failed\n");
                exit(1);
            }

            lba_offset += lbas;
            byte_offset += spdk_sequence_queue_local[submit_task]->copy_size;
            submit_task++;
        }

        for (auto i = 0; i < submit_task; i++) {
            logger.startTest("task:" + std::to_string(i) + " waiting I/O");
            while (!spdk_sequence_queue_local[i]->is_completed) {
                ret = spdk_nvme_qpair_process_completions(ns_entry->qpair, 0);
                if (ret < 0) {
                    fprintf(stderr, "proc I/O failed\n");
                } else {
                    complete_task += ret;
                }
            }
            logger.endTest("task:" + std::to_string(i) + " waiting I/O");

#if MEM_CPY_MODE == 1
            if (op == 0)
                memcpy(spdk_sequence_queue_local[i]->returnBuf,
                       spdk_sequence_queue_local[i]->buf,
                       spdk_sequence_queue_local[i]->copy_size);
            spdk_mem_free(spdk_sequence_queue_local[i]->buf);
#endif
            free(spdk_sequence_queue_local[i]);
            logger.endTest("task:" + std::to_string(i) + " doing I/O");
        }
        spdk_nvme_ctrlr_free_io_qpair(ns_entry->qpair);
    }

    logger.showLogs();
}

void hello_world(void *buffer, __u16 op, __u64 lba, __u64 size) {
    // __u64 remain_size = size / 2;
    // __u64 lba_offset = lba;
    // __u64 byte_offset = 0;

    // std::thread t1(_io_helper, buffer, op, remain_size, lba_offset,
    // byte_offset,
    //                std::ref(spdk_sequence_queue));

    // remain_size = size / 2;
    // lba_offset = lba + ((size / 2) >> 9);
    // byte_offset = size / 2;
    // std::thread t2(_io_helper, buffer, op, remain_size, lba_offset,
    // byte_offset,
    //                std::ref(spdk_sequence_queue2));

    // t1.join();
    // t2.join();

    __u64 remain_size = size;
    __u64 lba_offset = lba;
    __u64 byte_offset = 0;
    _io_helper(buffer, op, remain_size, lba_offset, byte_offset,
               std::ref(spdk_sequence_queue));
}

int processor(void *buffer, __u16 op, __u64 lba, __u64 size) {
    int ret = spdk_init();
    if (ret != 0) {
        fprintf(stderr, "spdk_init() failed\n");
        return 1;
    }

    locker.lock();
    hello_world(buffer, op, lba, size);
    locker.unlock();
    return 0;
}