#ifndef SERIES_H
#define SERIES_H 1

#include <array>
#include <tuple>
#include "iir_cores.h"

template<typename... Types> class Series{

    private:
        
        std::tuple<Types...> _t; 

        template<int i, typename U> 
        __attribute__((always_inline))
        inline U _proc(const U& x) {

            if constexpr (i >= std::tuple_size<decltype(_t)>::value) {
             
                return x;          
            } else {

                U r = std::get<i>(_t)(x);
                return _proc<i+1>(r);  
            };
        };
        
    public:

        Series(){};

        Series(Types...types): _t(types...){};

        template<typename U> 
        __attribute__((always_inline))
        inline U operator()(const U& x) { 
            return _proc<0>(x); 
        };

};


template <class T> struct unwrap_refwrapper { 
    using type = T; 
};

template <class T> struct unwrap_refwrapper<std::reference_wrapper<T>> { 
    using type = T&; 
};

template <class T> using unwrap_decay_t = typename unwrap_refwrapper<typename std::decay<T>::type>::type;

template <class... Types> constexpr Series<unwrap_decay_t<Types>...> make_series(Types&&... args) {
    return Series<unwrap_decay_t<Types>...>(std::forward<Types>(args)...);
};

template<typename V,size_t N, typename Array1, typename Array2, std::size_t... I>
__attribute__((always_inline))
auto make_series_from_coeffs(const Array1& coefs, const Array2& inits, std::index_sequence<I...>) {
    using Class = IirCoreOrderTwo<V,N>;
    return make_series(Class(coefs[I], inits[I])...); 
};

template<typename T, typename V, size_t N, size_t M, typename indices = std::make_index_sequence<M>>
__attribute__((always_inline))
auto series_from_coeffs(const T (&coefs)[M][5], const T (&inits)[M][4]={0}) { 
    return make_series_from_coeffs<V,N>(coefs, inits, indices{});
};




#endif 









































