#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../src/doctest.h"   
#include "../include/ph_decompos.h" 
#include "../include/fir_cores.h"
#include "../include/permute.h"
#include <numeric>
#include <array>



#ifdef DOCTEST_LIBRARY_INCLUDED


TEST_SUITE_BEGIN("ph decomposition:");

TEST_CASE("particular solution V4:"){

    using V = Vec4f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    PartSolutionV<V,N> PS(a1,a2); // test first constructor

    std::array<V,N> in,in_T,out,out_T,tmp_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<T,N*L> v,w;

    for (int n=0;n<N*L;n++){

        if (n == 0) v[0] = data[0] + b1*xi1 + b2*xi2;
        else if (n == 1) v[1] = data[1] + b1*data[0] + b2*xi1;
        else v[n] = data[n] + b1*data[n-1] + b2*data[n-2];
    }

    // block particular solution has the first block of indices l*N block 
    // and they don't add particular solution.
    for (int l=0;l<L;l++){
        for (int n=0;n<N;n++){

            if (n == 0) w[l*N] = v[l*N]; 
            else if (n == 1) w[l*N+1] = v[l*N+1] + a1*w[l*N];
            else w[l*N+n] = v[l*N+n] + a1*w[l*N+n-1] + a2*w[l*N+n-2];
        }
    }

    in_T = permuteV(in);
    tmp_T = F(in_T);
    out_T = PS(tmp_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(w[r] == doctest::Approx(res[r]));


}



TEST_CASE("particular solution V8:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    PartSolutionV<V,N> PS(a1,a2); // test first constructor

    std::array<V,N> in,in_T,out,out_T,tmp_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<T,N*L> v,w;

    for (int n=0;n<N*L;n++){

        if (n == 0) v[0] = data[0] + b1*xi1 + b2*xi2;
        else if (n == 1) v[1] = data[1] + b1*data[0] + b2*xi1;
        else v[n] = data[n] + b1*data[n-1] + b2*data[n-2];
    }

    // block particular solution has the first block of indices l*N block 
    // and they don't add particular solution.
    for (int l=0;l<L;l++){
        for (int n=0;n<N;n++){

            if (n == 0) w[l*N] = v[l*N]; 
            else if (n == 1) w[l*N+1] = v[l*N+1] + a1*w[l*N];
            else w[l*N+n] = v[l*N+n] + a1*w[l*N+n-1] + a2*w[l*N+n-2];
        }
    }

    in_T = permuteV(in);
    tmp_T = F(in_T);
    out_T = PS(tmp_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(w[r] == doctest::Approx(res[r]));


}


TEST_CASE("particular solution V16:"){

    using V = Vec16f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    PartSolutionV<V,N> PS(a1,a2); // test first constructor

    std::array<V,N> in,in_T,out,out_T,tmp_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<T,N*L> v,w;

    for (int n=0;n<N*L;n++){

        if (n == 0) v[0] = data[0] + b1*xi1 + b2*xi2;
        else if (n == 1) v[1] = data[1] + b1*data[0] + b2*xi1;
        else v[n] = data[n] + b1*data[n-1] + b2*data[n-2];
    }

    // block particular solution has the first block of indices l*N block 
    // and they don't add particular solution.
    for (int l=0;l<L;l++){
        for (int n=0;n<N;n++){

            if (n == 0) w[l*N] = v[l*N]; 
            else if (n == 1) w[l*N+1] = v[l*N+1] + a1*w[l*N];
            else w[l*N+n] = v[l*N+n] + a1*w[l*N+n-1] + a2*w[l*N+n-2];
        }
    }

    in_T = permuteV(in);
    tmp_T = F(in_T);
    out_T = PS(tmp_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(w[r] == doctest::Approx(res[r]));


}


TEST_CASE("homogeneous solutions V4:"){

    using V = Vec4f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    PartSolutionV<V,N> PS(a1,a2); // test first constructor
    HomoSolutionV<V,N> HS(a1,a2,yi1,yi2); // test first constructor

    std::array<V,N> in,in_T,out,out_T,tmp1_T,tmp2_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<T,N*L> y;

    for (int n=0;n<N*L;n++){

        if (n == 0) y[0] = data[0] + b1*xi1 + b2*xi2 + a1*yi1 + a2*yi2;
        else if (n == 1) y[1] = data[1] + b1*data[0] + b2*xi1 + a1*y[0] + a2*yi1;
        else y[n] = data[n] + b1*data[n-1] + b2*data[n-2] + a1*y[n-1] + a2*y[n-2];
    }

    in_T = permuteV(in);
    tmp1_T = F(in_T);
    tmp2_T = PS(tmp1_T);
    out_T = HS(tmp2_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(y[r] == doctest::Approx(res[r]));


}



TEST_CASE("homogeneous solutions V8:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = 16;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    PartSolutionV<V,N> PS(a1,a2); // test first constructor
    HomoSolutionV<V,N> HS(a1,a2,yi1,yi2); // test first constructor

    std::array<V,N> in,in_T,out,out_T,tmp1_T,tmp2_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<T,N*L> y;

    for (int n=0;n<N*L;n++){

        if (n == 0) y[0] = data[0] + b1*xi1 + b2*xi2 + a1*yi1 + a2*yi2;
        else if (n == 1) y[1] = data[1] + b1*data[0] + b2*xi1 + a1*y[0] + a2*yi1;
        else y[n] = data[n] + b1*data[n-1] + b2*data[n-2] + a1*y[n-1] + a2*y[n-2];
    }

    in_T = permuteV(in);
    tmp1_T = F(in_T);
    tmp2_T = PS(tmp1_T);
    out_T = HS(tmp2_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(y[r] == doctest::Approx(res[r]));


}

TEST_CASE("homogeneous solutions V16:"){

    using V = Vec16f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, a1 = 0.4, a2 = 0.5, xi2 = 1, xi1 = 2, yi1 = -3, yi2 = -5;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); 
    PartSolutionV<V,N> PS(a1,a2); // test first constructor
    HomoSolutionV<V,N> HS(a1,a2,yi1,yi2); // test first constructor

    std::array<V,N> in,in_T,out,out_T,tmp1_T,tmp2_T;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<T,N*L> y;

    for (int n=0;n<N*L;n++){

        if (n == 0) y[0] = data[0] + b1*xi1 + b2*xi2 + a1*yi1 + a2*yi2;
        else if (n == 1) y[1] = data[1] + b1*data[0] + b2*xi1 + a1*y[0] + a2*yi1;
        else y[n] = data[n] + b1*data[n-1] + b2*data[n-2] + a1*y[n-1] + a2*y[n-2];
    }

    in_T = permuteV(in);
    tmp1_T = F(in_T);
    tmp2_T = PS(tmp1_T);
    out_T = HS(tmp2_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(y[r] == doctest::Approx(res[r]));


}



TEST_SUITE_END();





#endif



