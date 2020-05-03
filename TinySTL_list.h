//
//  TinySTL_list.h
//  MyStl
//
//  Created by 姚传望 on 2020/5/2.
//  Copyright © 2020 姚传望. All rights reserved.
//

#ifndef TinySTL_list_h
#define TinySTL_list_h
namespace TinySTL
{
//list节点结构
template<class T>
struct _list_node{
    typedef _list_node<T>* list_pointer;//why void*? not _list_node*?
    list_pointer prev;
    list_pointer next;
    T data;
};

//list 迭代器
template<class T,class Ref,class Ptr>
struct _list_iterator
{
    typedef _list_iterator<T,T&,T*>     iterator;
    typedef const iterator const_iterator;
    typedef _list_iterator<T,Ref,Ptr>   self;

    typedef bidirectional_iterator_tag  iterator_category;//迭代器具备前移后移的功能，所以是bid类型
    typedef T   value_type;
    typedef Ref reference;
    typedef Ptr pointer;
    typedef _list_node<T>* link_type;
    typedef size_t  size_type;
    typedef ptrdiff_t   difference_type;
    
    link_type node;//指向节点
    
    //构造函数
    _list_iterator(link_type x):node(x){};
    _list_iterator(){};
    _list_iterator(const iterator& x):node(x.node){};
    
    //运算符重载
    bool operator==(const self& it) const { return it.node==node; }
    bool operator!=(const self& it) const { return it.node!=node; }

    //重载+号，使得operator支持随机访问
    // iterator operator+(size_type n){
    //     while(n){
    //         node = node->next;
    //         n--;
    //     }
    //     return node;
    // }
    
    reference operator*() const { return node->data;}
    pointer operator->() const { return &(operator*());}
    
    //前置++
    self& operator++(){
        node = (link_type)(node->next);
        return *this;
    }
    //后置++ 没有引用
    self operator++(int){
        self tmp = *this;
        ++*this;
        return tmp;
    }
    //前置--
    self& operator--(){
        node = (link_type)(node->prev);
        return *this;
    }
    //后置--
    self operator--(int){
        self tmp = *this;
        --*this;
        return tmp;
    }
};

//前闭后开区间,尾端是个空节点
template<class T,class Alloc = alloc>
class list {

protected:
    typedef _list_node<T> list_node;
    typedef simple_alloc<list_node,Alloc> list_node_allocator;
public:
    typedef list_node* link_type;

    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const value_type const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

public:
    typedef _list_iterator<T,T&,T*> iterator;
    typedef _list_iterator<T,const T&,const T*> const_iterator;

protected:
    link_type node;//only one pointer
    
    //生成一个节点空间
    link_type get_node(){return list_node_allocator::allocate();}
    //释放一个节点空间
    void put_node(link_type p){ list_node_allocator::deallocate(p);}

    //产生一个节点
    link_type create_node(const_reference value){
        link_type p = get_node();
        construct(&p->data,value);
        return p;
    }
    //销毁并释放节点
    void destroy_node(link_type p){
        destroy(&p->data);//如果list里存放的是类对象,则需要一个析构函数
        put_node(p);
    }
public:
    //前闭后开区间
    iterator begin() { return node->next;}
    iterator end() {return node;}

    bool empty() {return node->next==node; }

    size_type size() const{
        size_type result = 0;
        result = distance(begin(),end());
        return result;
    }

    reference front(){ return *begin();}
    reference back() {return *(--end());}//为什么--？？,因为产生了一个指向node的临时对象

    void swap(list<T,Alloc>& x){std::swap(node,x.node); }

public:
    list(){ empty_initialize(); }

protected:
    void empty_initialize(){
        node = get_node();
        node->next = node;
        node->prev = node;
    }

public:
    iterator insert(iterator position,const_reference value){
        link_type tmp = create_node(value);
        tmp->next = position.node;
        tmp->prev = position.node->prev;
        position.node->prev->next = tmp;
        position.node->prev = tmp;
        return tmp;//产生一个匿名的iterator对象，即iterator(tmp)
    }

    void push_back(const_reference value){
        insert(end(),value);
    }

    void push_front(const_reference value){
        insert(begin(),value);
    }

    iterator erase(iterator position){
        link_type prev_node = position.node->prev;
        link_type next_node = position.node->next;
        prev_node->next = next_node;
        next_node->prev = prev_node;
        destroy_node(position.node);
        return iterator(next_node);
    }

    void pop_front() {erase(begin());}

    void pop_back() {
        iterator tmp = end();
        erase(--tmp);
    }

    //将链表x接合到position之前
    void splice(iterator position,list& x){//为什么不是list<T>& x呢？？这样的话可能把两个类型不同的链表接合到一起，实际上list&可以是任意list的引用
        if(x.empty()) transfer(position,x.begin(),x.end());
    }

    //将i所指的元素接合于position所指位置之前，position和i可能指向同一个list
    void splice(iterator position,list&,iterator i){
        iterator j =i;
        ++j;
        //如果i指向pos，即把pos接合到pos之前……显然这是不合理到
        //如果next_i指向pos，即把pos前面到节点接合到pos之前，显然这是没必要到
        if(position == i || position == j) return ;
        transfer(position,i,j);
    }

    //将[first,last)内的所有元素接合到pos之前，但pos和first可能指向一个list
    //且pos不能位于first和last内
    void splice(iterator position,iterator first,iterator last){
        if(first != last) transfer(position,first,last);
    }


    void clear();//清空
    void remove(const_reference value);//删除所有值为value的节点
    iterator find(iterator first,iterator last,const_reference value){
        while(first!=last){

        }
    }

    void sort();
    void merge(list<T, Alloc>& x);

protected:
    //将[first,last)中的所有元素移动到position之前
    void transfer(iterator position,iterator first,iterator last){
        iterator pre_of_pos = position.node->prev;
        iterator pre_of_first = first.node->prev;
        iterator pre_of_last = last.node->prev;

        pre_of_last.node->next = position.node;
        pre_of_first.node->next = last.node;
        pre_of_pos.node->next = first.node;
        position.node->prev = pre_of_last.node;
        last.node->prev = pre_of_first.node;
        first.node->prev = pre_of_pos.node;
    }
};

template<class T,class Alloc>
void list<T,Alloc>::clear(){
    link_type cur = begin();
    while(cur!=node){
        link_type tmp = cur;
        cur = cur->next;
        destroy_node(tmp);
    }
    //恢复只有一个空节点的状态
    node->next = node;
    node->prev = node;
}

template<class T,class Alloc>
void list<T, Alloc>::remove(const_reference value){
    iterator first = begin();
    iterator last = end();
    while (first!=last){
        if(*first == value){
            iterator tmp = first;
            ++first;
            erase(tmp);
        }
        else{
            ++first;
        }
    }
}

template<class T,class Alloc>
void list<T,Alloc>::merge(list<T, Alloc>& x){
    iterator first1 = begin();
    iterator last1 = end();
    iterator first2 = x.begin();
    iterator last2 = x.end();

    while(first1 != last1 && first2 != last2){
        if(first1.node->data>first2.node->data){
            iterator next_i = first2;
            ++next_i;
            transfer(first1,first2,next_i);
            first2 = next_i;
        }
        else{
            ++first1;
        }
    }
    if(first2!=last2) transfer(last1,first2,last2);
}

template<class T,class Alloc>
void list<T,Alloc>::sort(){
    if(node->next==node || node->next->next==node) return;
    
    //中介数据存放区
    list<T, Alloc> carry; list<T, Alloc> counter[64];
    int fill = 0;
    while(!empty()){
        carry.splice(carry.begin(),*this,begin());
        int i = 0;
        while(i<fill && !counter[i].empty()){
            counter[i].merge(carry);
            carry.swap(counter[i++]);
        }
        carry.swap(counter[i]);
        if(i==fill) ++fill;
    }
    for (int i = 1; i < fill; ++i){
        counter[i].merge(counter[i-1]);
    }
    swap(counter[fill-1]);
    
}

} // namespace tinySTL


#endif /* TinySTL_list_h */
