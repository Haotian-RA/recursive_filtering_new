#ifndef SHIFT_REG_H
#define SHIFT_REG_H 1

#include "../src/vcl/vectorclass.h"
#include <utility> 

template<typename V> class Shift{

    using T = decltype(std::declval<V>().extract(0));
    constexpr static int L = V::size(); 

    private:

        V _buffer{0};

    public:

        Shift(){};

        inline void shift(const T x){

            // SSE
            if constexpr (L == 4) _buffer = blend4<1,2,3,4>(_buffer, x); 
            
            // AVX2
            if constexpr (L == 8) _buffer = blend8<1,2,3,4,5,6,7,8>(_buffer, x); 
            
            // AVX512
            if constexpr (L == 16) _buffer = blend16<1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16>(_buffer, x);
        }; 
            
        inline void shift(const V x){ _buffer = x;}; 
         
        inline T operator[](const int idx){ return (idx < 0) ? _buffer[L+idx] : _buffer[idx];}; 

        inline void reset(){ _buffer = V(0);};  

        
};

#endif 
