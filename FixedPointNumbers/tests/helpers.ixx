module;
#include<cfenv>
#include<limits>
#include<utility>
#include<random>
#include <cstdint>
//import fpn;
export module helpers;
//using namespace fpn;
export{
  constexpr int styleMacroMap[4]{FE_TOWARDZERO, FE_TONEAREST, FE_UPWARD, FE_DOWNWARD};
  constexpr std::float_round_style styleEnumMap[4]{
    std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity};
  //constexpr auto styleEnumSeq = std::integer_sequence<int8_t, std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity>{};
  static_assert(std::size(styleEnumMap) == std::size(styleMacroMap));
  inline std::minstd_rand rg32;
  /*template <class T>
  using NL = std::numeric_limits<T>;

  template<std::unsigned_integral Tu>
  Tu rndURoot2(rankOf<Tu>::half root,Tu r,std::float_round_style s) {
    switch (s) {
    case std::round_toward_infinity:
      return Tu(root)+(r!=0);
    case std::round_to_nearest: {
      Tu a= divRnd(Tu(r),root,std::round_toward_infinity);
      return Tu(root)+(a>root+1);
    }
      default:
      return root;
    }
  }*/
}
