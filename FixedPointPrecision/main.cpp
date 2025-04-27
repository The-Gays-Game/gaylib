#include<cstdint>
#include<iostream>
import fixed;
void test0()
{
    fx<int32_t,3> a(0b1000,true);
    std::cout<<"a"<< static_cast<float>(a)<<std::endl;
    fx<int32_t,3>b(0b1100011,true);
    std::cout<<"b"<< static_cast<float>(b)<<std::endl;
    fx<int32_t,2>c(0b1,true);
    std::cout<<"c"<< static_cast<float>(c)<<std::endl;
    fx<int32_t,1>d(0b100000000000000000000000);
    std::cout<<"d"<< static_cast<float>(d)<<" "<<static_cast<int32_t>(d)<<std::endl;
    float e=static_cast<float>(b);
    fx<int32_t,4>f(e);
    fx<int64_t,4>g(f);
    std::cout<<static_cast<float>(f)<<std::endl;
    std::cout<<static_cast<float>(g)<<std::endl;
    std::cout<<(f==fx<int32_t,4>(g))<<std::endl;
    ufx<uint32_t,3>h(uint32_t(a.repr));
    std::cout<<(h>=a)<<" "<<(a>h)<<std::endl;
}
void test1()
{
    //  uint32_t a=1;
    //  int32_t b=-1;
    //  auto c=a<=>b;
    //  std::cout<<(b<a)<<" "<<(b==a)<<" "<<(b>a)<<std::endl;
    // std::cout<<(c<0)<<" "<<(c==0)<<" "<<(c>0)<<std::endl;
}
void test2()
{
    int16_t a=-1;
    int16_t b=a<<2;
    uint16_t c=uint16_t(a)<<2;
    int16_t d=-4;
    //d<<=1;
    std::cout<<std::dec<<b<<"; "<<int16_t(c)<<" "<<(d>>1)<<"\n";
}
int main()
{
    test0();
    //test2();
}
