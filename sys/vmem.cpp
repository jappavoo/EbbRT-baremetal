#include <atomic>
#include <cinttypes>
#include <cstdint>
#include <cstring>

#include <sys/align.hpp>
#include <sys/debug.hpp>
#include <sys/e820.hpp>
#include <sys/early_page_allocator.hpp>
#include <sys/page_allocator.hpp>
#include <sys/vmem.hpp>

using namespace ebbrt;

pte ebbrt::page_table_root;

void ebbrt::early_map_memory(uint64_t addr, uint64_t length) {
  auto aligned_addr = align_down(addr, PAGE_SIZE);
  auto aligned_length = align_up(length + (addr - aligned_addr), PAGE_SIZE);

  traverse_page_table(page_table_root, aligned_addr,
                      aligned_addr + aligned_length, 0, 4,
                      [=](pte & entry, uint64_t base_virt, size_t level) {
                        if (entry.present()) {
                          kassert(entry.addr(level > 0) == base_virt);
                          return;
                        }
                        entry.set(base_virt, level > 0);
                        std::atomic_thread_fence(std::memory_order_release);
                        asm volatile("invlpg (%[addr])"
                                     :
                                     : [addr] "r"(base_virt)
                                     : "memory");
                      },
                      [=](pte & entry) {
    auto page = early_allocate_page();
    auto page_addr = pfn_to_addr(page);
    new (reinterpret_cast<void *>(page_addr)) pte[512];

    entry.set_normal(page_addr);
    return true;
  });
}

void ebbrt::early_unmap_memory(uint64_t addr, uint64_t length) {
  auto aligned_addr = align_down(addr, PAGE_SIZE);
  auto aligned_length = align_up(length + (addr - aligned_addr), PAGE_SIZE);

  traverse_page_table(page_table_root, aligned_addr,
                      aligned_addr + aligned_length, 0, 4,
                      [=](pte & entry, uint64_t base_virt, size_t level) {
                        kassert(entry.present());
                        entry.set_present(false);
                        std::atomic_thread_fence(std::memory_order_release);
                        asm volatile("invlpg (%[addr])"
                                     :
                                     : [addr] "r"(base_virt)
                                     : "memory");
                      },
                      [=](pte & entry) {
    kprintf("Asked to unmap memory that wasn't mapped!\n");
    kabort();
    return false;
  });
}

void ebbrt::map_memory(pfn_t vfn, pfn_t pfn, uint64_t length) {
  auto pte_root = pte{read_cr3()};
  auto vaddr = pfn_to_addr(vfn);
  traverse_page_table(pte_root, vaddr, vaddr + length, 0, 4,
                      [=](pte & entry, uint64_t base_virt, size_t level) {
                        kassert(!entry.present());
                        entry.set(pfn_to_addr(pfn) + (base_virt - vaddr),
                                  level > 0);
                        std::atomic_thread_fence(std::memory_order_release);
                      },
                      [](pte & entry) {
    auto page = page_allocator->Alloc();
    kbugon(page == 0);
    auto page_addr = pfn_to_addr(page);
    new (reinterpret_cast<void *>(page_addr)) pte[512];
    entry.set_normal(page_addr);
    return true;
  });
}

void ebbrt::enable_runtime_page_table() {
  asm volatile("mov %[page_table], %%cr3"
               :
               : [page_table] "r"(page_table_root));
}

void ebbrt::vmem_ap_init(size_t index) {
  enable_runtime_page_table();
  pte ap_pte_root;
  auto nid = cpus[index].get_nid();
  auto &p_allocator = PageAllocator::allocators[nid];
  auto page = p_allocator.Alloc(0, nid);
  kbugon(page == 0, "Failed to allocate page for initial page tables\n");
  auto page_addr = pfn_to_addr(page);
  std::memcpy(reinterpret_cast<void *>(page_addr),
              reinterpret_cast<void *>(page_table_root.addr(false)), 4096);
  ap_pte_root.set_normal(page_addr);

  asm volatile("mov %[page_table], %%cr3" : : [page_table] "r" (ap_pte_root));
}
