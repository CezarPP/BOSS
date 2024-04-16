/*
 * type_traits.h
 *
 *  Created on: 4/14/24.
 *      Author: Cezar PP
 */


namespace std {
    /// If the type T is a reference type, provides the member typedef type which is the type referred to by T.
    /// Otherwise type is T.
    template<class T>
    struct remove_reference {
        typedef T type;
    };
    template<class T>
    struct remove_reference<T &> {
        typedef T type;
    };
    template<class T>
    struct remove_reference<T &&> {
        typedef T type;
    };

    template<class T>
    using remove_reference_t = typename remove_reference<T>::type;

}