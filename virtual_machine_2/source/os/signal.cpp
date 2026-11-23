#include "signal.hpp"

#include "core/assert.hpp"

#include <csignal>
#include <cstdint>
#include <cstdlib>

#include <pthread.h>

extern "C" {
#include <signal.h>
}

#ifdef __APPLE__
#include <sys/signal.h>
#endif

namespace lamavm::os {

namespace {

page_fault_handler handle_page_fault = nullptr;
struct sigaction previous_segv{};
struct sigaction previous_bus{};

bool is_page_fault(int signum, int code) noexcept {
  if (signum == SIGSEGV) {
    return code == SEGV_MAPERR || code == SEGV_ACCERR;
  }

  return code == BUS_ADRALN || code == BUS_ADRERR || code == BUS_OBJERR;
}

void handle_signal(int signum, const siginfo_t& info) noexcept {
  const bool is_fault = (info.si_addr != nullptr) && is_page_fault(signum, info.si_code);
  if (is_fault) {
    handle_page_fault({.address = reinterpret_cast<std::uintptr_t>(info.si_addr)});
  }

  const auto& previous = signum == SIGSEGV ? previous_segv : previous_bus;
  if (::sigaction(signum, &previous, nullptr) != 0) {
    std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
  }

  if (is_fault) {
    return;
  }

  decltype(previous.sa_mask) unblocked{};
  if (sigemptyset(&unblocked) != 0 || sigaddset(&unblocked, signum) != 0) {
    std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
  }

  if (::pthread_sigmask(SIG_UNBLOCK, &unblocked, nullptr) != 0 || std::raise(signum) != 0) {
    std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
  }
}

} // namespace

void register_page_fault_handler(page_fault_handler handler) {
  LAMAVM_EXPECT(handle_page_fault == nullptr);
  LAMAVM_EXPECT(handler != nullptr);

  struct sigaction action{};
  action.sa_sigaction = [](int signum, siginfo_t* info, [[maybe_unused]] void* context) noexcept {
    handle_signal(signum, *info);
  };
  action.sa_flags = SA_SIGINFO | SA_ONSTACK;
  LAMAVM_EXPECT_B(sigemptyset(&action.sa_mask), ==, 0);
  LAMAVM_EXPECT_B(sigaddset(&action.sa_mask, SIGSEGV), ==, 0);
  LAMAVM_EXPECT_B(sigaddset(&action.sa_mask, SIGBUS), ==, 0);

  LAMAVM_EXPECT_B(::sigaction(SIGSEGV, nullptr, &previous_segv), ==, 0);
  LAMAVM_EXPECT_B(::sigaction(SIGBUS, nullptr, &previous_bus), ==, 0);

  handle_page_fault = handler;

  LAMAVM_EXPECT_B(::sigaction(SIGSEGV, &action, nullptr), ==, 0);
  LAMAVM_EXPECT_B(::sigaction(SIGBUS, &action, nullptr), ==, 0);
}

} // namespace lamavm::os
