#include<cstdint>
#include<iostream>
import fixed;
int main()
{
    fx<int8_t,4> a=0;
    fx<int8_t,4> b=1;
    std::cout<<+(a+b).v;
}