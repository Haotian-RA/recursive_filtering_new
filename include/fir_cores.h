#ifndef FIR_CORES_H
#define FIR_CORES_H 1


#include "../src/vcl/vectorclass.h"
#include "shift_reg.h"
#include <array>


template<typename V,size_t N> class FirCoreOrderTwo{

    using T = decltype(std::declval<V>().extract(0));

    constexpr static int L = V::size(); 

    private:

        Shift<V> _S;

        T _b1, _b2;

    public:

        FirCoreOrderTwo(){};

        FirCoreOrderTwo(const T b1,const T b2,const T xi1=0,const T xi2=0): _b1(b1), _b2(b2){

            _S.shift(xi2);
            _S.shift(xi1);
        };

        FirCoreOrderTwo(const T taps[3],const T inits[2]): _b1(taps[1]), _b2(taps[2]){

            _S.shift(inits[1]);
            _S.shift(inits[0]);
        };


        __attribute__((always_inline))
        inline std::array<V,N> operator()(const std::array<V,N>& x){

            V xvi2, xvi1;

            // SSE
            if constexpr (L == 4){

                xvi2 = blend4<4,0,1,2>(x[N-2], _S[-2]);
                xvi1 = blend4<4,0,1,2>(x[N-1], _S[-1]);
            }

            // AVX2
            if constexpr (L == 8){
                
                xvi2 = blend8<8,0,1,2,3,4,5,6>(x[N-2], _S[-2]);
                xvi1 = blend8<8,0,1,2,3,4,5,6>(x[N-1], _S[-1]);
            }

            // AVX512
            if constexpr (L == 16){

                xvi2 = blend16<16,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(x[N-2], _S[-2]);
                xvi1 = blend16<16,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(x[N-1], _S[-1]);
            }

            std::array<V,N> v;

            v[0] = mul_add(xvi2, _b2, x[0]); // interleave FMA of v[0] and v[1]
            v[1] = mul_add(xvi1, _b2, x[1]);
            v[0] = mul_add(xvi1, _b1, v[0]);
            v[1] = mul_add(x[0], _b1, v[1]);

            #pragma unroll  
            for (auto n=2; n<N; n++){

                v[n] = mul_add(x[n-2], _b2, x[n]);
                v[n] = mul_add(x[n-1], _b1, v[n]);
            }

            _S.shift(x[N-2][L-1]); // xi2, xi1 lies the last two (different) blocks.
            _S.shift(x[N-1][L-1]);

            return v;
        }

        inline void reset(){ _S.reset();};

        template<typename U> inline void reset(const U x){ _S.shift(x);};

};





#endif






