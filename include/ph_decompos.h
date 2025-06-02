#ifndef PH_DECOMPOS_H
#define PH_DECOMPOS_H 1

#include "../src/vcl/vectorclass.h"
#include <array>
#include "recursive_doubling.h"
#include "shift_reg.h"
#include <tuple>


template<typename V,size_t N> class PartSolutionV{

    using T = decltype(std::declval<V>().extract(0));
    constexpr static int L = V::size(); 

    private:

        T _b2, _b1, _a2, _a1;
        V _h2, _h1;

    public:

        PartSolutionV(){};

        PartSolutionV(const T a1,const T a2): _a1(a1),_a2(a2){};

        __attribute__((always_inline))
        inline std::array<V,N> operator()(const std::array<V,N>& v){

            std::array<V,N> w;

            w[0] = v[0];
            w[1] = mul_add(v[0],_a1,v[1]);
        
            #pragma unroll
            for (size_t n=2;n<N;n++){

                w[n] = mul_add(w[n-2],_a2,v[n]);
                w[n] = mul_add(w[n-1],_a1,w[n]);
            } 
        
            return w;
        }

};



template<typename V,size_t N> class HomoSolutionV{

    using T = decltype(std::declval<V>().extract(0));
    constexpr static int L = V::size(); 


    private:

        T _a2, _a1;
        RecurDoubling<V,N> _RD;
        Shift<V> _S;

    public:

        HomoSolutionV(){};

        __attribute__((always_inline))
        HomoSolutionV(const T a1,const T a2,const T yi1=0,const T yi2=0): _a1(a1),_a2(a2){

            _RD = RecurDoubling<V,N>(a1,a2); 
            _S.shift(yi2);
            _S.shift(yi1);
        };

        __attribute__((always_inline))
        inline std::array<V,N> forward(const std::array<V,N>& w,const V yv1,const V yv2){

            std::array<V,N> y;
            V yvi1,yvi2;

            // SSE
            if constexpr (L == 4){

                yvi2 = blend4<4,0,1,2>(yv2, _S[-2]);
                yvi1 = blend4<4,0,1,2>(yv1, _S[-1]);
            }

            // AVX2
            if constexpr (L == 8){

                yvi2 = blend8<8,0,1,2,3,4,5,6>(yv2, _S[-2]);
                yvi1 = blend8<8,0,1,2,3,4,5,6>(yv1, _S[-1]);
            }

            // AVX512
            if constexpr (L == 16){

                yvi2 = blend16<16,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(yv2, _S[-2]);
                yvi1 = blend16<16,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(yv1, _S[-1]);
            }
            
            #pragma unroll
            for (auto n=0; n<N-2; n++){

                y[n] = mul_add(yvi2, _RD._h2[n], w[n]);
                y[n] = mul_add(yvi1, _RD._h1[n], y[n]);
            };

            y[N-2] = yv2;
            y[N-1] = yv1;

            _S.shift(y[N-2][L-1]);
            _S.shift(y[N-1][L-1]); 

            return y;

        }

        __attribute__((always_inline))
        inline std::array<V,N> operator()(const std::array<V,N>& w){

            auto yv = _RD.block_recursive_doubling(w[N-1],w[N-2],_S[-1],_S[-2]);

            auto y = forward(w,std::get<0>(yv),std::get<1>(yv));

            return y;

        }

};



#endif
