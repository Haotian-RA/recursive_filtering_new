#ifndef IIR_CORES_H
#define IIR_CORES_H 1


#include "../src/vcl/vectorclass.h"
#include <array>

#include "shift_reg.h"
#include "ph_decompos.h"
#include "fir_cores.h"
#include "cyclic_reduction.h"
#include "block_filtering.h"
#include "permute.h"

template<typename V,size_t N> class IirCoreOrderTwo{

    using T = decltype(std::declval<V>().extract(0));
    constexpr static int L = V::size(); 

    private:

        PartSolutionV<V,N> _PSV;
        HomoSolutionV<V,N> _HSV;
        FirCoreOrderTwo<V,N> _F;
        CyclicReduction<V,N> _CR;
        BlockFiltering<V> _BF;

        Shift<V> _PS,_HS;

        T _b1,_b2,_a1,_a2;
        

    public:

        IirCoreOrderTwo(){};

        __attribute__((always_inline))
        IirCoreOrderTwo(const T b1,const T b2,const T a1,const T a2,const T xi1=0,const T xi2=0,const T yi1=0,const T yi2=0):
        _b1(b1),_b2(b2),_a1(a1),_a2(a2){
 
            _F = FirCoreOrderTwo<V,N>(b1,b2,xi1,xi2);
            _PSV = PartSolutionV<V,N>(a1,a2); 
            _HSV = HomoSolutionV<V,N>(a1,a2,yi1,yi2);
            _CR = CyclicReduction<V,N>(a1,a2,yi1,yi2);
            _BF = BlockFiltering<V>(b1,b2,a1,a2,xi1,xi2,yi1,yi2);

            _PS.shift(xi2);
            _PS.shift(xi1);
            _HS.shift(yi2);
            _HS.shift(yi1);
        };

        __attribute__((always_inline))
        IirCoreOrderTwo(const T taps[5],const T inits[4]):
        _b1(taps[1]),_b2(taps[2]),_a1(taps[3]),_a2(taps[4]){

            _F = FirCoreOrderTwo<V,N>(taps[1],taps[2],inits[0],inits[1]);
            _PSV = PartSolutionV<V,N>(taps[3],taps[4]); 
            _HSV = HomoSolutionV<V,N>(taps[3],taps[4],inits[2],inits[3]);
            _CR = CyclicReduction<V,N>(taps[3],taps[4],inits[2],inits[3]);
            _BF = BlockFiltering<V>(taps[1],taps[2],taps[3],taps[4],inits[0],inits[1],inits[2],inits[3]);

            _PS.shift(inits[1]);
            _PS.shift(inits[0]);
            _HS.shift(inits[3]);
            _HS.shift(inits[2]);
        };

        // __attribute__((always_inline))
        // inline T operator()(const T x){

        //     T y = x + _b2*_PS[-2] + _b1*_PS[-1] + _a2*_HS[-2] + _a1*_HS[-1];

        //     _PS.shift(x);
        //     _HS.shift(y);

        //     return y;
        // }

        // __attribute__((always_inline))
        // inline V operator()(const V x){

        //     V y = _BF(x);

        //     return y;
        // }

        // __attribute__((always_inline))
        // inline std::array<V,N> operator()(const std::array<V,N>& x){

        //     auto v = _F(x);
        //     auto w = _PSV(v);
        //     auto y = _HSV(w);

        //     return y;
        // }

        // __attribute__((always_inline))
        // inline std::array<V,N> operator()(const std::array<V,N>& x){

        //     auto v = _F(x);
        //     auto y = _CR(v);

        //     return y;
        // }

        __attribute__((always_inline))
        inline std::array<V,N> operator()(const std::array<V,N>& x){

            auto x_T = permuteV(x);
            auto v = _F(x_T);
            auto w = _PSV(v);
            auto y_T = _HSV(w);
            auto y = depermuteV(y_T);

            return y;
        }

        // __attribute__((always_inline))
        // inline std::array<V,N> operator()(const std::array<V,N>& x){

        //     auto x_T = permuteV(x);
        //     auto v = _F(x_T);
        //     auto y_T = _CR(v);
        //     auto y = depermuteV(y_T);

        //     return y;
        // }



};

#endif




