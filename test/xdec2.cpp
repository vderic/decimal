#include "decimal.hpp"

int main() {

  decimal128_t d1 = dec128_from_int64(12345567);
  decimal128_t d2;
  int precision, scale;

  decimal_status_t s = dec128_from_string("12345567", &d2, &precision, &scale);
  if (s) {
    printf("ERROR\n");
    return 1;
  }

  if (d1 == d2) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
  }
  dec128_print(stdout, d1, 38, 0);
  dec128_print(stdout, d2, 38, 0);

  decimal128_t d3 = d1 + d2;
  dec128_print(stdout, d3, 38, 0);

  return 0;
}
