//
//  TinySTL_iterator.h
//  MyStl
//
//  Created by 姚传望 on 2020/4/26.
//  Copyright © 2020 姚传望. All rights reserved.
//

#ifndef TinySTL_iterator_h
#define TinySTL_iterator_h
namespace TinySTL{
struct input_iterator_tag{};
struct output_iterator_tag{};
struct forward_iterator_tag: public input_iterator_tag{};
struct bidirectional_iterator_tag: public forward_iterator_tag{};
struct random_access_iterator_tag: public bidirectional_iterator_tag{};

template <class Category,class T,class Distance = ptrdiff_t,class Pointer = T*,class Reference = T&>
struct iterator{
    typedef Category    iterator_category;
    typedef T           value_type;
    typedef Distance    difference_type;
    typedef Pointer     pointer;
    typedef Reference   reference;
};

//萃取机
template <class Iterator>
struct iterator_traits {
    typedef typename Iterator::iterator_category    iterator_category;
    typedef typename Iterator::value_type           value_type;
    typedef typename Iterator::difference_type      difference_type;
    typedef typename Iterator::pointer              pointer;
    typedef typename Iterator::reference            reference;
};

//指针的偏特化
template <class T>
struct iterator_traits<T*> {
    typedef random_access_iterator_tag iterator_category;
    typedef T         value_type;
    typedef ptrdiff_t difference_type;
    typedef T*        pointer;
    typedef T&        reference;
};

//const指针偏特化
template <class T>
struct iterator_traits<const T*>{
    typedef random_access_iterator_tag iterator_category;
    typedef T           value_type;
    typedef ptrdiff_t   difference_type;
    typedef const T*    pointer;
    typedef const T&    reference;
};

template <class Iterator>
inline typename iterator_traits<Iterator>::iterator_category//返回值
iterator_categort(const Iterator&){
//    typedef typename iterator_traits<Iterator>::iterator_category category;
//    return category();
    return typename iterator_traits<Iterator>::iterator_category(); //category临时对象
}

template <class Iterator>
inline typename iterator_traits<Iterator>::difference_type*
distance_type(const Iterator&){
    return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
}

template <class Iterator>
inline typename iterator_traits<Iterator>::value_type*
value_type(const Iterator&){
    return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

template <class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
_distance(InputIterator first,InputIterator last,input_iterator_tag){
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while(first!=last){
        ++first; ++n;
    }
    return n;
}

template<class RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
_difference(RandomAccessIterator first,RandomAccessIterator last,random_access_iterator_tag){
    return last - first;
}

template <class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first,InputIterator last){
    typedef typename iterator_traits<InputIterator>::iterator_category category;
    return _distance(first, last, category());
}

}
#endif /* TinySTL_iterator_h */
