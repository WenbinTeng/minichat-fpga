#ifndef __FPGA_CONFIG_HPP__
#define __FPGA_CONFIG_HPP__

#include <cassert>
#include <cstdint>
#include <string>

#include <fcntl.h>
#include <unistd.h>

class FpgaConfig {
  private:
    /* data */
  public:
    FpgaConfig();
    ~FpgaConfig();
    void readFpga(void *varPtr, uint64_t size, uint64_t addr); /* Read from FPGA address space */
    void writeFpga(void *varPtr, uint64_t size, uint64_t addr); /* Write to FPGA address space */
    void programFpga(const char *binPtr, uint64_t size, uint64_t addr); /* Write to FPGA ICAP */
};

#endif