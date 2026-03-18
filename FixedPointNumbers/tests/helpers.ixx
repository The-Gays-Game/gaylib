module;
#include<cfenv>
#include<limits>
#include<utility>
#include<random>
#include <cstdint>
import fpn;
export module helpers;
//using namespace fpn;
export{
  constexpr int styleMacroMap[4]{FE_TOWARDZERO, FE_TONEAREST, FE_UPWARD, FE_DOWNWARD};
  constexpr std::float_round_style styleEnumMap[4]{
    std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity};
  //constexpr auto styleEnumSeq = std::integer_sequence<int8_t, std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity>{};
  static_assert(std::size(styleEnumMap) == std::size(styleMacroMap));
  std::minstd_rand rg32{std::random_device()()};

template <std::unsigned_integral Tu>
constexpr Tu uRoot2(const Tu base, const std::float_round_style S)
noexcept {
		Tu root,rem;
		std::tie(root,rem)=sqrtRem(base);
		switch (S) {
		case std::round_toward_infinity: {
			root+=rem!=0;
		} break;
		case std::round_to_nearest: {
			root+=rem>root;
		}
		default:;
		}
		return root;
	}
}
