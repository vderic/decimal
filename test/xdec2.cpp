#include "decimal_wrapper.hpp"

int main() {

  Decimal128 d1(100);
  d1.print(38, 2);
  Decimal128 d2(20);

  d1 /= d2;

  d1.print(38, 2);

  d1 = d1.IncreaseScaleBy(3);

  d1.print(38, 2);

  printf("size of deciml128 = %lu\n", sizeof(Decimal128));

  Decimal128 d3(1234567);
  Decimal128 d4;
  int precision, scale;

  decimal_status_t s =
      Decimal128::FromString("1234567", &d4, &precision, &scale);
  if (s) {
    printf("ERROR\n");
    return 1;
  }

  if (d3 == d4) {
    printf("OK\n");
  } else {
    printf("FAILED\n");
  }

  d3.print(38, 0);
  d4.print(38, 0);

  Decimal128 d5 = d1 + d2;
  d5.print(38, 0);

  return 0;
}
