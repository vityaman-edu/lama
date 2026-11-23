#pragma once

#include "os/signal.hpp"

namespace lamavm::interpreter {

void register_stack_fault_handler();

void stack_fault_handler(os::page_fault fault) noexcept;

} // namespace lamavm::interpreter
