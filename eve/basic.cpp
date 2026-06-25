#include <eve/eve.hpp>
#include <iostream>

int main()
{
  eve::wide<float, eve::fixed<8>> x( [](auto i, auto) { return 1.f+i; } );

  std::cout << "EVE is optimizing for: " << eve::current_api << "\n";
  std::cout << "x     = " << x.get(0) << "\n";
  std::cout << "2*x   = " << 1 - x << "\n";
  std::cout << "x^0.5 = " << eve::sqrt(eve::abs(1 - x)) << "\n";

  return 0;
}
