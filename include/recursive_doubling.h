#ifndef RECURSIVE_DOUBLING_H
#define RECURSIVE_DOUBLING_H 1

#include "../src/vcl/vectorclass.h"
#include <array>
#include "ph_decompos.h"
#include <algorithm>
#include <tuple>

#include <iostream>


template<typename V,size_t N> class RecurDoubling{

    using T = decltype(std::declval<V>().extract(0));
    constexpr static int L = V::size(); 

    private:

        T _a2, _a1;
        T _h2[N], _h1[N];

        V _h_22, _h_12, _h_21, _h_11;

        V _rd0_22, _rd0_12, _rd0_21, _rd0_11; 
        V _rd1_22, _rd1_12, _rd1_21, _rd1_11; 
        V _rd2_22, _rd2_12, _rd2_21, _rd2_11;
        V _rd3_22, _rd3_12, _rd3_21, _rd3_11;
        V _rd4_22, _rd4_12, _rd4_21, _rd4_11;

        template<typename,size_t> friend class HomoSolutionV;

    public:

        RecurDoubling(){};

        __attribute__((always_inline))
        RecurDoubling(const T a1,const T a2): _a1(a1),_a2(a2){

            recursive_impulse_response();
            recursive_doubling_factors();
        };

        inline std::tuple<V,V> block_recursive_doubling(const V wv1,const V wv2,const T yi1,const T yi2){

            V yv1,yv2,tmp1,tmp2;

            // step 1: initialization
            yv2 = mul_add(yi2, _rd0_22, wv2);
            yv1 = mul_add(yi2, _rd0_21, wv1);
            yv2 = mul_add(yi1, _rd0_12, yv2);
            yv1 = mul_add(yi1, _rd0_11, yv1);

            // SSE
            if constexpr (L == 4){

                // step 2: first recursion
                tmp2 = permute4<-1,0,-1,2>(yv2);
                tmp1 = permute4<-1,0,-1,2>(yv1);

                yv2 = mul_add(tmp2, _rd1_22, yv2);
                yv1 = mul_add(tmp2, _rd1_21, yv1);
                yv2 = mul_add(tmp1, _rd1_12, yv2);
                yv1 = mul_add(tmp1, _rd1_11, yv1);

                // step 3: second recursion
                tmp2 = permute4<-1,-1,1,1>(yv2);
                tmp1 = permute4<-1,-1,1,1>(yv1);

                yv2 = mul_add(tmp2, _rd2_22, yv2);
                yv1 = mul_add(tmp2, _rd2_21, yv1);
                yv2 = mul_add(tmp1, _rd2_12, yv2);
                yv1 = mul_add(tmp1, _rd2_11, yv1);
            };

            // AVX2
            if constexpr (L == 8){ 
                // step 2: first recursion
                tmp2 = permute8<-1,0,-1,2,-1,4,-1,6>(yv2);
                tmp1 = permute8<-1,0,-1,2,-1,4,-1,6>(yv1);

                yv2 = mul_add(tmp2, _rd1_22, yv2);
                yv1 = mul_add(tmp2, _rd1_21, yv1);
                yv2 = mul_add(tmp1, _rd1_12, yv2);
                yv1 = mul_add(tmp1, _rd1_11, yv1);

                // step 3: second recursion
                tmp2 = permute8<-1,-1,1,1,-1,-1,5,5>(yv2);
                tmp1 = permute8<-1,-1,1,1,-1,-1,5,5>(yv1);

                yv2 = mul_add(tmp2, _rd2_22, yv2);
                yv1 = mul_add(tmp2, _rd2_21, yv1);
                yv2 = mul_add(tmp1, _rd2_12, yv2);
                yv1 = mul_add(tmp1, _rd2_11, yv1);

                // step 4: third recursion
                tmp2 = permute8<-1,-1,-1,-1,3,3,3,3>(yv2);
                tmp1 = permute8<-1,-1,-1,-1,3,3,3,3>(yv1);

                yv2 = mul_add(tmp2, _rd3_22, yv2);
                yv1 = mul_add(tmp2, _rd3_21, yv1);
                yv2 = mul_add(tmp1, _rd3_12, yv2);
                yv1 = mul_add(tmp1, _rd3_11, yv1);
            };

            // AVX512
            if constexpr (L == 16){ 
                // step 2: first recursion
                tmp2 = permute16<-1,0,-1,2,-1,4,-1,6,-1,8,-1,10,-1,12,-1,14>(yv2);
                tmp1 = permute16<-1,0,-1,2,-1,4,-1,6,-1,8,-1,10,-1,12,-1,14>(yv1);

                yv2 = mul_add(tmp2, _rd1_22, yv2);
                yv1 = mul_add(tmp2, _rd1_21, yv1);
                yv2 = mul_add(tmp1, _rd1_12, yv2);
                yv1 = mul_add(tmp1, _rd1_11, yv1);

                // step 3: second recursion
                tmp2 = permute16<-1,-1,1,1,-1,-1,5,5,-1,-1,9,9,-1,-1,13,13>(yv2);
                tmp1 = permute16<-1,-1,1,1,-1,-1,5,5,-1,-1,9,9,-1,-1,13,13>(yv1);

                yv2 = mul_add(tmp2, _rd2_22, yv2);
                yv1 = mul_add(tmp2, _rd2_21, yv1);
                yv2 = mul_add(tmp1, _rd2_12, yv2);
                yv1 = mul_add(tmp1, _rd2_11, yv1);

                // step 4: third recursion
                tmp2 = permute16<-1,-1,-1,-1,3,3,3,3,-1,-1,-1,-1,11,11,11,11>(yv2);
                tmp1 = permute16<-1,-1,-1,-1,3,3,3,3,-1,-1,-1,-1,11,11,11,11>(yv1);

                yv2 = mul_add(tmp2, _rd3_22, yv2);
                yv1 = mul_add(tmp2, _rd3_21, yv1);
                yv2 = mul_add(tmp1, _rd3_12, yv2);
                yv1 = mul_add(tmp1, _rd3_11, yv1);

                // step 4: fourth recursion
                tmp2 = permute16<-1,-1,-1,-1,-1,-1,-1,-1,7,7,7,7,7,7,7,7>(yv2);
                tmp1 = permute16<-1,-1,-1,-1,-1,-1,-1,-1,7,7,7,7,7,7,7,7>(yv1);

                yv2 = mul_add(tmp2, _rd4_22, yv2);
                yv1 = mul_add(tmp2, _rd4_21, yv1);
                yv2 = mul_add(tmp1, _rd4_12, yv2);
                yv1 = mul_add(tmp1, _rd4_11, yv1);
            };


            return {yv1,yv2};
        }

        __attribute__((always_inline))
        inline void recursive_impulse_response(){

            T h0[N+1];
            
            h0[0] = 1;
            h0[1] = _a1;

            #pragma unroll 
            for (auto n=2; n<N+1; n++) 
                h0[n] = _a1*h0[n-1] + _a2*h0[n-2];    

            #pragma unroll
            for (size_t n=0; n<N; n++){
                _h2[n] = _a2*h0[n];   
                _h1[n] = h0[n+1]; 
            }

        };


        inline void C_factors(){
                    
            T h_22[L] = {0}, h_12[L] = {0}, h_21[L] = {0}, h_11[L] = {0}; 

            h_22[0] = _h2[N-2];
            h_12[0] = _h1[N-2];
            h_21[0] = _h2[N-1];
            h_11[0] = _h1[N-1];

            for (auto l=1; l<L; l++) {
                h_22[l] = _h2[N-2]*h_22[l-1] + _h2[N-1]*h_12[l-1];
                h_12[l] = _h1[N-2]*h_22[l-1] + _h1[N-1]*h_12[l-1];
                h_21[l] = _h2[N-2]*h_21[l-1] + _h2[N-1]*h_11[l-1];
                h_11[l] = _h1[N-2]*h_21[l-1] + _h1[N-1]*h_11[l-1];
            }

            _h_22.load(&h_22[0]);
            _h_12.load(&h_12[0]);
            _h_21.load(&h_21[0]);
            _h_11.load(&h_11[0]);
        };


        inline void recursive_doubling_factors(){

            C_factors();

            // SSE
            if constexpr (L == 4){

                // RD initialization, [C 0 0 0]
                _rd0_22 = permute4<0,-1,-1,-1>(_h_22); 
                _rd0_12 = permute4<0,-1,-1,-1>(_h_12);
                _rd0_21 = permute4<0,-1,-1,-1>(_h_21);
                _rd0_11 = permute4<0,-1,-1,-1>(_h_11);

                // RD recursion 1, [0 C 0 C]
                _rd1_22 = permute4<-1,0,-1,0>(_h_22);
                _rd1_12 = permute4<-1,0,-1,0>(_h_12);
                _rd1_21 = permute4<-1,0,-1,0>(_h_21);
                _rd1_11 = permute4<-1,0,-1,0>(_h_11);

                // RD recursion 2, [0 0 C C^2]
                _rd2_22 = permute4<-1,-1,0,1>(_h_22);
                _rd2_12 = permute4<-1,-1,0,1>(_h_12);
                _rd2_21 = permute4<-1,-1,0,1>(_h_21);
                _rd2_11 = permute4<-1,-1,0,1>(_h_11);
            };

            // AVX2
            if constexpr (L == 8){

                // RD initialization, [C 0 0 0 0 0 0 0]
                _rd0_22 = permute8<0,-1,-1,-1,-1,-1,-1,-1>(_h_22); 
                _rd0_12 = permute8<0,-1,-1,-1,-1,-1,-1,-1>(_h_12);
                _rd0_21 = permute8<0,-1,-1,-1,-1,-1,-1,-1>(_h_21);
                _rd0_11 = permute8<0,-1,-1,-1,-1,-1,-1,-1>(_h_11);

                // RD recursion 1, [0 C 0 C 0 C 0 C]
                _rd1_22 = permute8<-1,0,-1,0,-1,0,-1,0>(_h_22);
                _rd1_12 = permute8<-1,0,-1,0,-1,0,-1,0>(_h_12);
                _rd1_21 = permute8<-1,0,-1,0,-1,0,-1,0>(_h_21);
                _rd1_11 = permute8<-1,0,-1,0,-1,0,-1,0>(_h_11);

                // RD recursion 2, [0 0 C C^2 0 0 C C^2]
                _rd2_22 = permute8<-1,-1,0,1,-1,-1,0,1>(_h_22);
                _rd2_12 = permute8<-1,-1,0,1,-1,-1,0,1>(_h_12);
                _rd2_21 = permute8<-1,-1,0,1,-1,-1,0,1>(_h_21);
                _rd2_11 = permute8<-1,-1,0,1,-1,-1,0,1>(_h_11);

                // RD recursion 3, [0 0 0 0 C C^2 C^3 C^4]
                _rd3_22 = permute8<-1,-1,-1,-1,0,1,2,3>(_h_22);
                _rd3_12 = permute8<-1,-1,-1,-1,0,1,2,3>(_h_12);
                _rd3_21 = permute8<-1,-1,-1,-1,0,1,2,3>(_h_21);
                _rd3_11 = permute8<-1,-1,-1,-1,0,1,2,3>(_h_11);
            };

            // AVX512
            if constexpr (L == 16){

                // RD initialization, [C 0 0 ... 0]
                _rd0_22 = permute16<0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1>(_h_22); 
                _rd0_12 = permute16<0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1>(_h_12);
                _rd0_21 = permute16<0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1>(_h_21);
                _rd0_11 = permute16<0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1>(_h_11);

                // RD recursion 1, [0 C 0 C ... 0 C]
                _rd1_22 = permute16<-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0>(_h_22);
                _rd1_12 = permute16<-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0>(_h_12);
                _rd1_21 = permute16<-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0>(_h_21);
                _rd1_11 = permute16<-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0,-1,0>(_h_11);

                // RD recursion 2, [0 0 C C^2 0 0 C C^2 ... 0 0 C C^2]
                _rd2_22 = permute16<-1,-1,0,1,-1,-1,0,1,-1,-1,0,1,-1,-1,0,1>(_h_22);
                _rd2_12 = permute16<-1,-1,0,1,-1,-1,0,1,-1,-1,0,1,-1,-1,0,1>(_h_12);
                _rd2_21 = permute16<-1,-1,0,1,-1,-1,0,1,-1,-1,0,1,-1,-1,0,1>(_h_21);
                _rd2_11 = permute16<-1,-1,0,1,-1,-1,0,1,-1,-1,0,1,-1,-1,0,1>(_h_11);

                // RD recursion 3, [0 0 0 0 C C^2 C^3 C^4 0 0 0 0 C C^2 C^3 C^4]
                _rd3_22 = permute16<-1,-1,-1,-1,0,1,2,3,-1,-1,-1,-1,0,1,2,3>(_h_22);
                _rd3_12 = permute16<-1,-1,-1,-1,0,1,2,3,-1,-1,-1,-1,0,1,2,3>(_h_12);
                _rd3_21 = permute16<-1,-1,-1,-1,0,1,2,3,-1,-1,-1,-1,0,1,2,3>(_h_21);
                _rd3_11 = permute16<-1,-1,-1,-1,0,1,2,3,-1,-1,-1,-1,0,1,2,3>(_h_11);

                // RD recursion 4, [0 0 0 0 0 0 0 0 C C^2 C^3 C^4 C^5 C^6 C^7 C^8]
                _rd4_22 = permute16<-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7>(_h_22);
                _rd4_12 = permute16<-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7>(_h_12);
                _rd4_21 = permute16<-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7>(_h_21);
                _rd4_11 = permute16<-1,-1,-1,-1,-1,-1,-1,-1,0,1,2,3,4,5,6,7>(_h_11);
            };
        };



};






#endif
