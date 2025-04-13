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
    uint8_t f=1;
    uint32_t g=0;
    std::cout<<(unsigned int)(f<<12)<<std::endl;
}