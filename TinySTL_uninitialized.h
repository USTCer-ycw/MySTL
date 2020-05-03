//
//  TinySTL_uninitialized.h
//  MyStl
//
//  Created by 姚传望 on 2020/4/27.
//  Copyright © 2020 姚传望. All rights reserved.
//

#ifndef TinySTL_uninitialized_h
#define TinySTL_uninitialized_h
namespace TinySTL{

//基本数据类型直接调用copy
//返回空间的尾
template<class InputIterator,class ForwardIterator>
inline ForwardIterator _uninitialized_copy_aux(InputIterator first,InputIterator last,ForwardIterator result,__false_type){
    ForwardIterator cur = result;
    while(first!=last){
        construct(&*cur,*first);
        first++; cur++;
    }
    return cur;
}

//first到last的内容拷贝到result指向的地方
template<class InputIterator,class ForwardIterator>
inline ForwardIterator _uninitialized_copy_aux(InputIterator first,InputIterator last,ForwardIterator result,__true_type){
    return std::copy(first,last,result);
}

template<class InputIterator,class ForwardIterator,class T>
inline ForwardIterator _uninitialized_copy(InputIterator first,InputIterator last,ForwardIterator result,T*){
    typedef typename __type_traits<T>::is_POD_type is_POD;
    
    return _uninitialized_copy_aux(first,last,result,is_POD());
}

//如果指向范围未初始化，则产生拷贝构造
template<class InputIterator,class ForwardIterator>
ForwardIterator uninitialized_copy(InputIterator first,InputIterator last,ForwardIterator result){
    return _uninitialized_copy(first, last, result,value_type(result));
}

//针对char*的全特化，ps:模版函数只能全特化
template<>
inline char* uninitialized_copy(const char* first,const char* last,char* result){
    memmove(result,first,last-first);
    return result+(last-first);
}

//wchar_t* 全特化
template<>
inline wchar_t* uninitialized_copy(const wchar_t* first,const wchar_t* last,wchar_t* result){
    memmove(result,first,sizeof(wchar_t) * (last-first));
    return result+(last-first);
}


//分离内存配置与构造行为
//在[first,last)范围内，产生x的复制

template<class ForwardIterator,class T>
void _uninitialized_fill_aux(ForwardIterator first,ForwardIterator last,const T& value,__false_type){
    ForwardIterator cur = first;
    while(cur!=last){
        construct(&*cur, value);
        cur++;
    }
}

template<class ForwardIterator,class T>
void _uninitialized_fill_aux(ForwardIterator first,ForwardIterator last,const T& value,__true_type){
    fill(first, last, value);
}

template<class ForwardIterator,class T,class T1>
void _uninitialized_fill_(ForwardIterator first,ForwardIterator last,const T& value,T1*){
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    _uninitialized_fill_aux(first,last,value,is_POD());
}

template<class ForwardIterator,class T>
void uninitialized_fill(ForwardIterator first,ForwardIterator last,const T& value){
    _uninitialized_fill_(first,last,value,value_type(first));
}


//-------------------fill_n------------------------------------//

template<class ForwardIterator,class Size,class T>
ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first,Size n,const T& value,__true_type){
    return std::fill_n(first, n, value);
}

//返回空间的尾
template<class ForwardIterator,class Size,class T>
ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first,Size n,const T& value,__false_type){
    ForwardIterator cur = first;
    while(n>0){
        construct(&*cur, value);
        --n; ++cur;
    }
    return cur;
}

template<class ForwardIterator,class Size,class T,class T1>
inline ForwardIterator _uninitialized_fill_n(ForwardIterator first,Size n,const T& value,T1*){
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    return _uninitialized_fill_n_aux(first,n,value,is_POD());//应该产生is_POD临时对象作为形参
}

//在[first,first+n)范围内，产生x的复制
template<class ForwardIterator,class Size,class T>
ForwardIterator uninitialized_fill_n(ForwardIterator first,Size n,const T& value){
    return _uninitialized_fill_n(first,n,value,value_type(first));
}







}
#endif /* TinySTL_uninitialized_h */
