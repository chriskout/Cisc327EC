#include <time.h>
#include <stdio.h>
int main() {
  printf("C version: %ld\n", __STDC_VERSION__);
  fflush(stdout);
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
}
