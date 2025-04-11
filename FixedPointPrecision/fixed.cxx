module;
#include<concepts>
#include<cstdint>
#include<climits>
#include<type_traits>
export module fixed;
using namespace std;
export{
    template<signed_integral Bone,uint8_t Precision> requires(sizeof(Bone)*CHAR_BIT>=Precision)
    class fx
    {
        public:
        fx(const Bone& v):v(v){}
        fx(const fx&o):v(o.v){}
        fx operator+(const fx& o)const noexcept
        {
            return fx(v+o.v);
        }
    private:
Bone v;
    };
}