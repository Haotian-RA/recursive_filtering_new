#ifndef BLOCK_FILTERING_H
#define BLOCK_FILTERING_H 1

#include "../src/vcl/vectorclass.h"
#include <array>
#include "shift_reg.h"


template<typename V> class BlockFiltering{

    using T = decltype(std::declval<V>().extract(0));
    constexpr static int L = V::size(); 

    private:

        T _b1, _b2, _a1, _a2; 

        V _p2, _p1, _h2, _h1;

        std::array<V,L> _H;

        Shift<V> _PS, _HS;

    public:

        BlockFiltering(){};

        BlockFiltering(const T b1,const T b2,const T a1,const T a2,const T xi1=0,const T xi2=0,const T yi1=0,const T yi2=0): _b1(b1), _b2(b2), _a1(a1), _a2(a2){

            impulse_response();
            H_factor();

            _PS.shift(xi2);
            _PS.shift(xi1);
            _HS.shift(yi2);
            _HS.shift(yi1);
        };

        inline V operator()(const V x){

            V y{0};

            for (auto l=0; l<L; l++) {
                y = mul_add(_H[l],x[l],y);
            } 

            y = mul_add(_p2,_PS[-2],y);
            y = mul_add(_p1,_PS[-1],y);

            y = mul_add(_h2,_HS[-2],y);
            y = mul_add(_h1,_HS[-1],y);

            _PS.shift(x);
            _HS.shift(y);

            return y; 
        };


        inline void impulse_response(){
        
            T p2[L+1]={0}, p1[L+1]={0}, h0[L+1]={0}, h2[L]={0};

            p2[0] = _b2;
            p2[1] = _a1*_b2;
            p1[0] = _b1;
            p1[1] = _a1*_b1 + _b2;
            h0[0] = 1;
            h0[1] = _a1;

            for (auto l=2; l<L+1; l++){

                p2[l] = _a1*p2[l-1] + _a2*p2[l-2];
                p1[l] = _a1*p1[l-1] + _a2*p1[l-2];
                h0[l] = _a1*h0[l-1] + _a2*h0[l-2];  
            }

            for (auto l=0;l<L;l++) h2[l] = _a2*h0[l];

            _p2.load(&p2[0]);
            _p1.load(&p1[0]);
            _h1.load(&h0[1]);
            _h2.load(&h2[0]);

        };

        inline void H_factor(){

            V tmp={0};

            // SSE
            if constexpr (L == 4){

                tmp = blend4<4,0,1,2>(_h1+_p1, 1);

                for (int l=0;l<L;l++){

                    _H[l] = tmp;
                    tmp = permute4<-1,0,1,2>(tmp);
                }
            }

            // AVX2
            if constexpr (L == 8){ 
                tmp = blend8<8,0,1,2,3,4,5,6>(_h1+_p1, 1);

                for (auto l=0; l<L; l++){
                    _H[l] = tmp;
                    tmp = permute8<-1,0,1,2,3,4,5,6>(tmp);
                }
            }

            // AVX512
            if constexpr (L == 16){ 
                tmp = blend16<16,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(_h1+_p1, 1);

                for (auto l=0; l<L; l++){
                    _H[l] = tmp;
                    tmp = permute16<-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(tmp);
                }
            }
        };

};





#endif
