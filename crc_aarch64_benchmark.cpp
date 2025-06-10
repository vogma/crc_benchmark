#include <benchmark/benchmark.h>
#include "crc.h"

#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <vector>

static void pin_to_core(int core = 0)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
}

// --- utility: simple cache flush by streaming a buffer > L3 into memory
static void flush_caches()
{
    static const size_t FLUSH_SIZE = 12 * 1024 * 1024; // 12 MB
    static std::vector<uint8_t> buf(FLUSH_SIZE, 1);
    volatile uint8_t sink = 0;
    for (size_t i = 0; i < FLUSH_SIZE; ++i)
        sink ^= buf[i];
}

// --- perf_event_open helper
static int
open_perf(uint32_t type, uint64_t config)
{
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.type = type;
    attr.size = sizeof(attr);
    attr.config = config;
    attr.disabled = 1;
    attr.exclude_kernel = 1;
    attr.exclude_hv = 1;
    return syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
}

struct CrcFixture : benchmark::Fixture
{
    int fd_insn = -1, fd_cycles = -1;

    void SetUp(benchmark::State &state) override
    {
        pin_to_core(0);
        flush_caches();
        fd_insn = open_perf(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
        fd_cycles = open_perf(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
    }

    void TearDown(benchmark::State &state) override
    {
        ioctl(fd_insn, PERF_EVENT_IOC_DISABLE, 0);
        ioctl(fd_cycles, PERF_EVENT_IOC_DISABLE, 0);
        uint64_t insn, cycles;

        ssize_t read_result = read(fd_insn, &insn, sizeof(insn));
        ssize_t read_result2 = read(fd_cycles, &cycles, sizeof(cycles));

        if (read_result < 1 || read_result2 < 1)
        {
            perror("read failed");
            exit(1);
        }

        // ./autobench -i avx512_vpclmulqdq -p crc32c -a v3s4 100GB/s

        double ipc = double(insn) / double(cycles);
        double cpi = double(cycles) / double(state.bytes_processed());
        double cpi2 = double(insn) / double(cycles);

        // state.counters["IPC"] = ipc;
        state.counters["CyclesPerByte"] = cpi;
        state.counters["BytesPerCycles"] = 1 / cpi;
        // state.counters["InsnPerCycle"] = cpi2; Data not plausible

        close(fd_insn);
        close(fd_cycles);
    }
};

// single-size benchmark; byte‐throughput reported automatically
BENCHMARK_DEFINE_F(CrcFixture, crc32_chrome_scalar)(benchmark::State &state)
{
    const size_t size = state.range(0);
    std::vector<uint8_t> data(size, 0xA5);

    // reset + enable counters
    ioctl(this->fd_insn, PERF_EVENT_IOC_RESET, 0);
    ioctl(this->fd_cycles, PERF_EVENT_IOC_RESET, 0);
    ioctl(this->fd_insn, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(this->fd_cycles, PERF_EVENT_IOC_ENABLE, 0);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(crc32_chrome_scalar(0x00000000u, data.data(), size));
    }

    // report throughput in bytes
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

// single-size benchmark; byte‐throughput reported automatically
BENCHMARK_DEFINE_F(CrcFixture, armv8_crc32_pmull_little)(benchmark::State &state)
{
    const size_t size = state.range(0);
    std::vector<uint8_t> data(size, 0xA5);

    // reset + enable counters
    ioctl(this->fd_insn, PERF_EVENT_IOC_RESET, 0);
    ioctl(this->fd_cycles, PERF_EVENT_IOC_RESET, 0);
    ioctl(this->fd_insn, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(this->fd_cycles, PERF_EVENT_IOC_ENABLE, 0);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(armv8_crc32_pmull_little(0x00000000u, data.data(), size));
    }

    // report throughput in bytes
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

// single-size benchmark; byte‐throughput reported automatically
BENCHMARK_DEFINE_F(CrcFixture, armv8_crc32_cloudfare_little)(benchmark::State &state)
{
    const size_t size = state.range(0);
    std::vector<uint8_t> data(size, 0xA5);

    // reset + enable counters
    ioctl(this->fd_insn, PERF_EVENT_IOC_RESET, 0);
    ioctl(this->fd_cycles, PERF_EVENT_IOC_RESET, 0);
    ioctl(this->fd_insn, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(this->fd_cycles, PERF_EVENT_IOC_ENABLE, 0);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(armv8_crc32_cloudfare_little(0x00000000u, data.data(), size));
    }

    // report throughput in bytes
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

// single-size benchmark; byte‐throughput reported automatically
BENCHMARK_DEFINE_F(CrcFixture, crc32_update_no_xor)(benchmark::State &state)
{
    const size_t size = state.range(0);
    std::vector<uint8_t>    data(size, 0xA5);

    // reset + enable counters
    ioctl(this->fd_insn, PERF_EVENT_IOC_RESET, 0);
    ioctl(this->fd_cycles, PERF_EVENT_IOC_RESET, 0);
    ioctl(this->fd_insn, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(this->fd_cycles, PERF_EVENT_IOC_ENABLE, 0);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(crc32_update_no_xor(0x00000000u, data.data(), size));
    }

    // report throughput in bytes
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

static void CustomArgs(benchmark::internal::Benchmark *b)
{
    for (auto s : {size_t(256), size_t(1024), size_t(2 * 1024), size_t(4 * 1024), size_t(8 * 1024), size_t(10 * 1024 * 1024), size_t(16 * 1024 * 1024), size_t(32 * 1024 * 1024), size_t(100 * 1024 * 1024)})
        b->Arg(s);
}

BENCHMARK_REGISTER_F(CrcFixture, crc32_chrome_scalar)
    ->Apply(CustomArgs)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
