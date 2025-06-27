# gaylib
Utility library written using C++20 module. 

## fpn

A fixed point decimal library utilizing modern c++20 features while providing correct and fast implementation and allowing flexible usages. This library aims to have good test coverage, be as 
flexible, accurate, and fast as possible. 

For example, this library provides full rounding (all of `std::float_round_style`) control on calculations, allows templatized decimal point location, 
support __int128 and other compiler features, and using fast multiword algorithm when doing long multiplication and division. 

`constexpr` is taken into account when writing the library. All calculations are forwarded to compile time if possible. Template problems are greatly simplified using `concept`.