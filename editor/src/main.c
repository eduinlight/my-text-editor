#include <stdio.h>

#include "rust.h"

int main() {
  for (i32 i = 0; i < 1000000; i++) {
    printf("%d\n", i);
  }
}
