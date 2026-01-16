module;
#include "defs.h"
#include<cstdint>
#include <cmath>
#include<type_traits>
#include<concepts>
#include <tuple>
export module fpn;
export import :core;
export import :ari;
template <class T, uint8_t R> concept testSize = NL<T>::digits >= R;
using namespace fpn::core;
template<std::integral,uint8_t,std::float_round_style>
struct afx{};

export namespace fpn {
template <std::signed_integral Bone,uint8_t Radix,std::float_round_style Style> requires testSize<Bone,Radix>
struct fx {
    Bone repr;
  constexpr
  static fx raw(Bone a)
  noexcept {
    fx b;
    b.repr=a;
    return b;
  }
  CMATH_CE23
  std::tuple<Bone,fx> remQuo(fx divisor)const {
#ifdef checkArgs
    if (divisor.repr==0)
      throw std::domain_error("zero divisor.");
#endif
    auto [q,r]=remQuoS(repr,divisor.repr);
    return {q,raw(r)};
  }
};
  template <std::unsigned_integral Bone,uint8_t Radix,std::float_round_style Style> requires testSize<Bone,Radix>
struct ufx {
    Bone repr;
    static ufx raw(Bone a)
noexcept {
      ufx b;
      b.repr=a;
      return b;
    }
    constexpr
auto remQuo(ufx divisor,bool *overflow=nullptr)const
requires(Radix<NL<Bone>::digits)//at Radix==digits there's a loss of precision when converting to fx
{
      using Ts=fx<std::make_signed_t<Bone>,Radix,Style>;
      auto [q,r,of]=remQuoS(repr,divisor.repr);
      if (overflow!=nullptr)
        *overflow=of;
      return std::tuple<Bone,Ts>{q,Ts::raw(r)};
}
  };
template<std::integral B,uint8_t R,std::float_round_style S>
using afx_t=afx<B,R,S>::type;
}
template<std::signed_integral B,uint8_t R,std::float_round_style S>
struct afx<B,R,S> {
  using type=fpn::fx<B,R,S>;
};
template<std::unsigned_integral B,uint8_t R,std::float_round_style S>
struct afx<B,R,S> {
  using type=fpn::ufx<B,R,S>;
};