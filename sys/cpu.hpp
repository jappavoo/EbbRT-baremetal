#pragma once

#include <boost/container/static_vector.hpp>

#include <sys/numa.hpp>

namespace ebbrt {

struct cpu {
  cpu(uint8_t acpi_id, uint8_t apic_id)
      : acpi_id{ acpi_id }, apic_id{ apic_id } {}
  uint8_t acpi_id;
  uint8_t apic_id;
  nid_t nid;
};

const constexpr size_t MAX_NUM_CPUS = 256;

extern thread_local size_t my_cpu_index;

extern boost::container::static_vector<cpu, MAX_NUM_CPUS> cpus;

inline nid_t my_node() {
  return cpus[my_cpu_index].nid;
}

const constexpr uint32_t IA32_FS_BASE = 0xC0000100;

inline uint64_t rdmsr(uint32_t index) {
  uint32_t low, high;
  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(index));
  return low | (uint64_t(high) << 32);
}

inline void wrmsr(uint32_t index, uint64_t data) {
  uint32_t low = data;
  uint32_t high = data >> 32;
  asm volatile("wrmsr" : : "c"(index), "a"(low), "d"(high));
}
}
