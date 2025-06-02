#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../src/doctest.h"   
#include "../include/block_filtering.h" 
#include <numeric>
#include <array>



#ifdef DOCTEST_LIBRARY_INCLUDED


TEST_SUITE_BEGIN("block_filtering:");


TEST_CASE("block filtering V4:"){

    using V = Vec4f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in,out,out_tmp;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    BlockFiltering<V> BF(b1,b2,a1,a2,xi1,xi2,yi1,yi2); 

    std::array<T,N*L> y;

    for (int n=0;n<N*L;n++){

        if (n == 0) y[0] = data[0] + b2*xi2 + b1*xi1 + a2*yi2 + a1*yi1;
        else if (n == 1) y[1] = data[1] + b2*xi1 + b1*data[0] + a2*yi1 + a1*y[0];
        else y[n] = data[n] + b2*data[n-2] + b1*data[n-1] + a2*y[n-2] + a1*y[n-1];
    }

    for (int n=0;n<N;n++)  out[n] = BF(in[n]);
    

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(y[r] == doctest::Approx(res[r]));

}


TEST_CASE("block filtering V8:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in,out,out_tmp;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    BlockFiltering<V> BF(b1,b2,a1,a2,xi1,xi2,yi1,yi2); 

    std::array<T,N*L> y;

    for (int n=0;n<N*L;n++){

        if (n == 0) y[0] = data[0] + b2*xi2 + b1*xi1 + a2*yi2 + a1*yi1;
        else if (n == 1) y[1] = data[1] + b2*xi1 + b1*data[0] + a2*yi1 + a1*y[0];
        else y[n] = data[n] + b2*data[n-2] + b1*data[n-1] + a2*y[n-2] + a1*y[n-1];
    }

    for (int n=0;n<N;n++)  out[n] = BF(in[n]);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(y[r] == doctest::Approx(res[r]));

}



TEST_CASE("block filtering V16:"){

    using V = Vec16f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in,out,out_tmp;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    BlockFiltering<V> BF(b1,b2,a1,a2,xi1,xi2,yi1,yi2); 

    std::array<T,N*L> y;

    for (int n=0;n<N*L;n++){

        if (n == 0) y[0] = data[0] + b2*xi2 + b1*xi1 + a2*yi2 + a1*yi1;
        else if (n == 1) y[1] = data[1] + b2*xi1 + b1*data[0] + a2*yi1 + a1*y[0];
        else y[n] = data[n] + b2*data[n-2] + b1*data[n-1] + a2*y[n-2] + a1*y[n-1];
    }

    for (int n=0;n<N;n++)  out[n] = BF(in[n]);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(y[r] == doctest::Approx(res[r]));

}


TEST_SUITE_END();

#endif







