#include "FpgaConfig.hpp"

FpgaConfig::FpgaConfig() {}

FpgaConfig::~FpgaConfig() {}

constexpr size_t RW_MAX_SIZE = 0x7ffff000;
const char *C2H[4] = {"/dev/xdma0_c2h_0","/dev/xdma0_c2h_1","/dev/xdma0_c2h_2","/dev/xdma0_c2h_3"};
const char *H2C[4] = {"/dev/xdma0_h2c_0","/dev/xdma0_h2c_1","/dev/xdma0_h2c_2","/dev/xdma0_h2c_3"};

ssize_t dmaRead(int fd, char *buffer, uint64_t size, uint64_t base) {
    ssize_t rc;
    uint64_t count = 0;
    char *buf = buffer;
    off_t offset = base;

    while (count < size) {
        uint64_t bytes = size - count;
        if (bytes > RW_MAX_SIZE) {
            bytes = RW_MAX_SIZE;
        }
        if (offset) {
            rc = lseek(fd, offset, SEEK_SET);
            assert(rc == offset);
        }
        /* read data from file into memory buffer */
        rc = read(fd, buf, bytes);
        assert(rc >= 0);
        count += rc;
        buf += rc;
        offset += rc;
    }

    return count;
}

ssize_t dmaWrite(int fd, char *buffer, uint64_t size, uint64_t base) {
    ssize_t rc;
    uint64_t count = 0;
    char *buf = buffer;
    off_t offset = base;

    while (count < size) {
        uint64_t bytes = size - count;
        if (bytes > RW_MAX_SIZE) {
            bytes = RW_MAX_SIZE;
        }
        if (offset) {
            rc = lseek(fd, offset, SEEK_SET);
            assert(rc == offset);
        }
        /* write data to file from memory buffer */
        rc = write(fd, buf, bytes);
        assert(rc >= 0);
        count += rc;
        buf += rc;
        if (offset) {
            offset += rc;
        }
    }

    return count;
}

void FpgaConfig::readFpga(void *varPtr, uint64_t size, uint64_t addr) {
    int fpgaFd = -1;
    char *buffer = nullptr;
    size_t pageSize = sysconf(_SC_PAGESIZE);

    fpgaFd = open(C2H[0], O_RDWR);
    assert(fpgaFd >= 0);

    posix_memalign(reinterpret_cast<void **>(&buffer), pageSize, size + pageSize);
    assert(buffer != nullptr);

    dmaRead(fpgaFd, buffer, size, addr);
    memcpy(varPtr, buffer, size);
    close(fpgaFd);
    free(buffer);
}

void FpgaConfig::writeFpga(void *varPtr, uint64_t size, uint64_t addr) {
    int fpgaFd = -1;
    char *buffer = nullptr;
    size_t pageSize = sysconf(_SC_PAGESIZE);

    fpgaFd = open(H2C[0], O_RDWR);
    assert(fpgaFd >= 0);

    posix_memalign(reinterpret_cast<void **>(&buffer), pageSize, size + pageSize);
    assert(buffer != nullptr);

    memcpy(buffer, varPtr, size);
    dmaWrite(fpgaFd, buffer, size, addr);
    close(fpgaFd);
    free(buffer);
}

void FpgaConfig::programFpga(const char *binPtr, uint64_t size, uint64_t addr) {
    int binFd = -1;
    int fpgaFd = -1;
    char *buffer = nullptr;
    size_t pageSize = sysconf(_SC_PAGESIZE);

    binFd = open(binPtr, O_RDONLY);
    assert(binFd >= 0);

    fpgaFd = open(H2C[1], O_RDWR);
    assert(fpgaFd >= 0);

    posix_memalign(reinterpret_cast<void **>(&buffer), pageSize, size + pageSize);
    assert(buffer != nullptr);

    dmaRead(binFd, buffer, size, 0);
    dmaWrite(fpgaFd, buffer, size, addr);
    close(fpgaFd);
    close(binFd);
    free(buffer);
}