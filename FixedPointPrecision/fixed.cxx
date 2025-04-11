module;
#include<concepts>
#include<cstdint>
export module fixed;
export{
    template<std::signed_integral Bone,uint8_t at>
    class fx
    {
        public:
        fx(const Bone& v):v(v){}
        fx(const fx&o):v(o.v){}
        fx operator+(const fx& o)const
        {
            return fx(v+o.v);
        }
Bone v;
    private:

    };
}