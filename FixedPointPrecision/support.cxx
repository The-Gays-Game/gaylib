module;
#include "arithmetic.h"
#include<cmath>
module fixed:support;
template <std::floating_point F,test_Tint B>
#ifdef FP_MANIP_CE
#define TOF_CE
constexpr
#endif
F toF(B v, uint8_t radix, std::float_round_style S)
    noexcept(noexcept(std::ldexp(v, int{})))
{
  using nl = NL<F>;
  constexpr uint8_t d = NL<B>::digits;
  if (d > nl::digits)
  {
    uint8_t sd;
    if constexpr (std::is_unsigned_v<B>)
      sd = d - std::countl_zero(v);
    else
    {
      auto av = condNeg<std::make_unsigned_t<B>>(v, v < 0);
      sd = NL<decltype(av)>::digits - std::countl_zero(av);
    }

    bool subnorm = nl::has_denorm == std::denorm_present && int8_t(sd - radix) <= nl::min_exponent - 1;
    if (int8_t more = sd - nl::digits; S != std::round_indeterminate && !subnorm &&
#if defined(__GNUG__)||__has_builtin(__builtin_expect_with_probability)
        __builtin_expect_with_probability(more > 0, true, (d - nl::digits) / static_cast<double>(d))
#else
        more > 0
#endif
    )
    {
      //with radix<=128, sd<=128, then sd<=2 needs no rounding.
      v = aint_dt<typename rankOf<B>::half>(v).narrowArsRnd(more, S);
      radix -= more;
      return std::ldexp(v, -int8_t(radix));
    }
  }
  return std::ldexp(v, -int16_t(radix));
}