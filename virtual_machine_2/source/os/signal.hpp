#pragma once

#include <cstdint>

namespace lamavm::os {

struct page_fault {
  std::uintptr_t address = 0;
};

using page_fault_handler = void (*)(page_fault fault);

void register_page_fault_handler(page_fault_handler handler);

} // namespace lamavm::os
