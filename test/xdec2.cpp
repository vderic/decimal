#include "decimal_wrapper.hpp"
#include <iostream>

int main() {

  Decimal128 d1(100);
  d1 = d1.IncreaseScaleBy(2);

  Decimal128 d2(20);

  d1 /= d2;

  std::cout << d1.ToString(2) << std::endl;

  d1 = d1.IncreaseScaleBy(3);

  std::cout << d1.ToString(2) << std::endl;

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

  std::cout << d3.ToString(0) << std::endl;
  std::cout << d4.ToString(0) << std::endl;

  Decimal128 d5 = d1 + d2;
  std::cout << d5.ToString(0) << std::endl;

  int p1, p2, p3, s1, s2, s3;
  Decimal128 dd1, dd2, dd3;
  s = Decimal128::FromString("123456.24", &dd1, &p1, &s1);
  s = Decimal128::FromString("3456.24", &dd2, &p2, &s2);

  dec128_DIV_precision_scale(p1, s1, p2, s2, &p3, &s3);
  dd3 = Decimal128::Divide(dd1, s1, dd2, s2, p3, s3);

  std::cout << dd1.ToString(s1) << " / " << dd2.ToString(s2) << " = "
            << dd3.ToString(s3) << std::endl;

  dd3.Negate();
  std::cout << dd3.ToString(s3) << std::endl;
  dd3.Abs();
  std::cout << dd3.ToString(s3) << std::endl;

  std::string str = dd3.ToString(s3);
  float fp32 = dd3.ToFloat(s3);
  double fp64 = dd3.ToDouble(s3);
  int64_t i64 = static_cast<int64_t>(dd3);
  std::cout << str << ", " << std::to_string(fp32) << ", "
            << std::to_string(fp64) << ", " << std::to_string(i64) << std::endl;

  Decimal128 d6;

  precision = 18;
  scale = 6;
  s = Decimal128::FromReal(fp32, &d6, precision, scale);
  if (s) {
    printf("FromReal error");
  }
  std::cout << d6.ToString(scale) << std::endl;

  s = Decimal128::FromReal(fp64, &d6, precision, scale);
  if (s) {
    printf("FromReal error");
  }
  std::cout << d6.ToString(scale) << std::endl;

  // floor
  Decimal128 floor = Decimal128::Floor(d6, scale - 2);
  std::cout << floor.ToString(scale) << std::endl;

  // ceil
  Decimal128 ceil = Decimal128::Floor(d6, scale - 4);
  std::cout << ceil.ToString(scale) << std::endl;

  // mod
  p1 = precision;
  s1 = scale;
  Decimal128 d7;
  Decimal128::FromString("12.3456", &d7, &p2, &s2);

  dec128_MOD_precision_scale(p1, s1, p2, s2, &p3, &s3);

  Decimal128 mod = Decimal128::Mod(d6, s1, d7, s2);

  std::cout << d6.ToString(s1) << " % " << d7.ToString(s2) << " = "
            << mod.ToString(s3) << std::endl;

  // round
  Decimal128 round = Decimal128::Round(mod, s3, s3 - 2);
  std::cout << "round(" << mod.ToString(s3) << "," << std::to_string(s3 - 2)
            << ")=" << round.ToString(s3) << std::endl;

  // ToIntegerString
  std::cout << "toIntegerString(): " << round.ToIntegerString() << std::endl;

  return 0;
}
