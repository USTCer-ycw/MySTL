//
//  TinySTL_RBTree.h
//  MyStl
//
//  Created by 姚传望 on 2020/5/3.
//  Copyright © 2020 姚传望. All rights reserved.
//

#ifndef TinySTL_RBTree_h
#define TinySTL_RBTree_h
#ifndef NULL
#define NULL 0
#endif
namespace TinySTL
{

typedef bool _rb_tree_color_type;
const _rb_tree_color_type _rb_tree_red = false;
const _rb_tree_color_type _rb_tree_black = true;

struct _rb_tree_node_base
{
    typedef _rb_tree_color_type color_type;
    typedef _rb_tree_node_base* base_ptr;

    color_type color;
    base_ptr parent;
    base_ptr left;
    base_ptr right;

    //寻找红黑树中最小值
    static base_ptr minimum(base_ptr x){
        while(x->left){
            x=x->left;
        }
        return x;
    }
    //寻找红黑树中最大值
    static base_ptr maximum(base_ptr x){
        while(x->right){
            x=x->right;
        }
        return x;
    }
};

template <class Value>
struct _rb_tree_node:public _rb_tree_node_base
{
    typedef _rb_tree_node<Value>* link_type;
    Value value_field;//节点值
};

struct _rb_tree_iterator_base
{
    typedef _rb_tree_node_base::base_ptr base_ptr;
    typedef bidirectional_iterator_tag iterator_category;
    typedef ptrdiff_t difference_type;

    base_ptr node;
    
    void increment(){
        if(node->right){//找到右子树中最小的那个
            node = node->right;
            while(node->left){
                node = node->left;
            }
        }
        else{//找到二叉搜索树中序遍历的下一个节点
            base_ptr node_parent = node->parent;
            while(node == node_parent->right){
                node = node_parent;
                node_parent = node_parent->parent;
            }
            if(node->right != node_parent){
                node = node_parent;
            }
        }
    }
    void decrement(){
        if(node->color == _rb_tree_red && node->parent->parent == node){
            node = node->right;//如果node为header(end)时，返回node
        }
        else if(node->left){//下面寻找中序遍历的上一个节点
            base_ptr node_left = node->left;//寻找左子树中最大的那个数
            while(node_left->right){
                node_left = node_left->right;
            }
            node = node_left;
        }
        else{
            base_ptr node_parent = node->parent;
            while(node == node_parent->left){
                node = node_parent;
                node_parent = node_parent->parent;
            }
            node = node_parent;
        }
    }
};

inline void _rb_tree_rotate_left(_rb_tree_node_base* x,_rb_tree_node_base*& root){
    _rb_tree_node_base* y = x->right;
    x->right = y->left;
    if(y->left){
        y->left = x->parent;
    }
    y->parent = x->parent;
    if(x==root){
        root = y;
    }
    else if(x==x->parent->left){
        x->parent->left = y;
    }
    else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

inline void _rb_tree_rotate_right(_rb_tree_node_base* x,_rb_tree_node_base*& root){
    _rb_tree_node_base* y= x->left;
    x->left = y->right;
    if(y->right){
        y->right->parent =  x;
    }
    y->parent = x->parent;

    if(x==root){
        root = y;
    }
    else if(x==x->parent->right){
        x->parent->right = y;
    }
    else{
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

inline void _rb_tree_rebalance(_rb_tree_node_base* x,_rb_tree_node_base*& root){
    x->color = _rb_tree_red;
    while(x!=root && x->parent->color == _rb_tree_red){
        if(x->parent == x->parent->parent->left){
            _rb_tree_node_base* y = x->parent->parent->right;
            if(y && y->color == _rb_tree_red){
                x->parent->color = _rb_tree_black;
                x->parent->parent->color = _rb_tree_red;
                x = x->parent->parent;
            }
            else{
                if(x == x->parent->right){
                    x = x->parent;
                    _rb_tree_rotate_left(x,root);
                }
                x->parent->color = _rb_tree_black;
                x->parent->parent->color = _rb_tree_red;
                _rb_tree_rotate_right(x->parent->parent,root);
            }
        }
        else {
            _rb_tree_node_base* y = x->parent->parent->left;
            if(y && y->color == _rb_tree_red){
                x->parent->color = _rb_tree_black;
                y->color = _rb_tree_black;
                x->parent->parent->color = _rb_tree_red;
                x = x->parent->parent;
            }
            else {
                if(x==x->parent->left){
                    x = x->parent;
                    _rb_tree_rotate_right(x,root);
                }
                x->parent->color = _rb_tree_black;
                x->parent->parent->color = _rb_tree_red;
                _rb_tree_rotate_left(x->parent->parent,root);
            }
        }
    }
    root->color = _rb_tree_black;
}

template<class Value,class Ref,class Ptr>
struct _rb_tree_iterator:public _rb_tree_iterator_base
{
    typedef Value value_type;
    typedef Ref reference;
    typedef Ptr pointer;

    typedef _rb_tree_iterator<Value,Value&,Value*> iterator;
    typedef _rb_tree_iterator<Value,const Value&,const Value*> const_iterator;
    typedef _rb_tree_iterator<Value,Ref,Ptr> self;
    typedef _rb_tree_node<Value>* link_type;

    _rb_tree_iterator(){}
    //_rb_tree_iterator(link_type x)：node(x){}
    //上面这句是错误的，因为不能用子类的初始化列表初始化父类成员
    _rb_tree_iterator(link_type x){node = x;}//这是对的，因为node=x是“赋值”而不是初始化

    //注意const iterator和const_iterator的区别
    _rb_tree_iterator(const iterator& it) {node = it.node;}

    //父类指针转为子类
    reference operator*() const{ return link_type(node)->value_field; }
    pointer operator->() const{ return &(operator*()); }//只有value_field为类对象的时候才用得着

    self& operator++(){
        increment();
        return *this;
    }
    self& operator++(int){
        self tmp = *this;
        increment();
        return *this;
    }
    
    bool operator!=(iterator& it){
        return node==it.node;
    }
    
    self& operator--(){
        decrement();
        return *this;
    }
    self& operator--(int){
        self tmp = *this;
        decrement();
        return tmp;
    }
};

template<class Key,class Value,class KeyOfValue,class Compare,class Alloc = alloc>
class rb_tree
{
protected:
    typedef void* void_pointer;
    typedef _rb_tree_node_base* base_ptr;
    typedef _rb_tree_node<Value> rb_tree_node;
    typedef simple_alloc<rb_tree_node,Alloc> rb_tree_node_allocator;
    typedef _rb_tree_color_type color_type;

public:
    typedef Key key_type;
    typedef Value value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef rb_tree_node* link_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

protected:
    link_type get_node(){
        return rb_tree_node_allocator::allocate();
    }
    //分离配置空间和赋值行为
    link_type create_node(const value_type& value){
        link_type tmp = get_node();
        try{
            construct(&tmp->value_field,value);
        }
        catch(...){
            put_node(tmp);
        }
        return tmp;
    }

    link_type clone_node(link_type ptr){
        link_type tmp = create_node(ptr->value_field);
        tmp->color = ptr->color;
        tmp->left = tmp->right = 0;
        return tmp;
    }

    void put_node(link_type p){ rb_tree_node_allocator::deallocate(p);}

    void destroy(link_type ptr){
        destroy(&ptr->value_field);
        put_node(ptr);
    }

protected:
    size_type node_count;//记录树的节点数量
    link_type header; //header指向最小和最大的节点
    Compare key_compare;//节点之前键值大小比较的准册，Compare是个仿函数

    //header的parent指向root，left指向min，right指向max
    link_type& root() const { //为何要返回指针的引用? 方便作为左值进行赋值
        return (link_type&)header->parent;
    }
    link_type& leftmost() const{
        return (link_type&)header->left;
    }
    link_type& rightmost() const{
        return (link_type&)header->right;
    }

    //下面的函数用来方便取得节点x的成员
    static link_type& left(link_type x){
        return (link_type&) (x->left);
    }

    static link_type& right(link_type x){
        return (link_type&) (x->right);
    }

    static link_type& parent(link_type x){
        return (link_type&) (x->parent);
    }

    static reference value(link_type x){
        return x->value_field;
    }

    static const Key& key(link_type x){
        return KeyOfValue()(value(x));//KeyOfValue是函数或者函数指针
    }

    static color_type& color(link_type x){
        return (color_type&)x->color; //为什么要做个转换呢？
    }

    //下面的函数用来方便取得节点x的成员
    static link_type& left(base_ptr x){
        return (link_type&) (x->left);
    }

    static link_type& right(base_ptr x){
        return (link_type&) (x->right);
    }

    static link_type& parent(base_ptr x){
        return (link_type&) (x->parent);
    }

    static reference value(base_ptr x){
        return ((link_type)x)->value_field;
    }

    static const Key& key(base_ptr x){
        return KeyOfValue()(value((link_type)x));//KeyOfValue是函数或者函数指针
    }

    static color_type& color(base_ptr x){
        return (color_type&)(link_type(x)->color);
    }

    static link_type minimum(link_type x) {
        return (link_type) _rb_tree_node_base::minimum(x);
    }

    static link_type maximum(link_type x) {
        return (link_type) _rb_tree_node_base::maximum(x);
    }
public:
    typedef _rb_tree_iterator<value_type,reference,pointer> iterator;

private:
    iterator _insert(base_ptr x,base_ptr y,const value_type& v);
    link_type _copy(link_type x,link_type p);
    void _erase(link_type x);
//    void clear();

    //初始化header为红色，parent为空，left和right都指向自己
    void init(){
        header = get_node();
        color(header) = _rb_tree_red;
        root() = NULL;
        leftmost() = header;
        rightmost() = header;
    }

public:
    rb_tree(const Compare& comp = Compare()):node_count(0),key_compare(comp){
        init();
    }

    ~rb_tree(){
//        clear();
        put_node(header);
    }

public:
    rb_tree<Key,Value,KeyOfValue,Compare,Alloc>&
    operator=(const rb_tree<Key,Value,KeyOfValue,Compare,Alloc>& x);

public:
    Compare key_comp() const{
        return key_compare;
    }
    iterator begin(){
        // return header->left;
        return leftmost();
    }
    iterator end(){
        return rightmost();
    }

    bool empty() const{
        // return header->parent==NULL;
        return node_count==0;
    }
    size_type size() const{
        return node_count;
    }
    size_type max_size() const{
        return size_type(-1);//这什么操作？
    }

public:
    //把x插入到红黑树中,且节点值唯一
    std::pair<iterator,bool> insert_unique(const value_type& x);
    //允许节点值重复
    iterator insert_equal(const value_type& x);
};


template<class Key,class Value,class KeyOfValue,class Compare,class Alloc>
typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator
rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::
_insert(base_ptr _x,base_ptr _y,const Value& v){
    link_type x = (link_type) _x;
    link_type y = (link_type) _y;
    link_type z;

    if(y==header || x || key_compare(KeyOfValue()(v),key(y))){
        z = create_node(v);
        left(y) = z;
        if(y==header){
            root() = z;
            rightmost() = z;
        }
        else if(y==leftmost()){
            leftmost() = z;
        }
    }
    else {
        z = create_node(v);
        right(y) = z;
        if(y==rightmost()){
            rightmost() =z;
        }
    }
    parent(z) = y;
    left(z) = NULL;
    right(z) = NULL;

    _rb_tree_rebalance(z,header->parent);
    ++node_count;
    return iterator(z);
}

template<class Key,class Value,class KeyOfValue,class Compare,class Alloc>
typename rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::iterator
rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::
insert_equal(const Value& value){
    link_type insert_node = header;
    link_type root = root();
    while(root){
        insert_node = root;
        root = key_compare(KeyOfValue()(value),key(root))?left(root):right(root);
    }
    return _insert(root,insert_node,value);
}

template<class Key,class Value,class KeyOfValue,class Compare,class Alloc>
std::pair<typename rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::iterator,bool>
rb_tree<Key,Value,KeyOfValue,Compare,Alloc>::
insert_unique(const Value& value){
    link_type insert_node = header;
    link_type root = root();
    bool comp = true;
    while (root){
        insert_node = root;
        comp = key_compare(KeyOfValue()(value),key(root));//比较插入的值和当前的“根”节点的值
        root = comp?left(root):right(root);//comp为真，即插入值小于root，往左走;反之
    }//结束循环后，插入点的父节点已找到(insert_node)，root则为真正要插入的点，且它为空
    
    iterator index = iterator(insert_node);//index指向插入节点的父节点
    if(comp){//comp为真，插入左子树
        if(index==begin()){//如果插入点是最小节点的子节点
            return std::pair<iterator,bool>(_insert(root,insert_node,value),true);
        }
        else --index;
    }
    //插入右侧
    if(key_compare(key(index.node),KeyOfValue()(insert_node))){
        return std::pair<iterator,bool>(_insert(root,insert_node,value),true);
    }
    //如果值相同，则comp为假，且 --index指向的是比插入值更小的值，所以第二个if也不成立
    //表明新值重复，不进行插入
    return std::pair<iterator,bool>(index,false);
}

} // namespace TinySTL



#endif /* TinySTL_RBTree_h */
