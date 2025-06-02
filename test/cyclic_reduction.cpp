#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../src/doctest.h" 
#include "../include/cyclic_reduction.h" 
#include "../include/fir_cores.h" 
#include "../include/permute.h"
#include "../include/shift_reg.h"
#include <numeric>
#include <array>



#ifdef DOCTEST_LIBRARY_INCLUDED


TEST_SUITE_BEGIN("Cyclic Reduction:");



TEST_CASE("Cyclic Reduction V4:"){

    using V = Vec4f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;


    std::array<T,N*L> data,data_out;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in,out,in_T,out_T,tmp_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    CyclicReduction<V,N> CR(a1,a2,yi1,yi2); // test first constructor

    for (int n=0;n<N*L;n++){

        if (n == 0) data_out[0] = data[0] + b2*xi2 + b1*xi1 + a2*yi2 + a1*yi1;
        else if (n == 1) data_out[1] = data[1] + b2*xi1 + b1*data[0] + a2*yi1 + a1*data_out[0];
        else data_out[n] = data[n] + b2*data[n-2] + b1*data[n-1] + a2*data_out[n-2] + a1*data_out[n-1];
    }

    in_T = permuteV(in);
    tmp_T = F(in_T);
    out_T = CR(tmp_T);
    out = depermuteV(out_T);
    
    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) 
        CHECK(data_out[r] == doctest::Approx(res[r]));
    
}



TEST_CASE("Cyclic Reduction V8:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;


    std::array<T,N*L> data,data_out;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in,out,in_T,out_T,tmp_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    CyclicReduction<V,N> CR(a1,a2,yi1,yi2); // test first constructor

    for (int n=0;n<N*L;n++){

        if (n == 0) data_out[0] = data[0] + b2*xi2 + b1*xi1 + a2*yi2 + a1*yi1;
        else if (n == 1) data_out[1] = data[1] + b2*xi1 + b1*data[0] + a2*yi1 + a1*data_out[0];
        else data_out[n] = data[n] + b2*data[n-2] + b1*data[n-1] + a2*data_out[n-2] + a1*data_out[n-1];
    }

    in_T = permuteV(in);
    tmp_T = F(in_T);
    out_T = CR(tmp_T);
    out = depermuteV(out_T);
    
    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) 
        CHECK(data_out[r] == doctest::Approx(res[r]));
    
}


TEST_CASE("Cyclic Reduction V16:"){

    using V = Vec16f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;


    std::array<T,N*L> data,data_out;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in,out,in_T,out_T,tmp_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    CyclicReduction<V,N> CR(a1,a2,yi1,yi2); // test first constructor

    for (int n=0;n<N*L;n++){

        if (n == 0) data_out[0] = data[0] + b2*xi2 + b1*xi1 + a2*yi2 + a1*yi1;
        else if (n == 1) data_out[1] = data[1] + b2*xi1 + b1*data[0] + a2*yi1 + a1*data_out[0];
        else data_out[n] = data[n] + b2*data[n-2] + b1*data[n-1] + a2*data_out[n-2] + a1*data_out[n-1];
    }

    in_T = permuteV(in);
    tmp_T = F(in_T);
    out_T = CR(tmp_T);
    out = depermuteV(out_T);
    
    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) 
        CHECK(data_out[r] == doctest::Approx(res[r]));
    
}


TEST_SUITE_END();

#endif







