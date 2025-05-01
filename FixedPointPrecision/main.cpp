#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
import fixed;
void test0()
{
    fx<int32_t,3> a(0b1000,true);
    std::cout<<"a"<< static_cast<float>(a)<<std::endl;
    fx<int32_t,3>b(0b1100011,true);
    std::cout<<"b"<< static_cast<float>(b)<<std::endl;
    fx<int32_t,2>c(0b1,true);
    std::cout<<"c"<< static_cast<float>(c)<<std::endl;
    fx<int32_t,1,std::round_to_nearest>d(0b1000000000000000000000001,false);
    std::cout<<"d"<<std::fixed<<std::setprecision(8)<< static_cast<float>(d)<<" "<<static_cast<int32_t>(d)<<std::endl;
    float e=static_cast<float>(b);
    fx<int32_t,4>f(e);
    fx<int64_t,4>g(f);
    std::cout<<static_cast<float>(f)<<std::endl;
    std::cout<<static_cast<float>(g)<<std::endl;
    std::cout<<(f==fx<int32_t,4>(g))<<std::endl;
    ufx<uint32_t,3>h(uint32_t(a.repr));
    std::cout<<(h>=a)<<" "<<(a>h)<<std::endl;
}
static int div_round(int a, int b)
{
    auto [q,r]=std::div(a,b);
    //when divisor id even, quotient is odd, round half upward.
    auto special=q&(b&1^1);//(b&1)==0&&(q&1)==1
    if (a>0&&b>0)//6/4=1%2,10/4=2%2
    {
        return q+(r>b/2-special);//q+(-r<-b/2+special);
    }else if (a<0&&b<0)//-6/-4=1%-2,-10/-4=2%-2
    {
        return q+(-r>-b/2-special);//q+(r<b/2+special);
    }else if (a>0&&b<0)//-6/4=-1%-2,-10/4=-2%-2
    {
        return q-(-r>b/2-special);//q-(r<-b/2+special);
    }else//6/-4=-1%2,10/-4=-2%2
    {
        return q-(r>-b/2-special);//q-(-r<b/2+special);
    }
}



void test3()
{
    // fx<int32_t, 3,std::round_to_nearest>a(0.666015625f);
    // std::cout<<std::format("{:b}",a.repr)<<std::endl;
    std::cout<<div_round(2,4)<<std::endl;
    std::cout<<div_round(4,4)<<std::endl;
    std::cout<<div_round(6,4)<<std::endl;
    std::cout<<div_round(14,4)<<std::endl;
    std::cout<<div_round(10,4)<<" "<<((5 ^ 2) < 0)<<std::endl;
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
    uint8_t a=0;
    uint8_t b=127;
    uint8_t c=a-b;
    int8_t d=a-b;
    auto e=c+d;
    //d<<=1;
    std::cout<<std::dec<<int16_t(int8_t(c))<<" "<<int16_t(d)<<"\n";
}
int main()
{
    //test0();
    //test2();
    test3();
}
