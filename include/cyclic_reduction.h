#ifndef CYCLIC_REDUCTION_H
#define CYCLIC_REDUCTION_H 1

#include "../src/vcl/vectorclass.h"
#include <array>
#include "shift_reg.h"
#include <bit>  // c++20


template<typename V,size_t N> class CyclicReduction{

    using T = decltype(std::declval<V>().extract(0));
    constexpr static int L = V::size(); 
    constexpr static int R = std::bit_width(N)-1; 

    private:

        T _a2, _a1;

        T _f[R+1],_e[R+1],_fde[R+1];    
        T _d[R+1],_c[R+1];
        T _h[R+1],_g[R+1];

        // block filtering matrix for x
        std::array<V,L> _H;

        // block filtering vector for yi2 and yi1
        V _h2,_h1;

        // in BW, filtering the top block 
        std::array<V,R> _hb2_0,_hb1_0;

        // in BW, filtering the rest block (excluding the first round)
        std::array<V,R-1> _hb2,_hb1;

        Shift<V> _S;

    public:

        CyclicReduction(){};

        __attribute__((always_inline))
        CyclicReduction(const T a1,const T a2,const T yi1=0,const T yi2=0): _a1(a1),_a2(a2){

            gaussian_elimination_factors();
            block_filtering_factors();    
            backward_factor();
            _S.shift(yi2);
            _S.shift(yi1);
        };


        template<int round> 
        __attribute__((always_inline))
        inline std::array<V,N> forward(std::array<V,N> &x){

            if constexpr (round < R) {

                constexpr int K = 1 << round;
                constexpr int P = (N / K) >> 1;

                V tmp;
                if constexpr (L == 4) 
                    tmp = permute4<-1,0,1,2>(x[N-1]);

                if constexpr (L == 8) 
                    tmp = permute8<-1,0,1,2,3,4,5,6>(x[N-1]);

                if constexpr (L == 16)
                    tmp = permute16<-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(x[N-1]);

                x[K-1] = mul_add(-_fde[round], tmp, x[K-1]);

                #pragma unroll
                for (int n = 1; n < P; ++n) 
                    x[2*K*n + (K-1)] = mul_add(-_fde[round],x[2*K*n - 1],x[2*K*n + (K-1)]);
                
                #pragma unroll
                for (int n = 0; n < P; ++n) 
                    x[2*K*n + 2*K - 1] = mul_add(-_e[round],x[2*K*n + (K-1)],x[2*K*n + 2*K - 1]);
                
                return forward<round+1>(x);
            } 
            else
                return x;
        }

        __attribute__((always_inline))
        inline std::array<V,N> backward(const std::array<V,N>& x){

            std::array<V,N> y{0};

            for (auto l=0; l<L; l++) 
                y[N-1] = mul_add(_H[l],x[N-1][l],y[N-1]);

            y[N-1] = mul_add(_h2,-_S[-2],y[N-1]);
            y[N-1] = mul_add(_h1,-_S[-1],y[N-1]);
            
            
            V yvi2,yvi1,tmp1,tmp2;

            if constexpr (L == 4){

                yvi2 = blend4<4,0,1,2>(y[N-1],_S[-2]);
                yvi1 = blend4<4,0,1,2>(y[N-1],_S[-1]);
                tmp1 = permute4<0,0,1,2>(yvi1);
            }
            else if constexpr (L == 8) {

                yvi2 = blend8<8,0,1,2,3,4,5,6>(y[N-1], _S[-2]);
                yvi1 = blend8<8,0,1,2,3,4,5,6>(y[N-1], _S[-1]);
                tmp1 = permute8<0,0,1,2,3,4,5,6>(yvi1);

            } else {
                
                yvi2 = blend16<16,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(y[N-1], _S[-2]);
                yvi1 = blend16<16,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(y[N-1], _S[-1]); 
                tmp1 = permute16<0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(yvi1);
            }

            y[N/2-1] = mul_add(_hb2_0[R-1],-yvi2,x[N/2-1]);
            y[N/2-1] = mul_add(_hb1_0[R-1],-tmp1,y[N/2-1]);
            
            #pragma unroll
            for (int ro=R-2; ro>=0; ro--){

                int K = 1 << (ro+1);
                const int P = N/K;
                
                #pragma unroll
                for (int n=0; n<P; n++){
                    if (n == 0){

                        if constexpr (L == 4)
                            tmp2 = blend4<4,0,1,2>(y[N-K-1],_S[-2]);
                        
                        if constexpr (L == 8)    
                            tmp2 = blend8<8,0,1,2,3,4,5,6>(y[N-K-1],_S[-2]);
                        
                        if constexpr (L == 16)    
                            tmp2 = blend16<16,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(y[N-K-1],_S[-2]);
                        
                        y[K/2-1] = mul_add(_hb2_0[ro],-tmp2,x[K/2-1]);
                        y[K/2-1] = mul_add(_hb1_0[ro],-yvi1,y[K/2-1]);
                    }
                    else if (n == 1){

                        y[3*K/2-1] = mul_add(_hb2[ro],-yvi1,x[3*K/2-1]);
                        y[3*K/2-1] = mul_add(_hb1[ro],-y[K-1],y[3*K/2-1]);
                    }
                    else{

                        y[K*n+K/2-1] = mul_add(_hb2[ro],-y[K*(n-1)-1],x[K*n+K/2-1]);
                        y[K*n+K/2-1] = mul_add(_hb1[ro],-y[K*n-1],y[K*n+K/2-1]);
                    }
                }
            }

            _S.shift(y[N-2][N-1]);
            _S.shift(y[N-1][N-1]); 

            return y;
        }

        inline void gaussian_elimination_factors(){

            _f[0] = -_a2;
            _e[0] = -_a1;
            _h[0] = _f[0];
            _g[0] = _e[0];
            _fde[0] = _f[0]/_e[0];
            
            for (int n=1;n<R+1;n++){

                _d[n] = -_f[n-1]*_f[n-1]/_e[n-1];
                _c[n] = _e[n-1] - _f[n-1]/_e[n-1];
                _f[n] = -_e[n-1]*_d[n];
                _e[n] = _f[n-1] - _e[n-1]*_c[n];
                
                _h[n] = -_e[n-1]*_h[n-1];
                _g[n] = _f[n-1] - _e[n-1]*_g[n-1];

                _fde[n] = _f[n]/_e[n];
            }
        }

        inline void block_filtering_factors(){

            T h0[L]={0},h1[L]={0},h2[L]={0};
            V _h0;

            h0[0] = 1;
            h0[1] = -_e[R];

            for (int l=2;l<L;l++)
                h0[l] = -_e[R]*h0[l-1] - _f[R]*h0[l-2];

                h2[0] = _h[R]*h0[0];
                h1[0] = _g[R]*h0[0];
            
            for (int l = 1; l < L; ++l){
                h2[l] = _h[R]*h0[l];
                h1[l] = _g[R]*(h0[l] + _f[R]/_g[R]*h0[l-1]);
            }

            _h2.load(&h2[0]);
            _h1.load(&h1[0]);
            _h0.load(&h0[0]);

            // AVX2
            if constexpr (L == 8){ 
                for (auto l=0; l<L; l++){

                    _H[l] = _h0;
                    _h0 = permute8<-1,0,1,2,3,4,5,6>(_h0);
                }

            }

            // SSE
            if constexpr (L == 4){
                for (int l=0;l<L;l++){

                    _H[l] = _h0;
                    _h0 = permute4<-1,0,1,2>(_h0);
                }
            }
            
            // AVX512
            if constexpr (L == 16){ 
                for (auto l=0; l<L; l++){

                    _H[l] = _h0;
                    _h0 = permute16<-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14>(_h0);
                }
            }
        }


        inline void backward_factor(){

            T c[L],d[L];
            V h2,h1;

            for (int n=R;n>=1;n--){

                std::fill_n(c,L,_c[n]);
                std::fill_n(d,L,_d[n]);
                h2.load(&d[0]);
                h1.load(&c[0]);

                if (n == R){
    
                    if constexpr (L == 4){

                        _hb2_0[n-1] = blend4<4,0,0,0>(h1,_h[n-1]); 
                        _hb1_0[n-1] = blend4<4,0,0,0>(h2,_g[n-1]);
                    }
                    if constexpr (L == 8){
        
                        _hb2_0[n-1] = blend8<8,0,0,0,0,0,0,0>(h1,_h[n-1]);
                        _hb1_0[n-1] = blend8<8,0,0,0,0,0,0,0>(h2,_g[n-1]);
                    }
        
                    if constexpr (L == 16){
        
                        _hb2_0[n-1] = blend16<16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0>(h1,_h[n-1]);
                        _hb1_0[n-1] = blend16<16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0>(h2,_g[n-1]);
                    }
                }
                else{

                    _hb2[n-1] = h2; 
                    _hb1[n-1] = h1;

                    if constexpr (L == 4){

                        _hb2_0[n-1] = blend4<4,0,0,0>(h2,_h[n-1]);
                        _hb1_0[n-1] = blend4<4,0,0,0>(h1,_g[n-1]);
                    }

                    if constexpr (L == 8){

                        _hb2_0[n-1] = blend8<8,0,0,0,0,0,0,0>(h2,_h[n-1]);
                        _hb1_0[n-1] = blend8<8,0,0,0,0,0,0,0>(h1,_g[n-1]);
                    }

                    if constexpr (L == 16){

                        _hb2_0[n-1] = blend16<16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0>(h2,_h[n-1]);
                        _hb1_0[n-1] = blend16<16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0>(h1,_g[n-1]);
                    }
                }
            }
        }

        __attribute__((always_inline))
        inline std::array<V,N> operator()(std::array<V,N>& x){
            
            return backward(forward<0>(x));
        }
};

#endif

