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
void test0()
{
    std::cout<<"A  ";
    printPair<fx<int8_t,4>>(0b10010);

    std::cout<<"B  ";
    printPair<fx<int32_t,11,std::round_toward_neg_infinity>>(1);

    std::cout<<"C  ";
    printPair<fx<int32_t,1,std::round_toward_neg_infinity>>(0b1111111111111111111111111);

    std::cout<<"D  ";
    printPair<fx<int32_t,1,std::round_toward_infinity>>((1<<26));
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

void test4()
{
    fx<int16_t,5,std::round_to_nearest>a(-.0f);
    std::cout<<std::setprecision(9)<<float(a)<<std::endl;
    std::cout<<std::setprecision(17)<<double(a)<<std::endl;
}
int main()
{
    test0();
    test4();
}
