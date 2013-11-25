#pragma once

#include <cstdint>
#include <vector>

#include <sys/priority.hpp>

namespace ebbrt {
namespace pci {

class device {
  static const constexpr uint8_t VENDOR_ID_ADDR = 0x00;
  static const constexpr uint8_t DEVICE_ID_ADDR = 0x02;
  static const constexpr uint8_t COMMAND_ADDR = 0x04;
  static const constexpr uint8_t STATUS_ADDR = 0x06;
  static const constexpr uint8_t REVISION_ID_ADDR = 0x08;
  static const constexpr uint8_t PROG_IF_ADDR = 0x09;
  static const constexpr uint8_t SUBCLASS_ADDR = 0x0A;
  static const constexpr uint8_t CLASS_CODE_ADDR = 0x0B;
  static const constexpr uint8_t CACHE_LINE_SIZE_ADDR = 0x0C;
  static const constexpr uint8_t LATENCY_TIMER_ADDR = 0x0D;
  static const constexpr uint8_t HEADER_TYPE_ADDR = 0x0E;
  static const constexpr uint8_t BIST_ADDR = 0x0F;
  static const constexpr uint8_t BAR0_ADDR = 0x10;
  static const constexpr uint8_t BAR1_ADDR = 0x14;
  static const constexpr uint8_t BAR2_ADDR = 0x18;
  static const constexpr uint8_t BAR3_ADDR = 0x1C;
  static const constexpr uint8_t BAR4_ADDR = 0x20;
  static const constexpr uint8_t BAR5_ADDR = 0x24;
  static const constexpr uint8_t CARDBUS_CIS_PTR_ADDR = 0x28;
  static const constexpr uint8_t SUBSYSTEM_VENDOR_ID_ADDR = 0x2C;
  static const constexpr uint8_t SUBSYSTEM_ID_ADDR = 0x2E;
  static const constexpr uint8_t CAPABILITIES_PTR_ADDR = 0x34;
  static const constexpr uint8_t INTERRUPT_LINE_ADDR = 0x3C;
  static const constexpr uint8_t INTERRUPT_PIN_ADDR = 0x3D;
  static const constexpr uint8_t MIN_GRANT_ADDR = 0x3E;
  static const constexpr uint8_t MAX_LATENCY_ADDR = 0x3F;

  static const constexpr uint16_t COMMAND_BUS_MASTER = 1 << 2;
  static const constexpr uint16_t COMMAND_INT_DISABLE = 1 << 10;

  uint8_t bus_;
  uint8_t device_;
  uint8_t func_;
  bool msix_enabled_;

  uint8_t read8(uint8_t offset) const;
  uint16_t read16(uint8_t offset) const;
  uint32_t read32(uint8_t offset) const;
  void write8(uint8_t offset, uint8_t val);
  void write16(uint8_t offset, uint16_t val);
  void write32(uint8_t offset, uint32_t val);

public:
  static const constexpr uint8_t CAP_PM = 0x01;
  static const constexpr uint8_t CAP_AGP = 0x02;
  static const constexpr uint8_t CAP_VPD = 0x03;
  static const constexpr uint8_t CAP_SLOTID = 0x04;
  static const constexpr uint8_t CAP_MSI = 0x05;
  static const constexpr uint8_t CAP_CHSWP = 0x06;
  static const constexpr uint8_t CAP_PCIX = 0x07;
  static const constexpr uint8_t CAP_HT = 0x08;
  static const constexpr uint8_t CAP_VENDOR = 0x09;
  static const constexpr uint8_t CAP_DEBUG = 0x0a;
  static const constexpr uint8_t CAP_CRES = 0x0b;
  static const constexpr uint8_t CAP_HOTPLUG = 0x0c;
  static const constexpr uint8_t CAP_SUBVENDOR = 0x0d;
  static const constexpr uint8_t CAP_AGP8X = 0x0e;
  static const constexpr uint8_t CAP_SECDEV = 0x0f;
  static const constexpr uint8_t CAP_EXPRESS = 0x10;
  static const constexpr uint8_t CAP_MSIX = 0x11;
  static const constexpr uint8_t CAP_SATA = 0x12;
  static const constexpr uint8_t CAP_PCIAF = 0x13;

  device(uint8_t bus, uint8_t device, uint8_t func);

  uint16_t get_vendor_id() const;
  uint16_t get_device_id() const;
  uint16_t get_command() const;
  void set_command(uint16_t val);
  uint8_t get_revision_id() const;
  uint16_t get_subsystem_vendor_id() const;
  uint16_t get_subsystem_id() const;

  void set_bus_master(bool val);
  uint8_t get_capabilities() const;
  uint8_t find_capability(uint8_t id) const;
  void enable_msix(uint8_t pointer);
  bool is_msix_enabled() const;
  uint32_t msix_table_offset(uint8_t ptr) const;
  uint8_t msix_table_BIR(uint8_t ptr) const;
  uint32_t get_bar(uint8_t bar) const;
};

extern std::vector<bool (*)(device)> pci_driver_probes;

#define PCI_REGISTER_DRIVER(func)                                              \
  __attribute__((                                                              \
      constructor(PCI_DRIVER_PRIORITY))) void install_driver_probe() {         \
    ebbrt::pci::pci_driver_probes.push_back(func);                             \
  }

uint8_t read_config8(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);

uint16_t read_config16(uint8_t bus, uint8_t device, uint8_t func,
                       uint8_t offset);

uint32_t read_config32(uint8_t bus, uint8_t device, uint8_t func,
                       uint8_t offset);

void init();
}
}
