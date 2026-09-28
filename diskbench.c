#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>

#define FILESIZE   (4ULL * 1024 * 1024 * 1024)
#define TESTS      2000

typedef struct {
    int fd;
    size_t bs;
    int ops;
    int threads;
    unsigned int seed;
    double seconds;
} worker_arg;

static double now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static void *worker(void *p) {
    worker_arg *a = (worker_arg *)p;
    char *buf = aligned_alloc(4096, a->bs);
    memset(buf, 0xA5, a->bs);

    uint64_t blocks = FILESIZE / a->bs;

    double t = now();

    for (int i = 0; i < a->ops; i++) {
        uint64_t block = ((uint64_t)rand_r(&a->seed) << 32 |
                          rand_r(&a->seed)) % blocks;
        off_t off = (off_t)(block * a->bs);

        ssize_t r = pread(a->fd, buf, a->bs, off);
        if (r != (ssize_t)a->bs) {
            perror("pread");
            break;
        }
    }

    a->seconds = now() - t;
    free(buf);
    return NULL;
}

static double random_read(size_t bs, int threads, int total_ops) {
    pthread_t *th = calloc(threads, sizeof(pthread_t));
    worker_arg *args = calloc(threads, sizeof(worker_arg));

    int per = total_ops / threads;

    double start = now();

    for (int i = 0; i < threads; i++) {
        args[i].fd = -1;
        args[i].bs = bs;
        args[i].ops = per;
        args[i].threads = threads;
        args[i].seed = (unsigned int)(0x12345678 + i * 7919);
    }

    /*
     * Open once, shared by all threads.
     */
    int fd = open("/tmp/diskbench.dat", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    for (int i = 0; i < threads; i++) {
        args[i].fd = fd;
        pthread_create(&th[i], NULL, worker, &args[i]);
    }

    for (int i = 0; i < threads; i++)
        pthread_join(th[i], NULL);

    double elapsed = now() - start;

    close(fd);
    free(th);
    free(args);

    return (double)total_ops / elapsed;
}

static void seq_test(void) {
    int fd = open("/tmp/diskbench.dat", O_CREAT | O_TRUNC | O_RDWR, 0600);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    size_t bs = 1024 * 1024;
    char *buf = malloc(bs);
    memset(buf, 0, bs);

    uint64_t blocks = FILESIZE / bs;

    /* Sequential write */
    double t = now();

    for (uint64_t i = 0; i < blocks; i++) {
        if (write(fd, buf, bs) != (ssize_t)bs) {
            perror("write");
            exit(1);
        }
    }

    fsync(fd);

    double write_sec = now() - t;

    /* Evict filesystem cache as much as possible */
    close(fd);
    system("sync");
    system("purge >/dev/null 2>&1");

    fd = open("/tmp/diskbench.dat", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    /* Sequential read */
    t = now();

    uint64_t total = 0;
    while (total < FILESIZE) {
        ssize_t n = read(fd, buf, bs);
        if (n <= 0) break;
        total += n;
    }

    double read_sec = now() - t;

    close(fd);
    free(buf);

    double write_mbs = FILESIZE / write_sec / 1000000.0;
    double read_mbs  = FILESIZE / read_sec  / 1000000.0;

    printf("Sequential Read       %10.1f MB/s\n", read_mbs);
    printf("Sequential Write      %10.1f MB/s\n", write_mbs);
}

int main(void) {
    printf("\n");
    printf("============================================\n");
    printf("           macOS Disk Benchmark\n");
    printf("============================================\n");
    printf("Test file : /tmp/diskbench.dat\n");
    printf("Size      : %.1f GB\n\n", FILESIZE / 1e9);

    seq_test();

    printf("\n");

    /* 4K QD1 */
    system("sync");
    system("purge >/dev/null 2>&1");

    double iops4k = random_read(4096, 1, TESTS);
    double mb4k = iops4k * 4096.0 / 1000000.0;

    printf("Random Read 4K QD1     %10.1f MB/s  (%8.0f IOPS)\n",
           mb4k, iops4k);

    /* 32K QD20 */
    system("sync");
    system("purge >/dev/null 2>&1");

    double iops32k = random_read(32768, 20, TESTS * 5);
    double mb32k = iops32k * 32768.0 / 1000000.0;

    printf("Random Read 32K QD20   %10.1f MB/s  (%8.0f IOPS)\n",
           mb32k, iops32k);

    printf("============================================\n");

    return 0;
}
