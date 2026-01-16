module;
#include<cfenv>
#include<limits>
#include<utility>
#include<random>
#include <cstdint>
export module helpers;

export{
  constexpr int styleMacroMap[4]{FE_TOWARDZERO, FE_TONEAREST, FE_UPWARD, FE_DOWNWARD};
  constexpr std::float_round_style styleEnumMap[4]{
    std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity};
  //constexpr auto styleEnumSeq = std::integer_sequence<int8_t, std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity>{};
  static_assert(std::size(styleEnumMap) == std::size(styleMacroMap));
  inline std::minstd_rand rg32;
  template <class T>
  using NL = std::numeric_limits<T>;
}
