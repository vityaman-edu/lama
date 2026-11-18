#define _GNU_SOURCE 1
#include "runtime/runtime_common.h"

#undef BOX
#define BOX(x) ((aint)(((auint)(x) << 1) | 1))

#include "runtime/runtime.c"

void lamavm_regexp_free(void* expression) {
  regfree(expression);
  free(expression);
}
