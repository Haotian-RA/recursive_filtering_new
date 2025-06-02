#ifndef PERMUTE_H
#define PERMUTE_H 1

#include "../src/vcl/vectorclass.h"
#include <array>



template<typename V,size_t N> 
__attribute__((always_inline))
inline std::array<V,N> permuteV(const std::array<V,N>& matrix) {

    std::array<V,N> matrix_T{};

    // SSE
    if constexpr (V::size() == 4) permuteV4<V,N>(matrix.data(), matrix_T.data());

    // AVX2
    if constexpr (V::size() == 8) permuteV8<V,N>(matrix.data(), matrix_T.data());

    // AVX512
    if constexpr (V::size() == 16) permuteV16<V,N>(matrix.data(), matrix_T.data());
    
    return matrix_T;
};


template<typename V,size_t N> 
__attribute__((always_inline))
inline std::array<V,N> depermuteV(const std::array<V,N>& matrix_T) {

    std::array<V,N> matrix{};

    // SSE
    if constexpr (V::size() == 4) depermuteV4<V,N>(matrix_T.data(), matrix.data());

    // AVX2
    if constexpr (V::size() == 8) depermuteV8<V,N>(matrix_T.data(), matrix.data());

    // AVX512
    if constexpr (V::size() == 16) depermuteV16<V,N>(matrix_T.data(), matrix.data());
    
    return matrix;
};


template<typename V,size_t N> 
__attribute__((always_inline))
inline void permuteV4(const V* matrix,V* matrix_T){

    V tmp[N];

    // swap bottom left and top right in matrix size L x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        tmp[2*n] = blend4<0,1,4,5>(matrix[n],matrix[n+N/2]);
        tmp[2*n+1] = blend4<2,3,6,7>(matrix[n],matrix[n+N/2]);
    }

    // swap bottom left and top right in sub-matrix size L/2 x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        matrix_T[2*n] = blend4<0,4,2,6>(tmp[n],tmp[n+N/2]);
        matrix_T[2*n+1] = blend4<1,5,3,7>(tmp[n],tmp[n+N/2]);
    }

}




template<typename V,size_t N> 
__attribute__((always_inline))
inline void permuteV8(const V* matrix,V* matrix_T){

    V tmp1[N],tmp2[N];

    // swap bottom left and top right in matrix size L x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        tmp1[2*n] = blend8<0,1,2,3,8,9,10,11>(matrix[n],matrix[n+N/2]);
        tmp1[2*n+1] = blend8<4,5,6,7,12,13,14,15>(matrix[n],matrix[n+N/2]);
    }

    // swap bottom left and top right in matrix size L/2 x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        tmp2[2*n] = blend8<0,1,8,9,4,5,12,13>(tmp1[n],tmp1[n+N/2]);
        tmp2[2*n+1] = blend8<2,3,10,11,6,7,14,15>(tmp1[n],tmp1[n+N/2]);
    }

    // swap bottom left and top right in sub-matrix size L/4 x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        matrix_T[2*n] = blend8<0,8,2,10,4,12,6,14>(tmp2[n],tmp2[n+N/2]);
        matrix_T[2*n+1] = blend8<1,9,3,11,5,13,7,15>(tmp2[n],tmp2[n+N/2]);
    }

}


template<typename V,size_t N> 
__attribute__((always_inline))
inline void permuteV16(const V* matrix,V* matrix_T){

    V tmp1[N],tmp2[N],tmp3[N];

    // swap bottom left and top right in matrix size L x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        tmp1[2*n] = blend16<0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23>(matrix[n],matrix[n+N/2]);
        tmp1[2*n+1] = blend16<8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31>(matrix[n],matrix[n+N/2]);
    }

    // swap bottom left and top right in matrix size L/2 x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        tmp2[2*n] = blend16<0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27>(tmp1[n],tmp1[n+N/2]);
        tmp2[2*n+1] = blend16<4,5,6,7,20,21,22,23,12,13,14,15,28,29,30,31>(tmp1[n],tmp1[n+N/2]);
    }

    // swap bottom left and top right in sub-matrix size L/4 x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        tmp3[2*n] = blend16<0,1,16,17,4,5,20,21,8,9,24,25,12,13,28,29>(tmp2[n],tmp2[n+N/2]);
        tmp3[2*n+1] = blend16<2,3,18,19,6,7,22,23,10,11,26,27,14,15,30,31>(tmp2[n],tmp2[n+N/2]);
    }

    // swap bottom left and top right in sub-matrix size L/8 x N
    #pragma unroll
    for (size_t n=0;n<N/2;n++){

        matrix_T[2*n] = blend16<0,16,2,18,4,20,6,22,8,24,10,26,12,28,14,30>(tmp3[n],tmp3[n+N/2]);
        matrix_T[2*n+1] = blend16<1,17,3,19,5,21,7,23,9,25,11,27,13,29,15,31>(tmp3[n],tmp3[n+N/2]);
    }

}


template<typename V,size_t N> 
__attribute__((always_inline))
inline void depermuteV4(const V* matrix_T,V* matrix){

    V tmp[N];

    // swap bottom left and top right in sub-matrix size L x L
    #pragma unroll
    for (size_t l=0;l<2;l++){ 
        #pragma unroll
        for (size_t n=0;n<N/4;n++){ 

            tmp[l*N/4+n] = blend4<0,1,4,5>(matrix_T[l+4*n],matrix_T[l+2+4*n]);
            tmp[l*N/4+n+N/2] = blend4<2,3,6,7>(matrix_T[l+4*n],matrix_T[l+2+4*n]);
        }
    }

    // swap bottom left and top right in sub-matrix size L/2 x N/2
    #pragma unroll
    for (size_t l=0;l<2;l++){
        #pragma unroll
        for (size_t n=0;n<N/4;n++){

            matrix[n+l*N/2] = blend4<0,4,2,6>(tmp[n+l*N/2],tmp[n+l*N/2+N/4]);
            matrix[n+l*N/2+N/4] = blend4<1,5,3,7>(tmp[n+l*N/2],tmp[n+l*N/2+N/4]);
        }
    }

}


template<typename V,size_t N>
__attribute__((always_inline))
inline void depermuteV8(const V* matrix_T,V* matrix){

    V tmp1[N],tmp2[N];

    // swap bottom left and top right in sub-matrix size L x L
    #pragma unroll
    for (size_t l=0;l<4;l++){
        #pragma unroll
        for (size_t n=0;n<N/8;n++){ 

            tmp1[l*N/8+n] = blend8<0,1,2,3,8,9,10,11>(matrix_T[l+8*n],matrix_T[l+4+8*n]);
            tmp1[l*N/8+n+N/2] = blend8<4,5,6,7,12,13,14,15>(matrix_T[l+8*n],matrix_T[l+4+8*n]);
        }
    }

    // swap bottom left and top right in sub-matrix size L/2 x N/2
    #pragma unroll
    for (size_t l=0;l<2;l++){
        #pragma unroll
        for (size_t n=0;n<N/4;n++){

            tmp2[n+l*N/2] = blend8<0,1,8,9,4,5,12,13>(tmp1[n+l*N/2],tmp1[n+l*N/2+N/4]);
            tmp2[n+l*N/2+N/4] = blend8<2,3,10,11,6,7,14,15>(tmp1[n+l*N/2],tmp1[n+l*N/2+N/4]);
        }
    }

    // swap bottom left and top right in sub-matrix size L/2 x N/4
    #pragma unroll  
    for (size_t l=0;l<4;l++){
        #pragma unroll
        for (size_t n=0;n<N/8;n++){

            matrix[n+l*N/4] = blend8<0,8,2,10,4,12,6,14>(tmp2[n+l*N/4],tmp2[n+l*N/4+N/8]);
            matrix[n+l*N/4+N/8] = blend8<1,9,3,11,5,13,7,15>(tmp2[n+l*N/4],tmp2[n+l*N/4+N/8]);
        }
    }

}


template<typename V,size_t N> 
__attribute__((always_inline))
inline void depermuteV16(const V* matrix_T,V* matrix){

    V tmp1[N],tmp2[N],tmp3[N];

    // swap bottom left and top right in sub-matrix size L x L
    #pragma unroll  
    for (size_t l=0;l<8;l++){ 
        #pragma unroll  
        for (size_t n=0;n<N/16;n++){    

            tmp1[l*N/16+n] = blend16<0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23>(matrix_T[l+16*n],matrix_T[l+8+16*n]);
            tmp1[l*N/16+n+N/2] = blend16<8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31>(matrix_T[l+16*n],matrix_T[l+8+16*n]);
        }
    }

    // swap bottom left and top right in sub-matrix size L/2 x N/2
    #pragma unroll  
    for (size_t l=0;l<2;l++){
        #pragma unroll  
        for (size_t n=0;n<N/4;n++){

            tmp2[n+l*N/2] = blend16<0,1,2,3,16,17,18,19,8,9,10,11,24,25,26,27>(tmp1[n+l*N/2],tmp1[n+l*N/2+N/4]);
            tmp2[n+l*N/2+N/4] = blend16<4,5,6,7,20,21,22,23,12,13,14,15,28,29,30,31>(tmp1[n+l*N/2],tmp1[n+l*N/2+N/4]);
        }
    }

    // swap bottom left and top right in sub-matrix size L/2 x N/4
    #pragma unroll  
    for (size_t l=0;l<4;l++){
        #pragma unroll  
        for (size_t n=0;n<N/8;n++){

            tmp3[n+l*N/4] = blend16<0,1,16,17,4,5,20,21,8,9,24,25,12,13,28,29>(tmp2[n+l*N/4],tmp2[n+l*N/4+N/8]);
            tmp3[n+l*N/4+N/8] = blend16<2,3,18,19,6,7,22,23,10,11,26,27,14,15,30,31>(tmp2[n+l*N/4],tmp2[n+l*N/4+N/8]);
        }
    }

    // swap bottom left and top right in sub-matrix size L/2 x N/8
    #pragma unroll  
    for (size_t l=0;l<8;l++){
        #pragma unroll  
        for (size_t n=0;n<N/16;n++){

            matrix[n+l*N/8] = blend16<0,16,2,18,4,20,6,22,8,24,10,26,12,28,14,30>(tmp3[n+l*N/8],tmp3[n+l*N/8+N/16]);
            matrix[n+l*N/8+N/16] = blend16<1,17,3,19,5,21,7,23,9,25,11,27,13,29,15,31>(tmp3[n+l*N/8],tmp3[n+l*N/8+N/16]);
        }
    }

}




#endif






