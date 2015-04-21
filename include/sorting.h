#ifndef DFREESE_SORTING_H
#define DFREESE_SORTING_H

#include <deque>
#include <vector>

template <class T> 
int insertion_sort(T & array) {
    for (typename T::size_type ii = 1; ii < array.size(); ii++) {
        for (typename T::size_type kk = ii; (kk > 0) && (array[kk] < array[kk-1]); kk--) {
            typename T::value_type temp = array[kk];
            array[kk] = array[kk - 1];
            array[kk - 1] = temp;
        }
    }
    return(0);
}

/*! Takes a function as an argument, where the function takes two arguments and
 * returns true if the first argument is less than the second argument.
 */
template <class T>
int insertion_sort(T & array, bool (*f) (typename T::value_type, typename T::value_type)) {
    for (typename T::size_type ii = 1; ii < array.size(); ii++) {
        for (typename T::size_type kk = ii;
             (0 < kk) && f(array[kk],array[kk-1]);
             kk--)
        {
            typename T::value_type temp = array[kk];
            array[kk] = array[kk - 1];
            array[kk - 1] = temp;
        }
    }
    return(0);
}

template <class T, class K> 
int insertion_sort_with_key(T & array, K & key) {
    if (array.size() != key.size()) {
        return(-1);
    }
    for (typename T::size_type ii = 1; ii < array.size(); ii++) {
        for (typename T::size_type kk = ii; (kk > 0) && (array[kk] < array[kk-1]); kk--) {
            typename T::value_type temp = array[kk];
            array[kk] = array[kk - 1];
            array[kk - 1] = temp;
            typename K::value_type temp_key = key[kk];
            key[kk] = key[kk - 1];
            key[kk - 1] = temp_key;
        }
    }
    return(0);
}


#endif /* DFREESE_SORTING_H */
