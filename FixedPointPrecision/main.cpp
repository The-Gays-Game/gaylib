#include<cstdint>
#include<iostream>
import fixed;
int main()
{
    fx<int32_t,3> a=0b1000;
    std::cout<< static_cast<float>(a)<<std::endl;
    fx<int32_t,3>b=0b1100011;
    std::cout<< static_cast<float>(b)<<std::endl;
    fx<int32_t,2>c=0b1;
    std::cout<< static_cast<float>(c)<<std::endl;
    fx<int32_t,0>d=0b100000000000000000000000;
    std::cout<< static_cast<float>(d)<<std::endl;
    float e=static_cast<float>(b);
    fx<int32_t,4>f(e);
    fx<int64_t,4>g(f);
    std::cout<<static_cast<float>(f)<<std::endl;
    std::cout<<static_cast<float>(g)<<std::endl;
    // uint8_t a=-1;
    // uint16_t b=a;
    // auto c=-a;
    // std::cout<<std::dec<<int16_t(b)<<"; "<<(int16_t)b<<"\n";
}