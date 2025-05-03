#include<cstdint>
#include <format>
#include<iostream>
#include<limits>
#include<iomanip>
import fixed;
template <class Y,class T>
static void printPair(T a)
{
    auto b=Y(a,true),c=Y(-a,true);
    std::cout<<std::setprecision(9)<<float(b)<<" "<<float(c)<<std::endl;
}
template <class Y,class T>
static void printPair1(T a)
{
    auto b=Y(a),c=Y(-a);
    std::cout<<std::format("{:b}",b.repr)<<" "<<std::format("{:b}",c.repr)<<std::endl;
}
void test0()
{
    std::cout<<"A  ";
    printPair<fx<int8_t,4>>(0b10010);

    std::cout<<"B  ";
    printPair<fx<int32_t,11,std::round_toward_neg_infinity>>(1);

    std::cout<<"C  ";
    printPair<fx<int32_t,1,std::round_toward_neg_infinity>>(0b1111111111111111111111111);

    std::cout<<"D  ";
    printPair<fx<int32_t,1,std::round_toward_zero>>((1<<26));
}
void test2()
{
    // std::cout<<"A  ";
    // printPair1<fx<int8_t,4>>(1.125f);
    //
    // std::cout<<"B  ";
    // printPair1<fx<int32_t,11,std::round_toward_neg_infinity>>(0.00048828125f);
    //
    // std::cout<<"C  ";
    // printPair1<fx<int32_t,1,std::round_toward_neg_infinity>>(33554431);

    std::cout<<"D  ";//33554432.f
    printPair1<fx<int32_t,1,std::round_toward_infinity>>(float(1<<21));
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
static int fcvt(float n,uint8_t b=23)
{
    const int magic=(1<<b-1)*3;
    //-0x4ac00000-0x4B400000std::bit_cast<int>(float(magic))
    //n+=3<<22;
    return ((std::bit_cast<int>(n+float(magic))&0x007fffff))-0x00400000;
}

static uint32_t fcvtu(float n)
{
    const int magic=(1<<22)*3;
    return (std::bit_cast<uint32_t>(n+float(magic))-0x400000)&0x7fffff;
}
static int64_t fcvt(double n)
{
    const int64_t magic=(int64_t(1)<<51)*3;
    return (std::bit_cast<int64_t>(n+double(magic))&0xfffffffffffff)-0x8000000000000;
}
void test1()
{
    const int t=(1<<22)-1;
    float a=(1<<22)-1,b=0.5,c=1,d=1.5,e=2,f=2.5;
    std::cout<<std::setprecision(9)<<fcvt(a)<<" "<<fcvt(b)<<" "<<fcvt(c)<<" "<<fcvt(d)<<" "<<fcvt(e)<<" "<<fcvt(f)<<std::endl;
    std::cout<<fcvt(-a)<<" "<<fcvt(-b)<<" "<<fcvt(-c)<<" "<<fcvt(-d)<<" "<<fcvt(-e)<<" "<<fcvt(-f)<<std::endl;
}
int main()
{
    //test2();
    test1();
}
