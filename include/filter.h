#ifndef FILTER_H
#define FILTER_H 1

#include "series.h"
#include "permute.h"



template<typename V,size_t M,size_t N> class Filter{

    using T = decltype(std::declval<V>().extract(0));
    constexpr static int L = V::size(); 

    private:

        using Series_t = decltype(series_from_coeffs<T,V,N>(std::declval<const T (&)[M][5]>(), std::declval<const T (&)[M][4]>())); 
        Series_t _S;

    public:

        Filter(){};
        
        __attribute__((always_inline))
        Filter(const T (&coefs)[M][5], const T (&inits)[M][4]): _S(series_from_coeffs<T,V,N>(coefs, inits)){}; 

        

        // template<typename InputIt, typename OutputIt>
        // __attribute__((always_inline))
        // inline OutputIt operator()(InputIt first, InputIt last, OutputIt d_first) {
            
        //     std::array<V,N> x, y, x_T, y_T;

        //     while (first <= last - N*L){

        //         #pragma unroll
        //         for (auto n=0; n<N; n++) x[n].load(&*(first + n*L));  

        //         x_T = permuteV(x);
        //         y_T = _S(x_T);
        //         y = depermuteV(y_T);
               
        //         #pragma unroll
        //         for (auto n=0; n<N; n++) y[n].store(&*(d_first + n*L));

        //         first += N*L;
        //         d_first += N*L;

        //     }

        //     return d_first;
        // };


        // template<typename InputIt, typename OutputIt> 
        // __attribute__((always_inline))
        // inline OutputIt operator()(InputIt first, InputIt last, OutputIt d_first) {
            
        //     V x, y;

        //     while (first <= last - L) {

        //         x.load(&*first);  
            
        //         y = _S(x);

        //         y.store(&*d_first);

        //         first += L;
        //         d_first += L;
        //     }

        //     return d_first;
        // };


        template<typename InputIt, typename OutputIt> 
        __attribute__((always_inline))
        inline OutputIt operator()(InputIt first, InputIt last, OutputIt d_first) {
    
            while (first <= last - 1){
               
                *d_first = _S(*first);

                first += 1;
                d_first += 1;

            }

            return d_first;
        };

};


#endif // header guard 



