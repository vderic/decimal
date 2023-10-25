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

  int p1, p2, p3, s1, s2, s3;
  Decimal128 dd1, dd2, dd3;
  s = Decimal128::FromString("123456.24", &dd1, &p1, &s1);
  s = Decimal128::FromString("3456.24", &dd2, &p2, &s2);

  dd3 = Decimal128::Divide(dd1, p1, s2, dd2, p2, s2, &p3, &s3);

  dd3.print(p3, s3);

  return 0;
}
