#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../src/doctest.h"   
#include "../include/permute.h" 
#include <numeric>

#ifdef DOCTEST_LIBRARY_INCLUDED

TEST_SUITE_BEGIN("permute:");


TEST_CASE("(de)permute V4:"){

    using V = Vec4f;
    constexpr int L = V::size();
    constexpr int N = 8*L;
    using T = float;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    V in[N];
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    V out_T[N],out[N];
    permuteV4<V,N>(in,out_T);
    depermuteV4<V,N>(out_T,out);


    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);   

    for (auto c=0; c<N; c++)
        for (auto r=0; r<L; r++) CHECK(res[L*c+r] == L*c+r);

    
    
}


TEST_CASE("(de)permute V8:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = 8*L;
    using T = float;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    V in[N];
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    V out_T[N],out[N];
    permuteV8<V,N>(in,out_T);
    depermuteV8<V,N>(out_T,out);


    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);   

    for (auto c=0; c<N; c++)
        for (auto r=0; r<L; r++) CHECK(res[L*c+r] == L*c+r);

    
    
}


TEST_CASE("(de)permute V16:"){

    using V = Vec16f;
    constexpr int L = V::size();
    constexpr int N = 8*L;
    using T = float;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    V in[N];
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    V out_T[N],out[N];
    permuteV16<V,N>(in,out_T);
    depermuteV16<V,N>(out_T,out);


    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);   

    for (auto c=0; c<N; c++)
        for (auto r=0; r<L; r++) CHECK(res[L*c+r] == L*c+r);

    
    
}


TEST_CASE("encapsulated (de)permute V4:"){

    using V = Vec4f;
    constexpr int L = V::size();
    constexpr int N = 8*L;
    using T = float;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<V,N> out_T,out;
    out_T = permuteV(in);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);   

    for (auto c=0; c<N; c++)
        for (auto r=0; r<L; r++) CHECK(res[L*c+r] == L*c+r);

}


TEST_CASE("encapsulated (de)permute V8:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = 8*L;
    using T = float;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<V,N> out_T,out;
    out_T = permuteV(in);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);   

    for (auto c=0; c<N; c++)
        for (auto r=0; r<L; r++) CHECK(res[L*c+r] == L*c+r);

}

TEST_CASE("encapsulated (de)permute V16:"){

    using V = Vec16f;
    constexpr int L = V::size();
    constexpr int N = 8*L;
    using T = float;

    std::array<T,N*L> data;
    std::iota(data.begin(), data.end(), 0);

    std::array<V,N> in;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    std::array<V,N> out_T,out;
    out_T = permuteV(in);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);   

    for (auto c=0; c<N; c++)
        for (auto r=0; r<L; r++) CHECK(res[L*c+r] == L*c+r);

}

TEST_SUITE_END();

#endif







