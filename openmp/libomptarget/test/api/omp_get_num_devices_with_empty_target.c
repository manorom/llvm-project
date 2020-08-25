// RUN: %libomptarget-compile-run-and-check-aarch64-unknown-linux-gnu
// RUN: %libomptarget-compile-run-and-check-powerpc64-ibm-linux-gnu
// RUN: %libomptarget-compile-run-and-check-powerpc64le-ibm-linux-gnu
// RUN: %libomptarget-compile-run-and-check-x86_64-pc-linux-gnu
// RUN: env VE_LD_LIBRARY_PATH=%host-rtl-dir %libomptarget-compile-run-and-check-ve-unknown-linux-unknown

#include <omp.h>
#include <stdio.h>

static int test_omp_get_num_devices_with_empty_target() {
  /* checks that omp_get_num_devices() > 0 */
  return omp_get_num_devices() > 0;
}

int main() {
  int failed = 0;

  if (!test_omp_get_num_devices_with_empty_target()) {
    ++failed;
  }

  if (failed) {
    printf("FAIL\n");
  } else {
    printf("PASS\n");
  }

  return failed;
}

// CHECK: PASS
