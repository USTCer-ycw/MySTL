//
//  TinySTL_construct.h
//  MyStl
//
//  Created by 姚传望 on 2020/4/26.
//  Copyright © 2020 姚传望. All rights reserved.
//

#ifndef TinySTL_construct_h
#define TinySTL_construct_h
#include <new>
#include "TinySTL_type_traits.h"
#include "TinySTL_uninitialized.h"
namespace TinySTL{

template<class T>
inline void destroy(T* pointer){
    pointer->~T();
}

template<class T1,class T2>
inline void construct(T1* p,const T2& value) {
    new (p) T1(value);//placement new，p指向的地方分配value
}
//如果含有non-trivial destructor
template <class ForwardIterator>
inline void _destroy_aux(ForwardIterator first, ForwardIterator last,__false_type){
    while(first<last){
        destroy(&*first);
        ++first;
    }
}

template <class ForwardIterator>
inline void _destroy_aux(ForwardIterator,ForwardIterator,__true_type){}//无关痛痒，do_nothing

template <class ForwardIterator,class T>
inline void _destroy(ForwardIterator first,ForwardIterator last,T*){
    typedef typename __type_traits<T>::has_trivial_destructor trival_destructor;
    _destroy_aux(first, last, trival_destructor());//萃取出false_type还是_true_type，if是true_type，则do_nothing，因为这时是trivial_destructor，无关痛痒
}

template <class ForwardIterator>
inline void destroy(ForwardIterator first,ForwardIterator last){
    _destroy(first,last,value_type(first));//特化
}

inline void destroy(char*,char*){}//do_nothing
inline void destroy(wchar_t*,wchar_t*){}//do_nothing

}


#endif /* TinySTL_construct_h */
