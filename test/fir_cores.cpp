#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../src/doctest.h"   
#include "../include/fir_cores.h" 
#include "../include/permute.h"
#include <numeric>
#include <array>



#ifdef DOCTEST_LIBRARY_INCLUDED


TEST_SUITE_BEGIN("fir_cores:");


TEST_CASE("multi-block filtering V4:"){

    using V = Vec4f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, xi2 = 1, xi1 = 2;

    std::array<T,N*L> data{0};
    data[0] = 1; // pass an impulse response 
    std::array<T,N*L> data_out=data;

    for (int n=0;n<N*L;n++){

        if (n == 0) data_out[0] = data[0] + b2*xi2 + b1*xi1;
        else if (n == 1) data_out[1] = data[1] + b2*xi1 + b1*data[0];
        else data_out[n] = data[n] + b2*data[n-2] + b1*data[n-1];
    }

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); // test first constructor

    std::array<V,N> in,in_T,out_T,out;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    in_T = permuteV(in);
    out_T = F(in_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(data_out[r] == doctest::Approx(res[r]));
}



TEST_CASE("multi-block filtering V8:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, xi2 = 1, xi1 = 2;

    std::array<T,N*L> data{0};
    data[0] = 1; // pass an impulse response 
    std::array<T,N*L> data_out=data;

    for (int n=0;n<N*L;n++){

        if (n == 0) data_out[0] = data[0] + b2*xi2 + b1*xi1;
        else if (n == 1) data_out[1] = data[1] + b2*xi1 + b1*data[0];
        else data_out[n] = data[n] + b2*data[n-2] + b1*data[n-1];
    }

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); // test first constructor

    std::array<V,N> in,in_T,out_T,out;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    in_T = permuteV(in);
    out_T = F(in_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(data_out[r] == doctest::Approx(res[r]));
}


TEST_CASE("multi-block filtering V16:"){

    using V = Vec16f;
    constexpr int L = V::size();
    constexpr int N = L;
    using T = float;

    float b2 = 2, b1 = 1, xi2 = 1, xi1 = 2;

    std::array<T,N*L> data{0};
    data[0] = 1; // pass an impulse response 
    std::array<T,N*L> data_out=data;

    for (int n=0;n<N*L;n++){

        if (n == 0) data_out[0] = data[0] + b2*xi2 + b1*xi1;
        else if (n == 1) data_out[1] = data[1] + b2*xi1 + b1*data[0];
        else data_out[n] = data[n] + b2*data[n-2] + b1*data[n-1];
    }

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); // test first constructor

    std::array<V,N> in,in_T,out_T,out;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    in_T = permuteV(in);
    out_T = F(in_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(data_out[r] == doctest::Approx(res[r]));
}




TEST_CASE("multi-block filtering - block size 2L:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = 2*L; 
    using T = float;

    float b2 = 2, b1 = 1, xi2 = 1, xi1 = 2;

    std::array<T,N*L> data{0};
    data[0] = 1; // pass an impulse response 
    std::array<T,N*L> data_out=data;

    for (int n=0;n<N*L;n++){

        if (n == 0) data_out[0] = data[0] + b2*xi2 + b1*xi1;
        else if (n == 1) data_out[1] = data[1] + b2*xi1 + b1*data[0];
        else data_out[n] = data[n] + b2*data[n-2] + b1*data[n-1];
    }

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); // test first constructor

    std::array<V,N> in,in_T,out_T,out;
    for (auto n=0; n<N; n++) in[n].load(&data[n*L]);

    in_T = permuteV(in);
    out_T = F(in_T);
    out = depermuteV(out_T);

    std::array<T,N*L> res;
    for (auto n=0; n<N; n++) out[n].store(&res[n*L]);       
    
    for (auto r=0; r<N*L; r++) CHECK(data_out[r] == doctest::Approx(res[r]));
}


TEST_CASE("multi-block filtering - streaming:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = 2*L; // 16
    constexpr int vector_size = 1024;
    constexpr int K = vector_size/L;
    using T = float;

    float b2 = 2, b1 = 1, xi2 = 1, xi1 = 2;

    std::array<T,vector_size> data{0};
    data[0] = 1; // pass an impulse response 
    std::array<T,vector_size> data_out;

    for (int n=0;n<vector_size;n++){

        if (n == 0) data_out[0] = data[0] + b2*xi2 + b1*xi1;
        else if (n == 1) data_out[1] = data[1] + b2*xi1 + b1*data[0];
        else data_out[n] = data[n] + b2*data[n-2] + b1*data[n-1];
    }

    FirCoreOrderTwo<V,N> F(b1,b2,xi1,xi2); // test first constructor

    std::array<V,K> in_total,out_total;
    for (auto n=0; n<K; n++) in_total[n].load(&data[n*L]);

    std::array<V,N> in,in_T,out_T,out;

    for (int i=0;i<K/N;i++){
        for (int j=0;j<N;j++){
            in[j] = in_total[j+i*N];
        }
        in_T = permuteV(in);
        out_T = F(in_T);
        out = depermuteV(out_T);
        for (int l=0;l<N;l++){
            out_total[l+i*N] = out[l];
        }
    }

    std::array<T,vector_size> res;
    for (auto n=0; n<K; n++) out_total[n].store(&res[n*L]);       
    
    for (auto r=0; r<vector_size; r++) CHECK(data_out[r] == doctest::Approx(res[r]));
}


TEST_CASE("4th order multi-block filtering - streaming:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = 2*L; // 16
    constexpr int vector_size = 1024*16;
    constexpr int K = vector_size/L;
    constexpr int M = 2; // 4th order
    using T = float;

    float b2 = 2, b1 = 1, xi2 = 1, xi1 = 2;

    std::array<T,vector_size> data{0};
    data[0] = 1; // pass an impulse response 
    std::array<T,vector_size> data_out=data,tmp;

    for (int round=0;round<M;round++){
        for (int n=0;n<vector_size;n++){

            if (n == 0) tmp[0] = data_out[0] + b2*xi2 + b1*xi1;
            else if (n == 1) tmp[1] = data_out[1] + b2*xi1 + b1*data_out[0];
            else tmp[n] = data_out[n] + b2*data_out[n-2] + b1*data_out[n-1];
        }
        data_out = tmp;
    }

    FirCoreOrderTwo<V,N> F1(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F2(b1,b2,xi1,xi2); // test first constructor

    std::array<V,K> in_total,out_total;
    for (auto n=0; n<K; n++) in_total[n].load(&data[n*L]);

    std::array<V,N> in,in_T,out_T,out;

    for (int i=0;i<K/N;i++){
        for (int j=0;j<N;j++){
            in[j] = in_total[j+i*N];
        }
        in_T = permuteV(in);
        out_T = F2(F1(in_T));
        out = depermuteV(out_T);
        for (int l=0;l<N;l++){
            out_total[l+i*N] = out[l];
        }
    }

    std::array<T,vector_size> res;
    for (auto n=0; n<K; n++) out_total[n].store(&res[n*L]);       
    
    for (auto r=0; r<vector_size; r++) CHECK(data_out[r] == doctest::Approx(res[r]));
}

TEST_CASE("8th order multi-block filtering - streaming:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = 2*L; // 16
    constexpr int vector_size = 1024*16;
    constexpr int K = vector_size/L;
    constexpr int M = 4; // 8th order
    using T = float;

    float b2 = 2, b1 = 1, xi2 = 1, xi1 = 2;

    std::array<T,vector_size> data{0};
    data[0] = 1; // pass an impulse response 
    std::array<T,vector_size> data_out=data,tmp;

    for (int round=0;round<M;round++){
        for (int n=0;n<vector_size;n++){

            if (n == 0) tmp[0] = data_out[0] + b2*xi2 + b1*xi1;
            else if (n == 1) tmp[1] = data_out[1] + b2*xi1 + b1*data_out[0];
            else tmp[n] = data_out[n] + b2*data_out[n-2] + b1*data_out[n-1];
        }
        data_out = tmp;
    }

    FirCoreOrderTwo<V,N> F1(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F2(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F3(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F4(b1,b2,xi1,xi2); // test first constructor

    std::array<V,K> in_total,out_total;
    for (auto n=0; n<K; n++) in_total[n].load(&data[n*L]);

    std::array<V,N> in,in_T,out_T,out;

    for (int i=0;i<K/N;i++){
        for (int j=0;j<N;j++){
            in[j] = in_total[j+i*N];
        }
        in_T = permuteV(in);
        out_T = F4(F3(F2(F1(in_T))));
        out = depermuteV(out_T);
        for (int l=0;l<N;l++){
            out_total[l+i*N] = out[l];
        }
    }

    std::array<T,vector_size> res;
    for (auto n=0; n<K; n++) out_total[n].store(&res[n*L]);       
    
    for (auto r=0; r<vector_size; r++) CHECK(data_out[r] == doctest::Approx(res[r]));
}


TEST_CASE("16th order multi-block filtering - streaming:"){

    using V = Vec8f;
    constexpr int L = V::size();
    constexpr int N = 2*L; // 16
    constexpr int vector_size = 1024*16;
    constexpr int K = vector_size/L;
    constexpr int M = 8; // 16th order
    using T = float;

    float b2 = 2, b1 = 1, xi2 = 1, xi1 = 2;

    std::array<T,vector_size> data{0};
    data[0] = 1; // pass an impulse response 
    std::array<T,vector_size> data_out=data,tmp;

    for (int round=0;round<M;round++){
        for (int n=0;n<vector_size;n++){

            if (n == 0) tmp[0] = data_out[0] + b2*xi2 + b1*xi1;
            else if (n == 1) tmp[1] = data_out[1] + b2*xi1 + b1*data_out[0];
            else tmp[n] = data_out[n] + b2*data_out[n-2] + b1*data_out[n-1];
        }
        data_out = tmp;
    }

    FirCoreOrderTwo<V,N> F1(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F2(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F3(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F4(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F5(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F6(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F7(b1,b2,xi1,xi2); // test first constructor
    FirCoreOrderTwo<V,N> F8(b1,b2,xi1,xi2); // test first constructor

    std::array<V,K> in_total,out_total;
    for (auto n=0; n<K; n++) in_total[n].load(&data[n*L]);

    std::array<V,N> in,in_T,out_T,out;

    for (int i=0;i<K/N;i++){
        for (int j=0;j<N;j++){
            in[j] = in_total[j+i*N];
        }
        in_T = permuteV(in);
        out_T = F8(F7(F6(F5(F4(F3(F2(F1(in_T))))))));
        out = depermuteV(out_T);
        for (int l=0;l<N;l++){
            out_total[l+i*N] = out[l];
        }
    }

    std::array<T,vector_size> res;
    for (auto n=0; n<K; n++) out_total[n].store(&res[n*L]);       
    
    for (auto r=0; r<vector_size; r++) CHECK(data_out[r] == doctest::Approx(res[r]));
}



TEST_SUITE_END();

#endif







