//
//  TinySTL_vector.h
//  MyStl
//
//  Created by 姚传望 on 2020/4/24.
//  Copyright © 2020 姚传望. All rights reserved.
//

#ifndef TinySTL_vector_h
#define TinySTL_vector_h
#include <cstddef>
#include "TinySTL_alloc.h"
#include "TinySTL_construct.h"

namespace TinySTL {
template<class T,class Alloc=alloc>//默认使用二级配置器
class vector{
public:
    typedef T                       value_type;
    typedef value_type*             pointer;
    typedef value_type*             iterator;
    typedef const iterator          const_iterator;
    typedef value_type&             reference;
//    typedef const reference const_reference;//这句为什么会产生警告？？？
    typedef const value_type&       const_reference;
    typedef ptrdiff_t               difference_type;//距离
    typedef size_t                  size_type;
    
protected:
    iterator start;//目前可用空间的头
    iterator finish;//目前可用空间的尾,指向最后一个元素的下一个位置
    iterator end_of_storage;//目前可用空间的尾
    
    typedef simple_alloc<value_type, Alloc> data_allocator;//分配器，仅分配内存
    
    void insert_aux(iterator position,const T& x);
    
    //大于128bytes调用第一级分配器，即free，否则调用第二级分配器
    void deallocate(){
        if(start) data_allocator::deallocate(start,end_of_storage-start);
    }
    
    iterator allocate_and_fill(size_type n,const T& value){
        iterator result = data_allocator::allocate(n);
        //接下来初始化分配的这块内存
        try{
            //调用placement new初始化内存
            uninitialized_fill_n(result, n, value);
            return result;
        }
        catch(...){
            //大于128bytes调用第一级分配器，即free，否则调用第二级分配器
            data_allocator::deallocate(result, n);
            throw ;
        }
        return result;
    }
    
    void fill_initialize(size_type n,const T& value){
        start = allocate_and_fill(n,value);
        finish = start + n;
        end_of_storage = finish;
    }
public:
    iterator begin() const{return start;}
//    iterator test(){return start;}
    iterator end() const{return finish;}
    
    reference front(){return *start;}
    reference back(){return *(finish-1);}
    
    //大小
    size_type size() const {return size_type(end()-begin());}
    //为空
    bool empty(){return begin()==end();}
    //容量
    size_type capacity()const {
        return size_type(end_of_storage-begin());
    }
    //重载[]
    reference operator[](size_type n){
        return *(begin()+n);
    }
public:
    vector():start(),finish(0),end_of_storage(0){}
    
    vector(size_type n,const T& value){
        fill_initialize(n, value);
    }
    vector(int n,const T& value){
        fill_initialize(n, value);
    }
    vector(long n,const T& value){
        fill_initialize(n, value);
    }
    
    vector(std::initializer_list<T> vals){
        size_t n = vals.size();
        fill_initialize(n, 0);//先初始化为0，再赋值
        iterator begin = start;
        for(auto p=vals.begin();p!=vals.end();++p){
            *begin = *p;
            begin++;
        }
    }
    
    explicit vector(size_type n){
        fill_initialize(n, T());
    }
    
    ~vector(){
//        destroy(start,finish);
        deallocate();
    }
    
public:
    void push_back(const T& value){
        if(finish!=end_of_storage){//
            construct(finish, value);
            //*finish++ = value;//效果一样？
            ++finish;
        }
        else{
            insert_aux(finish, value);
        }
    }
    
    void pop_back(){
        --finish;
        destroy(finish);//如果vector里放的是class对象，那么将会调用这个对象的析构函数
    }
    
    iterator erase(iterator position){
        if(position!=end()-1){
            uninitialized_copy(position+1, finish, position);//地址重叠，不过是向前复制，应该没问题
        }
//        for(iterator cur=position;cur!=finish-1;++cur){
//            *cur = *(cur+1);
//        }
        --finish;
        destroy(finish);//如果vector里放的是class对象，那么将会调用这个对象的析构函数
        return begin();
    }
    
    //前开后闭区间，删除[first,last)内的元素
    iterator erase(iterator first,iterator last){
        if(first>last) return iterator();//越界检查
//        const size_type n = last - first + 1;//删除的个数
        iterator i = uninitialized_copy(last, finish, first);//地址重叠，不过是向前复制
        destroy(i,finish);
        finish = finish - (last - first);
//        iterator t = begin;
        return begin();
    }
    
    void clear(){
        erase(begin(),end());
    }
    
    void insert(iterator position,const T& value);
    void insert(iterator position,size_t n,const T& value);
    
};//end of class vector

template<class T,class Alloc>
void vector<T,Alloc>::insert_aux(iterator position, const T &value){
    if(finish!=end_of_storage){//还有备用空间，且插入点不是末尾时，被insert调用
        construct(finish, *(finish-1));
        ++finish;
        T value_copy = value;
        std::copy_backward(position, finish-2, finish-1);
        *position = value_copy;
    }
    else{//无备用空间
        const size_type old_size = size();
        const size_type len = old_size==0?1:2*old_size;
        iterator new_start = data_allocator::allocate(len);
        iterator new_finish = new_start;
        try{
            //position即finish
            //对基本数据类型调用copy，否则调用n次placement new
            new_finish = uninitialized_copy(start, position, new_start);
            construct(new_finish, value);
            ++new_finish;
            //如果insert时发生了空间满，则要分两次把插入位置前后的元素都拷贝
            new_finish = uninitialized_copy(position, finish, new_finish);
            //new_finish指向的是新分配空间的尾
        }
        catch(...){
            TinySTL::destroy(new_start,new_finish);
            data_allocator::deallocate(new_start, len);
            throw;
        }
        //析构函数，只有vector存放类对象时起作用
        TinySTL::destroy(start,finish);
        //回收原来的空间
        deallocate();
        
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}

template<class T,class Alloc>
void vector<T,Alloc>::insert(iterator position, const T &value){
    if(finish!=end_of_storage && position==end()){//还有备用空间，且插入点是末尾
        construct(position, value);
        ++finish;
    }
    else{
        insert_aux(position, value);
    }
}

template<class T,class Alloc>
void vector<T,Alloc>::insert(iterator position, size_t n, const T &value){
    if(!n) return ;
    if(size_type(end_of_storage-finish)>=n){
        T value_copy = value;
        const size_type elems_after = finish - position;
        iterator old_finish = finish;
        if(elems_after>n){//插入点后面的元素个数大于n
            uninitialized_copy(finish-n, finish, finish);//因为直接copy会产生地址重叠，所以分两步
            finish+=n;
            copy_backward(position,old_finish-n,old_finish);
            fill(position,position+n,value_copy);
            
            //下面的操作是否可行？
//            uninitialized_copy(position, finish, finish);
//            finish+=finish-position;
//            fill(position,old_finish,value_copy);
        }
        else{
            uninitialized_fill_n(finish, n-elems_after, value_copy);
            finish += n - elems_after;
            uninitialized_copy(position, old_finish, finish);
            finish += elems_after;
            fill(position,old_finish,value_copy);
        }
    }
    else{//备用空间不足
        const size_type old_size = size();
        const size_type len = old_size + std::max(old_size,n);//如果旧长度的两倍依然不够，则old_size+n
        iterator new_start; iterator new_finish;
        new_start = data_allocator::allocate(len);
        new_finish = new_start;
        try{
            //插入点之前的元素复制到新空间
            new_finish = uninitialized_copy(start, position, new_start);
            //复制要插入的新元素
            new_finish = uninitialized_fill_n(position, n, value);
            //插入点后面的元素复制到新空间
            new_finish = uninitialized_copy(position, finish, new_finish);
        }
        catch(...){
            destroy(new_start,new_finish);
            data_allocator::deallocate(new_start, len);
            throw ;
        }
        //释放旧空间
        destroy(start,finish);
        deallocate();
        
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}

}//end of namespace

#endif /* TinySTL_vector_h */
