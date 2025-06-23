module;
#include "arithmetic.h"

#include <cfenv>
#include<cmath>
export module fixed:testHelper;
export{
  template <std::floating_point F,test_Tint B>
#ifdef FP_MANIP_CE
#define TOF_CE
constexpr
#endif
F e_toF(B v, uint8_t radix, std::float_round_style S)
    noexcept(noexcept(std::ldexp(v, int{}))) {
    return toF<F>(v,radix,S);
  }
  constexpr int styleMacroMap[4]{FE_TOWARDZERO,FE_TONEAREST,FE_UPWARD,FE_DOWNWARD};
  constexpr std::float_round_style styleEnumMap[4]{
    std::round_toward_zero, std::round_to_nearest, std::round_toward_infinity, std::round_toward_neg_infinity
};
}